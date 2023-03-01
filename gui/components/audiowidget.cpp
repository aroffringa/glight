#include "audiowidget.h"

#include "../../system/flacdecoder.h"

#include "../../theatre/scenes/scene.h"
#include "../../theatre/scenes/sceneitem.h"

#include <cairomm/context.h>

#include <gdkmm/general.h>  // set_source_pixbuf()

namespace {

inline constexpr size_t kChunkSize = 800;

void setColor(guint8 *dataPtr, unsigned char r, unsigned char g,
              unsigned char b) {
  dataPtr[0] = r;
  dataPtr[1] = g;
  dataPtr[2] = b;
}

int intValue(const unsigned char *chunk, unsigned pos) {
  return (((signed char)chunk[pos + 1]) << 8) + (signed int)chunk[pos];
}

int getMax(const unsigned char *chunk, unsigned size) {
  int max = intValue(chunk, 0);
  for (unsigned i = 2; i < size; i += 2) {
    if (max < intValue(chunk, i)) max = intValue(chunk, i);
  }
  return max;
}

int getMin(const unsigned char *chunk, unsigned size) {
  int min = intValue(chunk, 0);
  for (unsigned i = 2; i < size; i += 2) {
    if (min > intValue(chunk, i)) min = intValue(chunk, i);
  }
  return min;
}

int getStdDev(const unsigned char *chunk, unsigned size) {
  double avg = 0.0;
  for (unsigned i = 0; i < size; i += 2) avg += intValue(chunk, i);
  avg /= size;
  double val = 0.0;
  for (unsigned i = 0; i < size; i += 2) {
    double v = (double)intValue(chunk, i) - avg;
    val += v * v;
  }
  val /= size;
  return std::round(std::sqrt(val));
}

}  // namespace

namespace glight::gui {

AudioWidget::AudioWidget()
    : _cursorPosition(0),
      _renderStartPosition(0),
      _buffer(nullptr),
      _width(0),
      _height(0),
      _isUpToDate(false),
      _chunkBuffer(kChunkSize) {
  set_size_request(50, 50);

  add_events(Gdk::BUTTON_PRESS_MASK);

  signal_button_press_event().connect(
      sigc::mem_fun(*this, &AudioWidget::onButtonPressed));

  signal_draw().connect(sigc::mem_fun(*this, &AudioWidget::onExpose));
}

AudioWidget::~AudioWidget() = default;

void AudioWidget::ClearAudioData() {
  _audioDataMax.clear();
  _audioDataMin.clear();
  _audioDataStdDev.clear();
  _isUpToDate = false;
  queue_draw();
}

void AudioWidget::SetNoScene() {
  _keys.clear();
  _audioDataMax.clear();
  _audioDataMin.clear();
  _audioDataStdDev.clear();
  _isUpToDate = false;
  queue_draw();
}

void AudioWidget::SetAudioData(system::FlacDecoder &decoder) {
  _audioDataMax.clear();
  _audioDataMin.clear();
  _audioDataStdDev.clear();
  while (decoder.HasMore()) {
    size_t readSize = kChunkSize;
    decoder.GetSamples(_chunkBuffer.data(), readSize);
    _audioDataMax.push_back(getMax(_chunkBuffer.data(), readSize));
    _audioDataMin.push_back(getMin(_chunkBuffer.data(), readSize));
    _audioDataStdDev.push_back(getStdDev(_chunkBuffer.data(), readSize));
  }
  _isUpToDate = false;
  queue_draw();
}

void AudioWidget::SetPosition(double offsetInMS) {
  _cursorPosition = (offsetInMS * 44.100 * 4.0) / kChunkSize;
  _isUpToDate = false;
  queue_draw();
}

double AudioWidget::Position() const {
  return _cursorPosition * kChunkSize / (44.100 * 4.0);
}

void AudioWidget::ResizeBuffer() {
  _width = get_width();
  _height = get_height();
  if (_width > 0) {
    _buffer =
        Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, _width, _height);
  } else {
    _buffer.clear();
  }
}

