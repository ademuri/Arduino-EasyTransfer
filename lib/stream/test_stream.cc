#include "test_stream.h"

size_t Stream::write(uint8_t c) {
  deque_.push_back(c);
  return 1;
}

int Stream::available() {
  return deque_.size();
}

int Stream::read() {
  int ret = deque_.front();
  deque_.pop_front();
  return ret;
}
