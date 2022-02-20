#ifndef AUDIOWIDGET_H
#define AUDIOWIDGET_H

#include <cmath>
#include <map>

#include <gtkmm/drawingarea.h>

#include <gdkmm/pixbuf.h>

/**
        @author Andre Offringa
*/
class AudioWidget : public Gtk::DrawingArea {
 public:
  AudioWidget();
  ~AudioWidget();

  void SetAudioData(class FlacDecoder &decoder);
  void SetPosition(double offsetInMS) {
    _centerPosition = (offsetInMS * 44.100 * 4.0) / _chunkSize;
    _isUpToDate = false;
    queue_draw();
  }
  double Position() const {
    return _centerPosition * _chunkSize / (44.100 * 4.0);
  }
  void SetScene(class Scene &scene) { _scene = &scene; }
  void SetNoScene() { _scene = 0; }
  sigc::signal<void, double> SignalClicked() { return _signalClicked; }
  size_t DataSize() const { return _audioDataMax.size(); }

  /**
   * Mutex needs to be locked before calling
   */
  void UpdateKeys();

 private:
  enum KeyType { ItemStart, KeyStart };

  int _centerPosition;
  int _renderStartPosition;
  // Glib::RefPtr<Gdk::GC> _drawingAreaGC;
  Glib::RefPtr<Gdk::Pixbuf> _buffer;
  int _width, _height;
  bool _isUpToDate;
  std::vector<int> _audioDataMax, _audioDataMin, _audioDataStdDev;
  const int _chunkSize;
  std::vector<unsigned char> _chunkBuffer;
  sigc::signal<void, double> _signalClicked;
  class Scene *_scene;
  std::map<int, enum KeyType> _keys;

  void initialize();
  void draw(const Cairo::RefPtr<Cairo::Context> &context);
  void bufferToScreen(const Cairo::RefPtr<Cairo::Context> &context);
  bool onExpose(const Cairo::RefPtr<Cairo::Context> &context) {
    if ((_width != get_width() || _height != get_height()) && _height > 0 &&
        _width > 0) {
      initialize();
      _isUpToDate = false;
    }
    if (!_isUpToDate) draw(context);
    if (_buffer) bufferToScreen(context);
    return true;
  }
  bool onButtonPressed(GdkEventButton *event);
  int intValue(const unsigned char *chunk, unsigned pos) const {
    return (((signed char)chunk[pos + 1]) << 8) + (signed int)chunk[pos];
  }
  int getMax(const unsigned char *chunk, unsigned size) const {
    int max = intValue(chunk, 0);
    for (unsigned i = 2; i < size; i += 2) {
      if (max < intValue(chunk, i)) max = intValue(chunk, i);
    }
    return max;
  }
  int getMin(const unsigned char *chunk, unsigned size) const {
    int min = intValue(chunk, 0);
    for (unsigned i = 2; i < size; i += 2) {
      if (min > intValue(chunk, i)) min = intValue(chunk, i);
    }
    return min;
  }
  int getStdDev(const unsigned char *chunk, unsigned size) const {
    double avg = 0.0;
    for (unsigned i = 0; i < size; i += 2) avg += intValue(chunk, i);
    avg /= size;
    double val = 0.0;
    for (unsigned i = 0; i < size; i += 2) {
      double v = (double)intValue(chunk, i) - avg;
      val += v * v;
    }
    val /= size;
    return round(std::sqrt(val));
  }
  void setColor(guint8 *dataPtr, unsigned char r, unsigned char g,
                unsigned char b) {
    dataPtr[0] = r;
    dataPtr[1] = g;
    dataPtr[2] = b;
  }
  void verticalLine(guint8 *dataPtr, size_t rowStride, int x, unsigned char r,
                    unsigned char g, unsigned char b) {
    if (x >= 0 && x < _width) {
      guint8 *xPtr = dataPtr + x * 3;
      for (int y = 0; y < _height; ++y) {
        guint8 *v = xPtr + y * rowStride;
        v[0] = r;
        v[1] = g;
        v[2] = b;
      }
    }
  }
};

#endif
