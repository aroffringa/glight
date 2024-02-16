#ifndef GLIGHT_SYSTEM_MIDI_BUTTON_SET_H_
#define GLIGHT_SYSTEM_MIDI_BUTTON_SET_H_

#include <memory>

namespace glight::system::midi {

class ButtonSet {
 public:
  using iterator = unsigned char*;
  using const_iterator = const unsigned char*;

  ButtonSet(size_t queue_size) : list_(new unsigned char[queue_size]) {}

  operator bool() const { return size_ != 0; }

  unsigned char* Data() { return list_.get(); }

  unsigned char Size() const { return size_; }
  void SetSize(unsigned char size) { size_ = size; }

  bool Contains(unsigned char button) const {
    for (unsigned char i = 0; i != size_; ++i) {
      if (list_[i] == button) return true;
    }
    return false;
  }

  iterator begin() { return &list_[0]; }
  const_iterator begin() const { return &list_[0]; }
  iterator end() { return &list_[0] + size_; }
  const_iterator end() const { return &list_[0] + size_; }

 private:
  unsigned char size_ = 0;
  std::unique_ptr<unsigned char[]> list_;
};

}  // namespace glight::system::midi

#endif
