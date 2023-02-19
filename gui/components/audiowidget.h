#ifndef GUI_AUDIOWIDGET_H_
#define GUI_AUDIOWIDGET_H_

#include <cmath>
#include <map>

#include <gtkmm/drawingarea.h>

#include <gdkmm/pixbuf.h>

#include "../../theatre/forwards.h"

namespace glight::system {
class FlacDecoder;
}

namespace glight::gui {

class AudioWidget : public Gtk::DrawingArea {
 public:
  AudioWidget();
  ~AudioWidget();

  void ClearAudioData();
  void SetAudioData(system::FlacDecoder &decoder);
  void SetPosition(double offsetInMS);
  double Position() const;
  /**
   * Mutex needs to be locked before calling
   */
  void SetScene(theatre::Scene &scene);
  void SetNoScene();
  sigc::signal<void(double)> SignalClicked() { return _signalClicked; }
  size_t DataSize() const { return _audioDataMax.size(); }

 private:
  enum KeyType { ItemStart, KeyStart };

  int _cursorPosition;
  int _renderStartPosition;
  // Glib::RefPtr<Gdk::GC> _drawingAreaGC;
  Glib::RefPtr<Gdk::Pixbuf> _buffer;
  int _width, _height;
  bool _isUpToDate;
  std::vector<int> _audioDataMax, _audioDataMin, _audioDataStdDev;
  std::vector<unsigned char> _chunkBuffer;
  sigc::signal<void(double)> _signalClicked;
  std::map<int, enum KeyType> _keys;

  void ResizeBuffer();
  void DrawBuffer(Glib::RefPtr<Gdk::Pixbuf> &buffer);
  void bufferToScreen(const Cairo::RefPtr<Cairo::Context> &context);
  bool onExpose(const Cairo::RefPtr<Cairo::Context> &context) {
    if (_width != get_width() || _height != get_height()) {
      ResizeBuffer();
      DrawBuffer(_buffer);
    } else {
      if (!_isUpToDate) DrawBuffer(_buffer);
    }
    if (_buffer) bufferToScreen(context);
    return true;
  }
  bool onButtonPressed(GdkEventButton *event);
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

}  // namespace glight::gui

#endif
