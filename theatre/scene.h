#ifndef THEATRE_SCENE_H_
#define THEATRE_SCENE_H_

#include <algorithm>
#include <atomic>
#include <set>
#include <map>
#include <mutex>

#include "../system/audioplayer.h"

#include "controlsceneitem.h"
#include "keysceneitem.h"

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
    return _isPlaying &&
           (_audioPlayer->IsPlaying() || _endOfItems < kWaitSyncs);
  }

  ControlSceneItem *AddControlSceneItem(double offsetInMS,
                                        Controllable &controllable,
                                        size_t input) {
    std::unique_ptr<ControlSceneItem> item =
        std::make_unique<ControlSceneItem>(controllable, input);
    item->SetOffsetInMS(offsetInMS);
    ControlSceneItem *result = item.get();
    _items.emplace(offsetInMS, std::move(item));
    resetCurrentOffset();
    std::pair<const Controllable *, size_t> value(&controllable, input);
    if (std::find(controllables_.begin(), controllables_.end(), value) ==
        controllables_.end()) {
      controllables_.emplace_back(value);
    }
    return result;
  }

  KeySceneItem *AddKeySceneItem(double offsetInMS) {
    std::unique_ptr<KeySceneItem> item = std::make_unique<KeySceneItem>();
    item->SetOffsetInMS(offsetInMS);
    KeySceneItem *result = item.get();
    _items.emplace(offsetInMS, std::move(item));
    resetCurrentOffset();
    return result;
  }

  void ChangeSceneItemStartTime(SceneItem *item, double newOffsetInMS) {
    ItemMap::node_type node = _items.extract(find(item));
    node.mapped()->SetOffsetInMS(newOffsetInMS);
    node.key() = newOffsetInMS;
    _items.insert(std::move(node));
    resetCurrentOffset();
  }

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

  void Remove(SceneItem *item) {
    std::multimap<double, std::unique_ptr<SceneItem>>::iterator iter =
        find(item);
    // Make sure the object is deleted only at the end of this function:
    std::unique_ptr<SceneItem> owned_item = std::move(iter->second);
    _items.erase(iter);
    if (ControlSceneItem *c_item = dynamic_cast<ControlSceneItem *>(item);
        c_item) {
      RecalculateControllables();
    }
  }

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

 protected:
  void Start(double timeInMS) {
    _startTimeInMS = timeInMS;
    resetCurrentOffset();
    Stop();
    _startTimeInMS = _startTimeInMS - _startOffset;
    _isPlaying = true;
    _endOfItems = 0;
    if (_hasAudio) _audioPlayer->Play();
  }

  void Stop() {
    if (_isPlaying) {
      _audioPlayer.reset();
      _decoder.reset();
      _isPlaying = false;
      _startOffset = 0.0;
      initPlayer();
    } else {
      if (_audioPlayer == nullptr) initPlayer();
    }
  }

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

  void skipTo(double offsetInMS) {
    if (_currentOffset > offsetInMS) resetCurrentOffset();
    // "Start" all items that have started since the last tick
    while (_nextStartedItem != _items.end() &&
           _nextStartedItem->first <= offsetInMS) {
      _startedItems.push_back(_nextStartedItem->second.get());
      ++_nextStartedItem;
    }
    // "End" all items which duration have passed.
    for (std::vector<SceneItem *>::iterator i = _startedItems.begin();
         i != _startedItems.end(); ++i) {
      if ((*i)->OffsetInMS() + (*i)->DurationInMS() < offsetInMS) {
        --i;
        std::vector<SceneItem *>::iterator removePointer = i;
        ++removePointer;
        _startedItems.erase(removePointer);
      }
    }
    if (ItemsHaveEnd()) ++_endOfItems;
    _currentOffset = offsetInMS;
  }

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

  void RecalculateControllables() {
    std::set<std::pair<const Controllable *, size_t>> controllables;
    for (const std::pair<const double, std::unique_ptr<SceneItem>> &item :
         _items) {
      if (ControlSceneItem *c_item =
              dynamic_cast<ControlSceneItem *>(item.second.get());
          c_item) {
        controllables.emplace(
            std::make_pair(&c_item->GetControllable(), c_item->GetInput()));
      }
    }
    controllables_.assign(controllables.begin(), controllables.end());
  }

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

  void OnSyncUpdate(double offsetInMS) override;
};

}  // namespace glight::theatre

#endif
