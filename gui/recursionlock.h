#ifndef RECURSION_LOCK_H
#define RECURSION_LOCK_H

class RecursionLock {
public:
  RecursionLock() : _isTaken(false) {}

  class Token {
  public:
    Token(RecursionLock &ar) : _isTaken(&ar._isTaken) {
#ifndef NDEBUG
      if (ar._isTaken)
        throw std::runtime_error(
            "Logical error: DisableRecursion() called from multiple scopes");
#endif
      _owned = true;
      ar._isTaken = true;
    }
    Token(const Token &token) = delete;
    Token(Token &&token) = delete;

    ~Token() {
#ifndef NDEBUG
      if (_owned && !ar._isTaken)
        throw std::runtime_error(
            "Logical error: token was released from wrong scope");
#endif
      if (_owned)
        (*_isTaken) = false;
    }

    Token &operator=(const Token &token) = delete;
    Token &operator=(Token &&token) = delete;

    void Release() {
#ifndef NDEBUG
      if (!_owned)
        throw std::runtime_error(
            "Logical error: Release() was called twice for token");
      if (!ar._isTaken)
        throw std::runtime_error(
            "Logical error: token was released from wrong scope");
#endif
      if (_owned) {
        _owned = false;
        (*_isTaken) = false;
      }
    }

  private:
    bool _owned;
    bool *_isTaken;
  };

  bool IsFirst() const { return !_isTaken; }

private:
  bool _isTaken;
};

#endif
