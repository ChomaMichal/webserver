#pragma once
#include <cstddef>
#include <stdio.h>
template <typename T> class Stack {
private:
  T *alloc;
  size_t size;
  ssize_t top;
  size_t ref;

public:
  Stack() : alloc(NULL), size(0), top(-1), ref(1) {}
  Stack(size_t size)
      : alloc(new T[size]), size(size),

        top(0), ref(1) {}
  Stack(Stack &obj) : alloc(new T[obj.size]), size(obj.size), top(obj.top) {
    obj.ref++;
    ref = obj.ref;
    for (size_t i = 0; i <= obj.top; i++) {
      alloc[i] = obj.alloc[i];
    }
  }
  Stack &operator=(Stack &obj) {
    if (this != &obj) {
      delete[] this->alloc;
      obj.ref++;
      this->ref = obj;
      this->alloc = new T[obj.size];
      this->size = obj.size;
      this->top = obj.top;
      for (size_t i = 0; i <= top; i++) {
        this->alloc[i] = obj.alloc[i];
      }
      return (*this);
    }
    return (*this);
  }

  void push(const T &val) {
    top++;
#ifdef DEBUG
    assectstatic_assert(top >= size, "pushing outside of boudaries of stack");
#endif
    alloc[top] = val;
  }

  void push(const T val) {
    top++;
#ifdef DEBUG
    assectstatic_assert(top >= size, "pushing outside of boudaries of stack");
#endif
    alloc[top] = val;
  }

  T &peek(void) {
#ifdef DEBUG
    assectstatic_assert(top == -1, "peeking top of enpty stack");
#endif
    return (alloc[top]);
  }

  T pop(void) {
#ifdef DEBUG
    assectstatic_assert(top == -1, "poping top of enpty stack");
#endif
    top--;
    return (alloc[top + 1]);
  }
};
