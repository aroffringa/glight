#ifndef SYSTEM_AUDIOPLAYER_H_
#define SYSTEM_AUDIOPLAYER_H_

#include "flacdecoder.h"

#include <alsa/asoundlib.h>

#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>

namespace glight::system {

class SyncListener {
 public:
  virtual ~SyncListener(){};
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
        _isOpen(false),
        _decoder(decoder),
        _startPosition(0) {
    _syncListener = this;
  }

  virtual ~AudioPlayer() {
    setStopping();
    close();
  }

  void Play() {
    _decoder.Start();

    AlsaThread alsaThreadFunc(*this);
    _alsaThread.reset(new std::thread(alsaThreadFunc));
  }
  void Seek(double offsetInMS) {
    _decoder.Seek(offsetInMS);
    _startPosition = (unsigned)(offsetInMS * 44.100);
  }
  void SetSyncListener(SyncListener &listener) { _syncListener = &listener; }

 private:
  struct AlsaThread {
   public:
    AudioPlayer &_player;
    AlsaThread(AudioPlayer &player) : _player(player) {}
    void operator()() { _player.open(); }
  };

  unsigned char _alsaBuffer[1024 * 64 * 4];
  snd_pcm_t *_handle;
  unsigned _alsaPeriodSize;
  unsigned _alsaBufferSize;
  std::unique_ptr<std::thread> _alsaThread;
  bool _isStopping;
  bool _isOpen;
  std::mutex _mutex;
  FlacDecoder &_decoder;
  SyncListener *_syncListener;
  unsigned _startPosition;

  void open();
  void close();
  bool isStopping() {
    std::unique_lock<std::mutex> lock(_mutex);
    return _isStopping;
  }
  void setStopping() {
    std::unique_lock<std::mutex> lock(_mutex);
    _isStopping = true;
  }
  void OnSyncUpdate(double offsetInMS) {}
};

}

#endif
