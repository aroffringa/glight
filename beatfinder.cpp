#include "beatfinder.h"

#include <iostream>

#include <boost/date_time/posix_time/posix_time.hpp>

#include <aubio/aubio.h>
#include <aubio/tempo/tempo.h>
#include <aubio/tempo/beattracking.h>

void BeatFinder::open()
{
	if(_isOpen)
		throw AlsaError("Alsa was opened twice");

	snd_pcm_uframes_t buffer_size=_alsaBufferSize;
	snd_pcm_uframes_t period_size=_alsaPeriodSize;

	// Open PCM device for capture.
	int rc = snd_pcm_open(&_handle, "pulse", SND_PCM_STREAM_CAPTURE, 0);
	if (rc < 0)
		throw AlsaError(snd_strerror(rc));
	_isOpen = true;

	// Get default hardware parameters
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_hw_params_malloc(&hw_params);
	snd_pcm_hw_params_any(_handle, hw_params);

  // Interleaved mode
  rc = snd_pcm_hw_params_set_access(_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (rc < 0)
		throw AlsaError(snd_strerror(rc));

  // Signed 16-bit little-endian format
  unsigned samplerate = 44100;
  int dir=0;
  snd_pcm_hw_params_set_format(_handle, hw_params, SND_PCM_FORMAT_S16_LE);
  snd_pcm_hw_params_set_channels(_handle, hw_params, 2);
  snd_pcm_hw_params_set_rate_near(_handle, hw_params, &samplerate, &dir);
	buffer_size = 16*1024;
	snd_pcm_hw_params_set_buffer_size_near(_handle, hw_params, &buffer_size);
	snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size);
	snd_pcm_hw_params_get_period_size(hw_params, &period_size, 0);

	_alsaBufferSize = buffer_size;
	_alsaPeriodSize = period_size;

  // Write the parameters to the driver
  rc = snd_pcm_hw_params(_handle, hw_params);
	if (rc < 0)
		throw AlsaError(snd_strerror(rc));

	snd_pcm_hw_params_free(hw_params);

	// Get the software parameters
	snd_pcm_sw_params_t *sw_params;
	snd_pcm_sw_params_malloc(&sw_params);
	snd_pcm_sw_params_current(_handle, sw_params);

	snd_pcm_sw_params_set_start_threshold(_handle, sw_params, buffer_size - period_size);
	//snd_pcm_sw_params_set_avail_min(_handle, sw_params, period_size);

	rc = snd_pcm_sw_params(_handle, sw_params);
	if (rc < 0)
		throw AlsaError(snd_strerror(rc));

	snd_pcm_sw_params_free(sw_params);

	snd_pcm_prepare(_handle);
	unsigned readAtATime = _alsaPeriodSize;
	
	// Initial libaubio
	fvec_t* tempo_out = new_fvec(2);
	uint hop_size = period_size;
	char method[] = "default";
  aubio_tempo_t* tempo = new_aubio_tempo(method, hop_size*2, hop_size, samplerate);
	fvec_t* ibuf = new_fvec(hop_size);
	smpl_t silence_threshold = -30.;
	uint_t is_silence = 0;
 
	std::vector<int16_t> alsaBuffer(hop_size*2);
	
	_beat = Beat(0.0, 0.0);
	_audioLevel = 0;
	
	while(!_isStopping)
	{
		rc = snd_pcm_readi(_handle, alsaBuffer.data(), readAtATime);
		if(rc == -EPIPE)
		{
			std::cout << "Buffer overrun!\n";
			snd_pcm_prepare(_handle);
		}
		else if (rc < 0)
		{
			std::cout << "ERROR:" << snd_strerror(rc) << '\n';
			break;
		}
		else {
			if (rc != (int) readAtATime)
				std::cout << "Only " << rc << " frames were read in snd_pcm_readi().\n";
		}
		uint16_t localAudioLevel = 0;
		for(size_t i=0; i!=hop_size; ++i)
		{
			smpl_t s;
			s = alsaBuffer[i*2];
			if(unsigned(std::abs(alsaBuffer[i*2])) > localAudioLevel)
				localAudioLevel = std::abs(alsaBuffer[i*2]);
			s += alsaBuffer[i*2+1];
			if(unsigned(std::abs(alsaBuffer[i*2+1])) > localAudioLevel)
				localAudioLevel = std::abs(alsaBuffer[i*2+1]);
			fvec_set_sample(ibuf, s, i);
		}
		_audioLevel = localAudioLevel;
		aubio_tempo_do(tempo, ibuf, tempo_out);
		smpl_t is_beat = fvec_get_sample(tempo_out, 0);
		if(silence_threshold != -90)
			is_silence = aubio_silence_detection(ibuf, silence_threshold);

		if(is_beat && !is_silence)
		{
			smpl_t confidence = aubio_tempo_get_confidence(tempo);
			if(confidence > _minimumConfidence)
			{
				Beat beat = _beat; // atomic load
				beat.confidence = confidence;
				// We do a simple reset on high numbers just to make sure that the 'float' precision does not run out.
				// With 8 beats per second, this would still take 24h to run out.
				if(beat.value < (60.0*60.0*24.0*8.0))
					++beat.value;
				else
					beat.value = 0.0;
				_beat = beat; // atomic store
			}
		}
		else if(is_silence)
		{
			Beat beat = _beat; // atomic load
			beat.confidence = 0.0;
			_beat = beat; // atomic load
		}
	}
	snd_pcm_drop(_handle);
	
  del_aubio_tempo(tempo);
  del_fvec(tempo_out);
	del_fvec(ibuf);
}

void BeatFinder::close()
{
	_isStopping = true;
	if(_alsaThread != nullptr)
	{
		_alsaThread->join();
		_alsaThread.reset();
	}
	if(_isOpen)
	{
		snd_pcm_close(_handle);
		_isOpen = false;
	}
}

