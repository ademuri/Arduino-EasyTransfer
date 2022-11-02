#include "test_stream.h"

#include <cstdio>

size_t Stream::write(uint8_t c) {
  deque_.push_back(c);
  std::printf("write: %02X\n", c);
  return 1;
}

int Stream::available() {
  std::printf("available: %lu\n", deque_.size());
  return deque_.size();
}

int Stream::read() {
  int ret = deque_.front();
  deque_.pop_front();
  return ret;
}
