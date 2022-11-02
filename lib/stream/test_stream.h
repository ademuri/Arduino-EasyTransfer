#pragma once

#ifndef ARDUINO
#include <cstddef>
#include <cstdint>
#include <deque>

class Stream {
 public:
  size_t write(uint8_t);
  int available();
  int read();

 private:
  std::deque<int> deque_;
};

#endif  // ARDUINO
