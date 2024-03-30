#ifndef THEATRE_DUMMYDEVICE_H_
#define THEATRE_DUMMYDEVICE_H_

#include <cassert>
#include <unistd.h>

namespace glight::theatre {

/**
 * @author Andre Offringa
 */
class DummyDevice {
 public:
  DummyDevice() = default;

  ~DummyDevice() = default;

  void Open() { is_open_ = true; }

  size_t NUniverses() const { return 2; }

  UniverseType GetUniverseType(size_t universe) const {
    assert(universe < 2);
    return universe == 0 ? UniverseType::Output : UniverseType::Input;
  }

  void SetOutputValues(unsigned universe, const unsigned char *new_values,
                       size_t size) {}

  void GetOutputValues(unsigned universe, unsigned char *destination,
                       size_t size) {
    std::fill_n(destination, size, 0);
  }

  void GetInputValues(unsigned universe, unsigned char *destination,
                      size_t size) {}

  void WaitForNextSync() {
    usleep(40000);
    ++sync_;
  }

  void Abort() { is_open_ = false; }

  bool IsOpen() { return is_open_; }

 private:
  bool is_open_ = false;
};

}  // namespace glight::theatre

#endif  // DUMMYDEVICE_H
