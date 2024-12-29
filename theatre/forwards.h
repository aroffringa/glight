#ifndef THEATRE_FORWARDS_H_
#define THEATRE_FORWARDS_H_

namespace glight::theatre {

class BeatFinder;
class Chase;
class Color;
class Controllable;
class ControlSceneItem;
class DmxDevice;
class Effect;
class Fixture;
class FixtureControl;
class FixtureGroup;
class FixtureType;
class Folder;
class FolderObject;
class KeySceneItem;
class Management;
class PresetCollection;
class PresetValue;
class PropertySet;
class Scene;
class SceneItem;
class SingleSourceValue;
class SourceValue;
class Theatre;
class TimeSequence;
class ValueSnapshot;
enum class EffectType;
}  // namespace glight::theatre

namespace glight::system {
template <typename T>
class TrackablePtr;
template <typename T>
class ObservingPtr;
}  // namespace glight::system
#endif
