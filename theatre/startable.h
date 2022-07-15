#ifndef STARTABLE_H
#define STARTABLE_H

#include "folderobject.h"

/**
 * @author Andre Offringa
 */
class Startable : public FolderObject {
 public:
  Startable() {}
  virtual ~Startable() {}

  void Start(double timeInMS) {
    _startTimeInMS = timeInMS;
    onStart();
  }

  virtual bool HasEnd(double offsetInMS) = 0;

  double StartTimeInMS() const { return _startTimeInMS; }

 protected:
  virtual void onStart() = 0;
  void setStartTimeInMS(double startTimeInMS) {
    _startTimeInMS = startTimeInMS;
  }

 private:
  double _startTimeInMS;
};

#endif
