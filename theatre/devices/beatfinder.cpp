#include "beatfinder.h"

#include <cmath>
#include <iostream>
#include <vector>

#include <aubio/aubio.h>
#include <aubio/tempo/beattracking.h>
#include <aubio/tempo/tempo.h>

namespace glight::theatre {

void BeatFinder::open() {
  if (_isOpen) throw AlsaError("Alsa was opened twice");

  // Open PCM device for capture.
  int rc =
      snd_pcm_open(&_handle, device_name_.c_str(), SND_PCM_STREAM_CAPTURE, 0);
  if (rc < 0) throw AlsaError(snd_strerror(rc));
  _isOpen = true;

  // Get default hardware parameters
  snd_pcm_hw_params_t *hw_params = nullptr;
  snd_pcm_hw_params_malloc(&hw_params);
  snd_pcm_hw_params_any(_handle, hw_params);

  // Interleaved mode
  rc = snd_pcm_hw_params_set_access(_handle, hw_params,
                                    SND_PCM_ACCESS_RW_INTERLEAVED);
  if (rc < 0) throw AlsaError(snd_strerror(rc));

  // Signed 16-bit little-endian format
  unsigned samplerate = 44100;
  int dir = 0;
  snd_pcm_hw_params_set_format(_handle, hw_params, SND_PCM_FORMAT_S16_LE);
  snd_pcm_hw_params_set_channels(_handle, hw_params, 2);
  snd_pcm_hw_params_set_rate_near(_handle, hw_params, &samplerate, &dir);
  snd_pcm_uframes_t buffer_size = 16 * 1024;
  snd_pcm_hw_params_set_buffer_size_near(_handle, hw_params, &buffer_size);
  snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size);
  snd_pcm_uframes_t period_size = _alsaPeriodSize;
  snd_pcm_hw_params_get_period_size(hw_params, &period_size, nullptr);

  _alsaBufferSize = buffer_size;
  _alsaPeriodSize = period_size;

  // Write the parameters to the driver
  rc = snd_pcm_hw_params(_handle, hw_params);
  if (rc < 0) throw AlsaError(snd_strerror(rc));

  snd_pcm_hw_params_free(hw_params);

  // Get the software parameters
  snd_pcm_sw_params_t *sw_params = nullptr;
  snd_pcm_sw_params_malloc(&sw_params);
  snd_pcm_sw_params_current(_handle, sw_params);

  snd_pcm_sw_params_set_start_threshold(_handle, sw_params,
                                        buffer_size - period_size);
  // snd_pcm_sw_params_set_avail_min(_handle, sw_params, period_size);

  rc = snd_pcm_sw_params(_handle, sw_params);
  if (rc < 0) throw AlsaError(snd_strerror(rc));

  snd_pcm_sw_params_free(sw_params);

  snd_pcm_prepare(_handle);

  // Initial libaubio
  fvec_t *tempo_out = new_fvec(2);
  constexpr unsigned hopsPerAudioLevel = 4;
  const char method[] = "default";
  aubio_tempo_t *tempo =
      new_aubio_tempo(method, period_size * 2, period_size, samplerate);
  fvec_t *ibuf = new_fvec(period_size);
  const smpl_t silence_threshold = -30.;
  uint_t is_silence = 0;

  std::vector<int16_t> alsaBuffer(period_size * 2);

  _beat = Beat(0.0, 0.0);
  _audioLevel = 0;
  uint32_t audio_level_accumulator = 0;
  uint16_t nAudioLevels = 0;

  unsigned samples_since_last_beat = 0;
  unsigned last_beat_duration = std::numeric_limits<unsigned>::max();
  enum class Stage { Start, Quarter, Half, ThreeQuarter } stage = Stage::Start;

  while (!_isStopping) {
    rc = snd_pcm_readi(_handle, alsaBuffer.data(), period_size);
    if (rc == -EPIPE) {
      std::cout << "Buffer overrun!\n";
      snd_pcm_prepare(_handle);
    } else if (rc < 0) {
      std::cout << "ERROR:" << snd_strerror(rc) << '\n';
      break;
    } else {
      if (rc != static_cast<int>(period_size))
        std::cout << "Only " << rc << " frames were read in snd_pcm_readi().\n";
    }
    uint32_t audioRMS = 0;
    for (size_t i = 0; i != period_size; ++i) {
      int16_t l = alsaBuffer[i * 2];
      int16_t r = alsaBuffer[i * 2 + 1];

      smpl_t s = static_cast<smpl_t>(l) + static_cast<smpl_t>(r);
      fvec_set_sample(ibuf, s, i);

      audioRMS += static_cast<uint32_t>(static_cast<int32_t>(l) *
                                        static_cast<int32_t>(l)) >>
                  8;
      audioRMS += static_cast<uint32_t>(static_cast<int32_t>(r) *
                                        static_cast<int32_t>(r)) >>
                  8;
    }
    audio_level_accumulator += audioRMS / (2 * period_size);
    ++nAudioLevels;
    if (nAudioLevels > hopsPerAudioLevel) {
      _audioLevel = audio_level_accumulator / hopsPerAudioLevel;
      nAudioLevels = 0;
      audio_level_accumulator = 0;
    }
    aubio_tempo_do(tempo, ibuf, tempo_out);
    const smpl_t is_beat = fvec_get_sample(tempo_out, 0);
    if (silence_threshold != -90)
      is_silence = aubio_silence_detection(ibuf, silence_threshold);

    if (is_beat && !is_silence) {
      const smpl_t confidence = aubio_tempo_get_confidence(tempo);
      if (confidence > _minimumConfidence) {
        Beat beat = _beat;  // atomic load
        beat.confidence = confidence;
        // We do a simple reset on high numbers just to make sure that the
        // 'float' precision does not run out. With 8 beats per second, this
        // would still take 24h to run out.
        if (beat.value < (60.0 * 60.0 * 24.0 * 8.0))
          beat.value = std::floor(beat.value + 1);
        else
          beat.value = 0.0;
        _beat = beat;  // atomic store
        last_beat_duration = samples_since_last_beat;
        samples_since_last_beat = 0;
        stage = Stage::Start;
      }
    } else if (is_silence) {
      Beat beat = _beat;  // atomic load
      beat.confidence = 0.0;
      _beat = beat;  // atomic store
    }

    ++samples_since_last_beat;
    switch (stage) {
      case Stage::Start:
        if (samples_since_last_beat * 4 > last_beat_duration) {
          stage = Stage::Quarter;
          Beat beat = _beat;  // atomic load
          beat.value += 0.25;
          _beat = beat;
        }
        break;
      case Stage::Quarter:
        if (samples_since_last_beat * 2 > last_beat_duration) {
          stage = Stage::Half;
          Beat beat = _beat;  // atomic load
          beat.value += 0.25;
          _beat = beat;
        }
        break;
      case Stage::Half:
        if (samples_since_last_beat * 4 > last_beat_duration * 3) {
          stage = Stage::ThreeQuarter;
          Beat beat = _beat;  // atomic load
          beat.value += 0.25;
          _beat = beat;
        }
        break;
      case Stage::ThreeQuarter:
        break;
    }
  }
  snd_pcm_drop(_handle);

  del_aubio_tempo(tempo);
  del_fvec(tempo_out);
  del_fvec(ibuf);
}

void BeatFinder::close() {
  _isStopping = true;
  if (_alsaThread != nullptr) {
    _alsaThread->join();
    _alsaThread.reset();
  }
  if (_isOpen) {
    snd_pcm_close(_handle);
    _isOpen = false;
  }
}

}  // namespace glight::theatre
