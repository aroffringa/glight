#include "blackoutsceneitem.h"

#include "scene.h"

namespace glight::theatre {

void BlackoutSceneItem::Start(Scene& scene) {
  switch (operation_) {
    case BlackoutOperation::Blackout:
      scene.BlackOut(fade_speed_);
      break;
    case BlackoutOperation::Restore:
      scene.RestoreFromBlackout(fade_speed_);
      break;
  }
}

}  // namespace glight::theatre
