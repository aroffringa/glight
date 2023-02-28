#ifndef THEATRE_SCENEITEM_H_
#define THEATRE_SCENEITEM_H_

namespace glight::theatre {

class Scene;
class Timing;

class SceneItem {
 public:
  SceneItem() = default;
  virtual ~SceneItem() noexcept = default;

  double OffsetInMS() const { return _offsetInMS; }
  double DurationInMS() const { return _durationInMS; }

  void SetOffsetInMS(double offsetInMS) { _offsetInMS = offsetInMS; }
  void SetDurationInMS(double durationInMS) { _durationInMS = durationInMS; }

  virtual std::string Description() const = 0;

  /**
   * This is for scene items that need to perform a one-time action
   * once the scene item's starting time has reached.
   */
  virtual void Start(Scene& scene) {}
  
  virtual void Mix(const Timing &timing, bool primary) {}

 private:
  double _offsetInMS = 0.0;
  double _durationInMS = 0.0;
};

}  // namespace glight::theatre

#endif  // SCENEITEM_H
