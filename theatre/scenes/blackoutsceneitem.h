#ifndef THEATRE_BLACKOUT_SCENE_ITEM_H_
#define THEATRE_BLACKOUT_SCENE_ITEM_H_

#include <sstream>

#include "sceneitem.h"

namespace glight::theatre {

enum class BlackoutOperation { Blackout, Restore };

class Scene;

inline std::string ToString(BlackoutOperation operation) {
  switch (operation) {
    default:
    case BlackoutOperation::Blackout:
      return "blackout";
    case BlackoutOperation::Restore:
      return "restore";
  }
}

inline BlackoutOperation GetBlackoutOperation(const std::string& str) {
  if (str == "blackout")
    return BlackoutOperation::Blackout;
  else  // if (str == "highlight")
    return BlackoutOperation::Restore;
}

/**
 * @author Andre Offringa
 */
class BlackOutSceneItem final : public SceneItem {
 public:
  BlackOutSceneItem() : operation_(BlackoutOperation::Blackout) {}
  ~BlackOutSceneItem() {}

  BlackoutOperation Operation() const { return operation_; }
  void SetOperation(BlackoutOperation operation) { operation_ = operation; }

  double FadeSpeed() const { return fade_speed_; }
  void SetFadeSpeed(double fade_speed) { fade_speed_ = fade_speed; }

  std::string Description() const override {
    switch (operation_) {
      case BlackoutOperation::Blackout:
      default:
        return "blackout";
      case BlackoutOperation::Restore:
        return "restore from blackout";
    }
  }

  void Start(Scene& scene) override;

 private:
  BlackoutOperation operation_;
  double fade_speed_ = 0;
};

}  // namespace glight::theatre

#endif
