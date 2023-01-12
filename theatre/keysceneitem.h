#ifndef THEATRE_KEYSCENEITEM_H_
#define THEATRE_KEYSCENEITEM_H_

#include <sstream>

#include "sceneitem.h"

namespace glight::theatre {

enum KeySceneLevel { Key, Beat, Highlight, Measure, Section };

inline std::string ToString(KeySceneLevel level) {
  switch (level) {
    default:
    case KeySceneLevel::Key:
      return "key";
    case KeySceneLevel::Beat:
      return "beat";
    case KeySceneLevel::Highlight:
      return "highlight";
    case KeySceneLevel::Measure:
      return "measure";
    case KeySceneLevel::Section:
      return "section";
  }
}

inline KeySceneLevel GetKeySceneLevel(const std::string &str) {
  if (str == "beat")
    return KeySceneLevel::Beat;
  else if (str == "highlight")
    return KeySceneLevel::Highlight;
  else if (str == "measure")
    return KeySceneLevel::Measure;
  else if (str == "section")
    return KeySceneLevel::Section;
  else  // "key"
    return KeySceneLevel::Key;
}

/**
 * @author Andre Offringa
 */
class KeySceneItem final : public SceneItem {
 public:
  KeySceneItem() : _level(Key) {}
  ~KeySceneItem() {}

  KeySceneLevel Level() const { return _level; }
  void SetLevel(KeySceneLevel level) { _level = level; }

  std::string Description() const override {
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

  void Mix(const Timing &timing, bool primary) override{};

 private:
  KeySceneLevel _level;
};

}  // namespace glight::theatre

#endif
