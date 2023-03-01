#ifndef THEATRE_SCENE_H_
#define THEATRE_SCENE_H_

#include <algorithm>
#include <atomic>
#include <set>
#include <map>
#include <mutex>

#include "../../system/audioplayer.h"

#include "blackoutsceneitem.h"
#include "controlsceneitem.h"
#include "keysceneitem.h"

#include "../sourcevaluestore.h"

namespace glight::theatre {

class Management;

/**
 * @author Andre Offringa
 */
class Scene : public Controllable, private system::SyncListener {
 private:
  constexpr static int kWaitSyncs = 5;

 public:
  Scene(Management &management);
  ~Scene();

  double StartTimeInMS() const { return _startTimeInMS; }

  /**
   * This method is thread safe and thus may be called from a
   * thread other than the management thread.
   */
  bool IsPlaying() const {
    return _isPlaying && ((_audioPlayer && _audioPlayer->IsPlaying()) ||
                          _endOfItems < kWaitSyncs);
  }

  ControlSceneItem *AddControlSceneItem(double offsetInMS,
                                        Controllable &controllable,
                                        size_t input);

  KeySceneItem *AddKeySceneItem(double offsetInMS);

  BlackoutSceneItem &AddBlackoutItem(double offsetInMS);

  void ChangeSceneItemStartTime(SceneItem *item, double newOffsetInMS);

  const std::multimap<double, std::unique_ptr<SceneItem>> &SceneItems() const {
    return _items;
  }

  size_t NInputs() const override { return 1; }

  ControlValue &InputValue(size_t) override { return input_value_; }

  FunctionType InputType(size_t) const override { return FunctionType::Master; }

  size_t NOutputs() const override { return controllables_.size(); }

  std::pair<const Controllable *, size_t> Output(size_t index) const override {
    return controllables_[index];
  }

  void Mix(const Timing &timing, bool primary) override {
    if (InputValue(0)) {
      if (primary && !_isPlaying) Start(timing.TimeInMS());
      const double relTimeInMs = timing.TimeInMS() - StartTimeInMS();
      const Timing relTiming(relTimeInMs, timing.TimestepNumber(),
                             timing.BeatValue(), timing.AudioLevel(),
                             timing.TimestepRandomValue());
      skipTo(relTimeInMs);

      for (SceneItem *scene_item : _startedItems) {
        scene_item->Mix(relTiming, primary);
      }
    } else if (primary && _isPlaying) {
      Stop();
    }
  }

  void Remove(SceneItem *item);

  /**
   * Allows skipping part of the scene and starting the scene at a later point.
   * This is e.g. used to start a scene from the selected point in the scene
   * window.
   */
  void SetStartOffset(double offsetInMS) {
    if (offsetInMS != _startOffset) {
      _startOffset = offsetInMS;
      initPlayer();
    }
  }

  bool HasAudio() const { return _hasAudio; }
  const std::string &AudioFile() const { return _audioFilename; }
  void SetNoAudioFile() {
    _hasAudio = false;
    _audioFilename = "";
    Stop();
  }
  void SetAudioFile(const std::string &audioFilename) {
    _audioFilename = audioFilename;
    _hasAudio = true;
    initPlayer();
  }

  void BlackOut(double fade_speed);

  void RestoreFromBlackout(double fade_speed);

 protected:
  void Start(double timeInMS);

  void Stop();

  /**
   * True when the last item has been played (irrespectively of
   * whether the audio player is still playing).
   */
  bool ItemsHaveEnd() const {
    return _nextStartedItem == _items.end() && _startedItems.empty();
  }

  void initPlayer();

  void resetCurrentOffset() {
    _nextStartedItem = _items.begin();
    _startedItems.clear();
  }

  void skipTo(double offsetInMS);

  std::multimap<double, std::unique_ptr<SceneItem>>::iterator find(
      SceneItem *item) {
    for (std::multimap<double, std::unique_ptr<SceneItem>>::iterator i =
             _items.begin();
         i != _items.end(); ++i) {
      if (item == i->second.get()) {
        return i;
      }
    }
    return _items.end();
  }

  void RecalculateControllables();

 private:
  Management &_management;
  std::mutex &_mutex;
  ControlValue input_value_;
  std::vector<SceneItem *> _startedItems;
  /**
   * Set of controllables that is used in this scene.
   */
  std::vector<std::pair<const Controllable *, size_t>> controllables_;
  using ItemMap = std::multimap<double, std::unique_ptr<SceneItem>>;
  ItemMap _items;
  ItemMap::iterator _nextStartedItem;
  double _currentOffset, _startOffset;
  std::unique_ptr<system::FlacDecoder> _decoder;
  std::unique_ptr<system::AudioPlayer> _audioPlayer;
  bool _hasAudio;
  std::atomic<bool> _isPlaying;
  std::atomic<int> _endOfItems;
  std::string _audioFilename;
  double _startTimeInMS;
  SourceValueStore _storedSourceValues;

  void OnSyncUpdate(double offsetInMS) override;
};

}  // namespace glight::theatre

#endif
