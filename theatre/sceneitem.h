#ifndef THEATRE_SCENEITEM_H_
#define THEATRE_SCENEITEM_H_

namespace glight::theatre {

class SceneItem {
 public:
  SceneItem() : _offsetInMS(0.0), _durationInMS(0.0) {}
  virtual ~SceneItem() {}

  double OffsetInMS() const { return _offsetInMS; }
  double DurationInMS() const { return _durationInMS; }

  void SetOffsetInMS(double offsetInMS) { _offsetInMS = offsetInMS; }
  void SetDurationInMS(double durationInMS) { _durationInMS = durationInMS; }

  virtual std::string Description() const = 0;

  virtual void Mix(unsigned *channelValues, unsigned universe,
                   const class Timing &timing) = 0;

 private:
  double _offsetInMS;
  double _durationInMS;
};

}  // namespace glight::theatre

#endif  // SCENEITEM_H
