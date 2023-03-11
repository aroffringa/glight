#ifndef SYSTEM_AUDIOPLAYER_H_
#define SYSTEM_AUDIOPLAYER_H_

#include "flacdecoder.h"

#include <alsa/asoundlib.h>

#include <stdexcept>
#include <string>
#include <thread>

namespace glight::system {

class SyncListener {
 public:
  virtual ~SyncListener(){};
  /**
   * This call may not lock the mutex, as this can lead to
   * a deadlock when the audioplayer is closing with
   * locked mutex (e.g. through Scene::Mix()).
   */
  virtual void OnSyncUpdate(double offsetInMS) = 0;
};

class AudioPlayer : private SyncListener {
 public:
  class AlsaError : public std::runtime_error {
   public:
    AlsaError(const std::string &message)
        : runtime_error(std::string("Alsa error: ") + message) {}
  };

  AudioPlayer(FlacDecoder &decoder)
      : _alsaPeriodSize(256),
        _alsaBufferSize(2048),
        _alsaThread(),
        _isStopping(false),
        _isPlaying(false),
        _isOpen(false),
        _decoder(decoder),
        _startPosition(0) {
    _syncListener = this;
  }

  virtual ~AudioPlayer() {
    _isStopping = true;
    close();
  }

  bool IsPlaying() const { return _isPlaying; }

  void Play() {
    _decoder.Start();

    _alsaThread.reset(new std::thread([&]() { open(); }));
  }
  void Seek(double offsetInMS) {
    _decoder.Seek(offsetInMS);
    _startPosition = (unsigned)(offsetInMS * 44.100);
  }
  void SetSyncListener(SyncListener &listener) { _syncListener = &listener; }

 private:
  unsigned char _alsaBuffer[1024 * 64 * 4];
  snd_pcm_t *_handle;
  unsigned _alsaPeriodSize;
  unsigned _alsaBufferSize;
  std::unique_ptr<std::thread> _alsaThread;
  std::atomic<bool> _isStopping;
  std::atomic<bool> _isPlaying;
  bool _isOpen;
  FlacDecoder &_decoder;
  SyncListener *_syncListener;
  unsigned _startPosition;

  void open();
  void close();
  void OnSyncUpdate(double offsetInMS) {}
};

}  // namespace glight::system

#endif
