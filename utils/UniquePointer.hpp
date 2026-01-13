#pragma once
#include <cstddef>
template <typename T>

class UniquePointer {
private:
  T *allocation;
  UniquePointer(UniquePointer<T> &other);
  UniquePointer &operator=(UniquePointer &other);

public:
  UniquePointer() : allocation(NULL) {}
  UniquePointer(T *ptr) : allocation(ptr) {}
  template <typename Args> explicit UniquePointer(Args &args) {
    this->allocation = new T(args);
  }
  ~UniquePointer() { delete this->allocation; }
  void move(UniquePointer &other) {
    if (this != &other) {
      delete this->allocation;
      this->allocation = other.allocation;
      other.allocation = NULL;
    }
  }
  T *get(void) const { return (this->allocation); }
  T *operator->(void) const { return (this->allocation); }
  T &operator*(void) const { return (*this->allocation); }
  T *release(void) {
    T *tmp = this->allocation;
    this->allocation = NULL;
    return (tmp);
  }
  void reset(T *ptr = 0) {
    if (allocation != ptr) {
      delete allocation;
      allocation = ptr;
    }
  }
};
