#ifndef RANDOM_SELECT_EFFECT_PS
#define RANDOM_SELECT_EFFECT_PS

#include "propertyset.h"

#include "../effects/randomselecteffect.h"

namespace glight::theatre {

class RandomSelectEffectPS final : public PropertySet {
 public:
  RandomSelectEffectPS() {
    addProperty(
        Property("delay", "Delay for reselection", PropertyType::Duration));
    addProperty(Property("count", "Number of outputs", PropertyType::Integer));
    addProperty(Property("transition", "Transition", PropertyType::Transition));
  }

 protected:
  virtual void setDuration(FolderObject &object, size_t index,
                           double value) const final {
    RandomSelectEffect &rfx = static_cast<RandomSelectEffect &>(object);
    switch (index) {
      case 0:
        rfx.SetDelay(value);
        break;
    }
  }

  virtual double getDuration(const FolderObject &object,
                             size_t index) const final {
    const RandomSelectEffect &rfx =
        static_cast<const RandomSelectEffect &>(object);
    switch (index) {
      case 0:
        return rfx.Delay();
    }
    return 0;
  }

  virtual void setInteger(FolderObject &object, size_t index,
                          int value) const final {
    RandomSelectEffect &rfx = static_cast<RandomSelectEffect &>(object);
    switch (index) {
      case 1:
        rfx.SetCount(value);
        break;
    }
  }

  virtual int getInteger(const FolderObject &object, size_t index) const final {
    const RandomSelectEffect &rfx =
        static_cast<const RandomSelectEffect &>(object);
    switch (index) {
      case 1:
        return rfx.Count();
    }
    return 0;
  }

  virtual void setTransition(FolderObject &object, size_t index,
                             const Transition &value) const final {
    RandomSelectEffect &rfx = static_cast<RandomSelectEffect &>(object);
    switch (index) {
      case 2:
        rfx.SetTransition(value);
        break;
    }
  }

  virtual Transition getTransition(const FolderObject &object,
                                   size_t index) const final {
    const RandomSelectEffect &rfx =
        static_cast<const RandomSelectEffect &>(object);
    return rfx.GetTransition();
  }
};

}  // namespace glight::theatre

#endif
