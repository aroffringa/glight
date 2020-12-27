#ifndef KEYSCENEITEM_H
#define KEYSCENEITEM_H

#include <sstream>

#include "sceneitem.h"

/**
        @author Andre Offringa
*/
class KeySceneItem : public SceneItem {
public:
  enum Level { Key, Beat, Highlight, Measure, Section };

  KeySceneItem() : _level(Key) {}

  ~KeySceneItem() {}

  enum Level Level() const { return _level; }
  void SetLevel(enum Level level) { _level = level; }

  virtual std::string Description() const {
    switch (_level) {
    case Key:
    default:
      return "key";
    case Beat:
      return "beat";
    case Highlight:
      return "highlight";
    case Measure:
      return "-measure-";
    case Section:
      return "-=SECTION=-";
    }
  }
  virtual void Mix(unsigned *channelValues, unsigned universe,
                   const class Timing &timing) {}

private:
  enum Level _level;
};

#endif
