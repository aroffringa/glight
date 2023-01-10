#ifndef THEATRE_SHOW_H_
#define THEATRE_SHOW_H_

#include <memory>
#include <vector>

#include "scene.h"

namespace glight::theatre {

class Management;
class Timing;

/**
 * @author Andre Offringa
 */
class Show {
 public:
  Show(Management &management) : _management(management) {}
  ~Show() { Clear(); }

  void Clear();

  Scene &AddScene(bool in_folder);

  void StartScene(double _timeInMS, Scene &scene) {
    if (!isRunning(&scene)) _runningScenes.push_back(&scene);
    scene.Start(_timeInMS);
  }
  void StopScene(Scene &scene) {
    if (isRunning(&scene)) {
      removeRunningScene(_runningScenes, &scene);
    }
    scene.Stop();
  }
  void Mix(unsigned *channelValues, unsigned universe, const Timing &timing);
  const std::vector<std::unique_ptr<Scene>> &Scenes() const { return _scenes; }

 private:
  bool isRunning(Scene *scene) const {
    for (const Scene *rScene : _runningScenes)
      if (scene == rScene) return true;
    return false;
  }

  void removeRunningScene(std::vector<Scene *> &container, Scene *scene) const {
    for (std::vector<Scene *>::iterator i = container.begin();
         i != container.end(); ++i) {
      if (*i == scene) {
        container.erase(i);
        return;
      }
    }
  }

  Management &_management;
  std::vector<std::unique_ptr<Scene>> _scenes;
  std::vector<Scene *> _runningScenes;
};

}  // namespace glight::theatre

#endif
