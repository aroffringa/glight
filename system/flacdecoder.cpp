#include "flacdecoder.h"

#include <iostream>

namespace glight::system {

void FlacDecoder::open() {
  if (_isOpen) throw std::runtime_error("Flac was opened twice");

  _isOpen = true;
  process_until_end_of_stream();
  _lane.write_end();
}

void FlacDecoder::close() {
  _lane.write_end();
  while (!_lane.empty()) _lane.discard(_lane.capacity());
  if (_decodeThread.joinable()) _decodeThread.join();
  _isOpen = false;
}

FLAC__StreamDecoderWriteStatus FlacDecoder::write_callback(
    const FLAC__Frame *frame, const FLAC__int32 *const buffer[]) {
  if (_lane.is_end()) return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

  _buffer.clear();
  _buffer.reserve(frame->header.blocksize * 4);
  for (unsigned i = 0; i < frame->header.blocksize; i++) {
    _buffer.emplace_back(
        static_cast<FLAC__int16>(buffer[0][i]));  // left channel
    _buffer.emplace_back((static_cast<FLAC__int16>(buffer[0][i])) >>
                         8);  // left channel
    _buffer.emplace_back(
        static_cast<FLAC__int16>(buffer[1][i]));  // right channel
    _buffer.emplace_back((static_cast<FLAC__int16>(buffer[1][i])) >>
                         8);  // right channel
  }
  _lane.write(_buffer.data(), _buffer.size());
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void FlacDecoder::error_callback(FLAC__StreamDecoderErrorStatus /*status*/) {
  std::cout << "Decoder reported an error" << std::endl;
}

/*void FlacDecoder::metadata_callback(const ::FLAC__StreamMetadata *metadata) {
  if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
    // unsigned total_samples = metadata->data.stream_info.total_samples;
    // unsigned sample_rate = metadata->data.stream_info.sample_rate;
    // unsigned channels = metadata->data.stream_info.channels;
    // unsigned bps = metadata->data.stream_info.bits_per_sample;
  }
}*/

}  // namespace glight::system
