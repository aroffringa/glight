#include "audioplayer.h"

#include <iostream>

#include <boost/date_time/posix_time/posix_time.hpp>

void AudioPlayer::open() {
  if (_isOpen) throw AlsaError("Alsa was opened twice");

  snd_pcm_uframes_t buffer_size = _alsaBufferSize;
  snd_pcm_uframes_t period_size = _alsaPeriodSize;

  void **hints;
  if (snd_device_name_hint(-1, "pcm", &hints) < 0)
    throw AlsaError("snd_device_name_hint() returned error");
  size_t hi = 0;
  while (hints[hi] != nullptr) {
    char *deviceName = snd_device_name_get_hint(hints[hi], "NAME");
    std::cout << " - " << deviceName << '\n';
    free(deviceName);
    ++hi;
  }
  snd_device_name_free_hint(hints);

  // Open PCM device for playback.
  int rc = snd_pcm_open(&_handle, "pulse", SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0)
    throw AlsaError(std::string("snd_pcm_open() returned: ") +
                    snd_strerror(rc));
  _isOpen = true;

  // Get default hardware parameters
  snd_pcm_hw_params_t *hw_params;
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
  buffer_size = 16 * 1024;
  snd_pcm_hw_params_set_buffer_size_near(_handle, hw_params, &buffer_size);
  snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size);
  std::cout << "Buffer size: " << buffer_size << '\n';
  snd_pcm_hw_params_get_period_size(hw_params, &period_size, nullptr);
  std::cout << "Period size: " << period_size << '\n';

  _alsaBufferSize = buffer_size;
  _alsaPeriodSize = period_size;

  // Write the parameters to the driver
  rc = snd_pcm_hw_params(_handle, hw_params);
  if (rc < 0) throw AlsaError(snd_strerror(rc));

  snd_pcm_hw_params_free(hw_params);

  // Get the software parameters
  snd_pcm_sw_params_t *sw_params;
  snd_pcm_sw_params_malloc(&sw_params);
  snd_pcm_sw_params_current(_handle, sw_params);

  snd_pcm_sw_params_set_start_threshold(_handle, sw_params,
                                        buffer_size - period_size);
  // snd_pcm_sw_params_set_avail_min(_handle, sw_params, period_size);

  rc = snd_pcm_sw_params(_handle, sw_params);
  if (rc < 0) throw AlsaError(snd_strerror(rc));

  snd_pcm_sw_params_free(sw_params);

  snd_pcm_prepare(_handle);
  unsigned writeAtATime = _alsaPeriodSize;
  unsigned position = _startPosition;

  while (!isStopping() && _decoder.HasMore()) {
    size_t readSize = writeAtATime * 4;
    _decoder.GetSamples(_alsaBuffer, readSize);

    if (position > buffer_size)
      _syncListener->OnSyncUpdate((double)(position - buffer_size) / 44.100);
    else
      _syncListener->OnSyncUpdate(0.0);

    rc = snd_pcm_writei(_handle, _alsaBuffer, writeAtATime);
    if (rc == -EPIPE) {
      std::cout << "Buffer underrun!\n";
      snd_pcm_prepare(_handle);
    } else if (rc < 0) {
      std::cout << "ERROR:" << snd_strerror(rc) << '\n';
      break;
    } else if (rc != (int)writeAtATime)
      std::cout << "Only " << rc
                << " frames were written in snd_pcm_writei().\n";

    position += writeAtATime;
  }
  if (isStopping())
    snd_pcm_drop(_handle);
  else
    snd_pcm_drain(_handle);
}

void AudioPlayer::close() {
  if (_alsaThread != nullptr) {
    _alsaThread->join();
    _alsaThread.reset();
  }
  if (_isOpen) {
    snd_pcm_close(_handle);
    _isOpen = false;
  }
}