void AudioWidget::DrawBuffer(Glib::RefPtr<Gdk::Pixbuf> &buffer) {
  _renderStartPosition = std::max(_cursorPosition - _width / 2, 0);
  const int renderWidth = std::max(
      0, std::min(_width, static_cast<int>(DataSize()) - _renderStartPosition));

  if (buffer) {
    guint8 *data = buffer->get_pixels();
    size_t rowStride = buffer->get_rowstride();
    for (int x = 0; x < renderWidth; ++x) {
      int xDataPos = x + _renderStartPosition;
      guint8 *xa = data + x * 3;
      const int yStd1 = std::clamp(
          (_height / 2) - (_audioDataStdDev[xDataPos] * _height) / 65536, 0,
          _height / 2);
      const int yStd2 = std::clamp(
          (_audioDataStdDev[xDataPos] * _height) / 65536 + (_height / 2),
          _height / 2 + 1, _height);
      const int yStart = std::min(
          (_height / 2) - (_audioDataMax[xDataPos] * _height) / 65536, yStd1);
      const int yEnd = std::max(
          ((_height / 2) - (_audioDataMin[xDataPos] * _height) / 65536), yStd2);
      for (int y = 0; y < yStart; ++y)
        setColor(xa + rowStride * y, 255, 255, 255);
      for (int y = yStart; y < yStd1; ++y)
        setColor(xa + rowStride * y, 0, 0, 255);
      for (int y = yStd1; y < _height / 2; ++y)
        setColor(xa + rowStride * y, 0, 0, 127);
      setColor(xa + rowStride * (_height / 2), 0, 0, 0);
      for (int y = _height / 2 + 1; y < yStd2; ++y)
        setColor(xa + rowStride * y, 0, 0, 127);
      for (int y = yStd2; y < yEnd; ++y)
        setColor(xa + rowStride * y, 0, 0, 255);
      for (int y = yEnd; y < _height; ++y)
        setColor(xa + rowStride * y, 255, 255, 255);
    }
    // Set any remaining part to white
    for (int y = 0; y < _height; ++y) {
      guint8 *data_ptr = data + renderWidth * 3 + rowStride * y;
      for (int x = renderWidth; x != _width; ++x) {
        setColor(data_ptr, 255, 255, 255);
        data_ptr += 3;
      }
    }
    verticalLine(data, rowStride, _cursorPosition - _renderStartPosition - 1,
                 255, 0, 0);
    verticalLine(data, rowStride, _cursorPosition - _renderStartPosition, 255,
                 0, 0);
    verticalLine(data, rowStride, _cursorPosition - _renderStartPosition + 1,
                 255, 0, 0);

    std::map<int, enum KeyType>::const_iterator i =
        _keys.lower_bound(_renderStartPosition);
    while (i != _keys.end() && i->first < _renderStartPosition + renderWidth) {
      switch (i->second) {
        case KeyStart:
          verticalLine(data, rowStride, i->first - _renderStartPosition - 1, 0,
                       128, 0);
          verticalLine(data, rowStride, i->first - _renderStartPosition, 0, 128,
                       0);
          verticalLine(data, rowStride, i->first - _renderStartPosition + 1, 0,
                       128, 0);
          break;
        case ItemStart:
          verticalLine(data, rowStride, i->first - _renderStartPosition, 0, 255,
                       0);
          break;
      }
      ++i;
    }
  }
  _isUpToDate = true;
}

void AudioWidget::bufferToScreen(const Cairo::RefPtr<Cairo::Context> &context) {
  Gdk::Cairo::set_source_pixbuf(context, _buffer);
  context->rectangle(0, 0, _width, _height);
  context->fill();
}

bool AudioWidget::onButtonPressed(GdkEventButton *event) {
  const int position = std::clamp<int>(
      static_cast<int>(event->x) + _renderStartPosition, 0, DataSize() - 1);
  _signalClicked.emit(static_cast<double>(position) * kChunkSize /
                      (44.100 * 4.0));
  return true;
}

void AudioWidget::SetScene(theatre::Scene &scene) {
  _keys.clear();

  const std::multimap<double, std::unique_ptr<theatre::SceneItem>> &items =
      scene.SceneItems();
  for (const std::pair<const double, std::unique_ptr<theatre::SceneItem>> &p :
       items) {
    theatre::SceneItem *item = p.second.get();
    theatre::KeySceneItem *key = dynamic_cast<theatre::KeySceneItem *>(item);
    if (key != nullptr)
      _keys.insert(std::pair<int, KeyType>(
          static_cast<int>(
              std::round(item->OffsetInMS() * 44.100 * 4.0 / kChunkSize)),
          KeyStart));
    else
      _keys.insert(std::pair<int, KeyType>(
          static_cast<int>(
              std::round(item->OffsetInMS() * 44.100 * 4.0 / kChunkSize)),
          ItemStart));
  }
  _isUpToDate = false;
  queue_draw();
}

}  // namespace glight::gui
