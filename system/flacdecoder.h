#ifndef SYSTEM_FLAC_DECODER_H_
#define SYSTEM_FLAC_DECODER_H_

#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

#include <FLAC++/decoder.h>

#include "lane.h"

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
        _decodeThread(),
        _isStopping(false),
        _lane(1024 * 1024 * 2) {
    FLAC__StreamDecoderInitStatus init_status = init(_filename);
    if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
      throw FlacError("ERROR initializing decoder");
    process_until_end_of_metadata();
  }

  ~FlacDecoder() { close(); }

  void Start() {
    _lane.clear();
    _decodeThread = std::thread([&]() { open(); });
  }

  void Seek(double offsetInMS) {
    if (!seek_absolute((FLAC__uint64)(44.1000 * offsetInMS)))
      throw FlacError("Seek failed");
  }

  void GetSamples(unsigned char *buffer, size_t &count) {
    count = _lane.read(buffer, count);
  }

  bool HasMore() const { return !_lane.is_end_and_empty(); }

 private:
  bool _isOpen;
  std::string _filename;
  std::thread _decodeThread;
  std::atomic<bool> _isStopping;
  aocommon::Lane<unsigned char> _lane;
  std::vector<unsigned char> _buffer;

  void open();
  void close();

  FLAC__StreamDecoderWriteStatus write_callback(
      const FLAC__Frame *frame, const FLAC__int32 *const buffer[]) override;
  void error_callback(FLAC__StreamDecoderErrorStatus status) override;
  // void metadata_callback(const ::FLAC__StreamMetadata *metadata) override;
};

}  // namespace glight::system

#endif
