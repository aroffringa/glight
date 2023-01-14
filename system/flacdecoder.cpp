#include "flacdecoder.h"

namespace glight::system {

void FlacDecoder::open() {
  if (_isOpen) throw std::runtime_error("Flac was opened twice");

  _isOpen = true;
  _hasMore = true;
  process_until_end_of_stream();
  setStopping();
}

void FlacDecoder::close() {
  if (_decodeThread != nullptr) {
    _decodeThread->join();
    _decodeThread.reset();
  }
  _isOpen = false;
}

FLAC__StreamDecoderWriteStatus FlacDecoder::write_callback(
    const FLAC__Frame *frame, const FLAC__int32 *const buffer[]) {
  if (isStopping()) return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

  unsigned actWriteBuffer = activeWriteBuffer();
  unsigned char *writeBuffer = _buffer[actWriteBuffer];
  for (unsigned i = 0; i < frame->header.blocksize; i++) {
    if (_writePos >= _bufferSize) {
      setReadyForReading(actWriteBuffer);
      actWriteBuffer = 1 - actWriteBuffer;
      waitForWriteReady(actWriteBuffer);
      writeBuffer = _buffer[actWriteBuffer];
      _writePos = 0;
    }
    writeBuffer[_writePos] = (static_cast<FLAC__int16>(buffer[0][i]));  // left channel
    ++_writePos;
    writeBuffer[_writePos] = (static_cast<FLAC__int16>(buffer[0][i])) >> 8;  // left channel
    ++_writePos;
    writeBuffer[_writePos] = (static_cast<FLAC__int16>(buffer[1][i]));  // right channel
    ++_writePos;
    writeBuffer[_writePos] = (static_cast<FLAC__int16>(buffer[1][i])) >> 8;  // right channel
    ++_writePos;
  }
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void FlacDecoder::GetSamples(unsigned char *buffer, size_t &count) {
  unsigned actReadBuffer = activeReadBuffer();
  unsigned char *readBuffer = _buffer[actReadBuffer];
  if (isStopping() && actReadBuffer == activeWriteBuffer()) {
    size_t stillAvailable = _writePos - _readPos;
    if (stillAvailable < count) {
      count = stillAvailable;
      setHasMore(false);
    }
  }
  for (size_t i = 0; i < count; ++i) {
    if (_readPos >= _bufferSize) {
      setReadyForWriting(actReadBuffer);
      actReadBuffer = 1 - actReadBuffer;
      waitForReadReady(actReadBuffer);
      readBuffer = _buffer[actReadBuffer];
      _readPos = 0;
      if (isStopping() && actReadBuffer == activeWriteBuffer()) {
        size_t stillToWrite = count - i;
        size_t stillAvailable = _writePos;
        if (stillAvailable < stillToWrite) {
          count = i + stillAvailable;
          setHasMore(false);
        }
      }
    }
    buffer[i] = readBuffer[_readPos];
    ++_readPos;
  }
}

void FlacDecoder::error_callback(FLAC__StreamDecoderErrorStatus  /*status*/) {
  std::cout << "Decoder reported an error" << std::endl;
}

void FlacDecoder::metadata_callback(const ::FLAC__StreamMetadata *metadata) {
  if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
    // unsigned total_samples = metadata->data.stream_info.total_samples;
    unsigned sample_rate = metadata->data.stream_info.sample_rate;
    unsigned channels = metadata->data.stream_info.channels;
    unsigned bps = metadata->data.stream_info.bits_per_sample;

    std::cout << "sample rate    : " << sample_rate
              << "\nchannels       : " << channels
              << "\nbits per sample: " << bps << std::endl;
  }
}

}  // namespace glight::system
