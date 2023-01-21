#include "audiowidget.h"

#include "../../system/flacdecoder.h"

#include "../../theatre/scene.h"
#include "../../theatre/sceneitem.h"

#include <cairomm/context.h>
#include <gdkmm/general.h>  // set_source_pixbuf()

namespace glight::gui {

AudioWidget::AudioWidget()
    : _centerPosition(0),
      _renderStartPosition(0),
      _buffer(nullptr),
      _width(0),
      _height(0),
      _isUpToDate(false),
      _chunkSize(800),
      _chunkBuffer(_chunkSize),
      _scene(nullptr) {
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
  _scene = nullptr;
  ClearAudioData();
}

void AudioWidget::SetAudioData(system::FlacDecoder &decoder) {
  _audioDataMax.clear();
  _audioDataMin.clear();
  _audioDataStdDev.clear();
  while (decoder.HasMore()) {
    size_t readSize = _chunkSize;
    decoder.GetSamples(_chunkBuffer.data(), readSize);
    _audioDataMax.push_back(getMax(_chunkBuffer.data(), readSize));
    _audioDataMin.push_back(getMin(_chunkBuffer.data(), readSize));
    _audioDataStdDev.push_back(getStdDev(_chunkBuffer.data(), readSize));
  }
  initialize();
  queue_draw();
}

void AudioWidget::initialize() {
  _width = get_width();
  if (_width > 0) {
    _height = get_height();
    _buffer =
        Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, _width, _height);
  } else {
    _buffer.clear();
  }
}

void AudioWidget::draw(Glib::RefPtr<Gdk::Pixbuf> &buffer) {
  int renderWidth = _width;
  if (renderWidth > static_cast<int>(DataSize())) renderWidth = DataSize();
  _renderStartPosition = _centerPosition - renderWidth / 2;
  if (_renderStartPosition < 0) _renderStartPosition = 0;
  if (_renderStartPosition + renderWidth / 2 > static_cast<int>(DataSize()))
    _renderStartPosition = DataSize() - renderWidth / 2;

  if (buffer) {
    guint8 *data = buffer->get_pixels();
    size_t rowStride = buffer->get_rowstride();
    for (int x = 0; x < renderWidth; ++x) {
      int xDataPos = x + _renderStartPosition;
      guint8 *xa = data + x * 3;
      int yStart = (_height / 2) - (_audioDataMax[xDataPos] * _height) / 65536;
      int yStd1 =
          (_height / 2) - (_audioDataStdDev[xDataPos] * _height) / 65536;
      int yStd2 =
          (_audioDataStdDev[xDataPos] * _height) / 65536 + (_height / 2);
      int yEnd = (_height / 2) - (_audioDataMin[xDataPos] * _height) / 65536;
      if (yStd1 > _height / 2) yStd1 = _height / 2;
      if (yStart > yStd1) yStart = yStd1;
      if (yStd1 < 0) yStd1 = 0;
      if (yStd2 > _height) yStd2 = _height;
      if (yStd2 <= _height / 2) yStd2 = _height / 2 + 1;
      if (yEnd > _height) yEnd = _height;
      if (yEnd < yStd2) yEnd = yStd2;
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
    verticalLine(data, rowStride, _centerPosition - _renderStartPosition - 1,
                 255, 0, 0);
    verticalLine(data, rowStride, _centerPosition - _renderStartPosition, 255,
                 0, 0);
    verticalLine(data, rowStride, _centerPosition - _renderStartPosition + 1,
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
  int position = (event->x + _renderStartPosition);
  if (position >= static_cast<int>(DataSize())) position = DataSize() - 1;
  if (position < 0) position = 0;
  _signalClicked.emit(static_cast<double>(position) * _chunkSize /
                      (44.100 * 4.0));
  return true;
}

void AudioWidget::UpdateKeys() {
  _keys.clear();

  if (_scene) {
    const std::multimap<double, std::unique_ptr<theatre::SceneItem>> &items =
        _scene->SceneItems();
    for (const std::pair<const double, std::unique_ptr<theatre::SceneItem>> &p :
         items) {
      theatre::SceneItem *item = p.second.get();
      theatre::KeySceneItem *key = dynamic_cast<theatre::KeySceneItem *>(item);
      if (key != nullptr)
        _keys.insert(std::pair<int, KeyType>(
            static_cast<int>(
                round(item->OffsetInMS() * 44.100 * 4.0 / _chunkSize)),
            KeyStart));
      else
        _keys.insert(std::pair<int, KeyType>(
            static_cast<int>(
                round(item->OffsetInMS() * 44.100 * 4.0 / _chunkSize)),
            ItemStart));
    }
  }
  _isUpToDate = false;
  queue_draw();
}

}  // namespace glight::gui
