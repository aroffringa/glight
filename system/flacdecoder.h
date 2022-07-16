#ifndef SYSTEM_FLAC_DECODER_H_
#define SYSTEM_FLAC_DECODER_H_

#include <iostream>
#include <stdexcept>

#include <condition_variable>
#include <mutex>
#include <thread>

#include <FLAC++/decoder.h>

namespace glight::system {

class FlacDecoder : private FLAC::Decoder::File {
 public:
  class FlacError : public std::runtime_error {
   public:
    FlacError(const std::string &message)
        : runtime_error(std::string("Alsa error: ") + message) {}
  };

  FlacDecoder(const std::string &filename)
      : _isOpen(false),
        _filename(filename),
        _writePos(0),
        _activeWriteBuffer(0),
        _readPos(0),
        _activeReadBuffer(0),
        _decodeThread(),
        _isStopping(false),
        _bufferSize(1024 * 1024) {
    _buffer[0] = new unsigned char[_bufferSize];
    _buffer[1] = new unsigned char[_bufferSize];
    FLAC__StreamDecoderInitStatus init_status = init(_filename);
    if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
      throw FlacError("ERROR initializing decoder");
    process_until_end_of_metadata();
  }
  ~FlacDecoder() {
    setStopping();
    close();
    delete[] _buffer[1];
    delete[] _buffer[0];
  }
  void Start() {
    _bufferReady[0] = false;
    _bufferReady[1] = false;
    DecodeThread decodeThreadFunc(*this);
    _decodeThread.reset(new std::thread(decodeThreadFunc));
    std::cout << "Thread started." << std::endl;
    waitForReadReady(0);
    std::cout << "waitForReadReady(0) returned." << std::endl;
  }
  void Seek(double offsetInMS) {
    if (!seek_absolute((FLAC__uint64)(44.1000 * offsetInMS)))
      throw FlacError("Seek failed");
  }
  void GetSamples(unsigned char *buffer, size_t &count);
  bool HasMore() { return hasMore(); }

 private:
  struct DecodeThread {
   public:
    FlacDecoder &_decoder;
    DecodeThread(FlacDecoder &decoder) : _decoder(decoder) {}
    void operator()() {
      _decoder.open();
      std::cout << "Decode thread finished." << std::endl;
    }
  };
  bool _isOpen;
  bool _hasMore;
  std::string _filename;
  unsigned char *_buffer[2];
  bool _bufferReady[2];
  unsigned _writePos, _activeWriteBuffer;
  unsigned _readPos, _activeReadBuffer;
  std::unique_ptr<std::thread> _decodeThread;
  std::condition_variable _bufferReadyCondition;
  bool _isStopping;
  std::mutex _mutex;
  const unsigned _bufferSize;

  bool isStopping() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _isStopping;
  }
  void setStopping() {
    std::unique_lock<std::mutex> lock(_mutex);
    _isStopping = true;
    lock.unlock();
    _bufferReadyCondition.notify_all();
  }
  bool hasMore() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _hasMore;
  }
  void setHasMore(bool newValue) {
    std::lock_guard<std::mutex> lock(_mutex);
    _hasMore = newValue;
  }

  void open();
  void close();

  void waitForReadReady(unsigned buffer) {
    std::unique_lock<std::mutex> lock(_mutex);
    while (!_bufferReady[buffer] && !_isStopping) {
      _bufferReadyCondition.wait(lock);
    }
    _bufferReady[buffer] = false;
    _activeReadBuffer = buffer;
  }
  void setReadyForReading(unsigned buffer) {
    std::unique_lock<std::mutex> lock(_mutex);
    _bufferReady[buffer] = true;
    _activeWriteBuffer = 1 - buffer;
    lock.unlock();
    _bufferReadyCondition.notify_all();
  }
  void waitForWriteReady(unsigned buffer) {
    std::unique_lock<std::mutex> lock(_mutex);
    while (_activeReadBuffer == buffer && !_isStopping) {
      _bufferReadyCondition.wait(lock);
    }
    _activeWriteBuffer = buffer;
  }
  void setReadyForWriting(unsigned buffer) {
    std::unique_lock<std::mutex> lock(_mutex);
    _activeReadBuffer = 1 - buffer;
    lock.unlock();
    _bufferReadyCondition.notify_all();
  }
  unsigned activeWriteBuffer() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _activeWriteBuffer;
  }
  unsigned activeReadBuffer() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _activeReadBuffer;
  }
  virtual FLAC__StreamDecoderWriteStatus write_callback(
      const FLAC__Frame *frame, const FLAC__int32 *const buffer[]);
  virtual void error_callback(FLAC__StreamDecoderErrorStatus status);
  virtual void metadata_callback(const ::FLAC__StreamMetadata *metadata);
};

}  // namespace glight::system

#endif
