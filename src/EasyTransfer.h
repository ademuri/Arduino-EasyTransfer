/******************************************************************
*  EasyTransfer Arduino Library
*		details and example sketch:
*			http://www.billporter.info/easytransfer-arduino-library/
*
*		Brought to you by:
*              Bill Porter
*              www.billporter.info
*
*		See Readme for other info and version history
*
*
*This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or(at your option) any later
version. This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
details. <http://www.gnu.org/licenses/>
*
*This work is licensed under the Creative Commons Attribution-ShareAlike 3.0
Unported License. *To view a copy of this license, visit
http://creativecommons.org/licenses/by-sa/3.0/ or *send a letter to Creative
Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
******************************************************************/
#pragma once

#ifdef ARDUINO
#include "Arduino.h"
#else
#include <cstdint>
#include <cstdlib>
#include <cstring>

// TODO: remove
#include <cstdio>

#include "test_stream.h"
#endif

template <typename DataType>
class EasyTransfer {
 public:
  EasyTransfer(DataType *data);
  void begin(Stream *theStream);
  void sendData();
  bool receiveData();

 private:
  enum class ParserState {
    INIT,
    READ_PREAMBLE_ONE,
    READ_PREAMBLE_TWO,
    READ_SIZE,
  };

  ParserState state_ = ParserState::INIT;
  DataType *const data_;  // address of struct

  Stream *stream_ = nullptr;
  uint8_t rx_buffer_[sizeof(DataType) + 1];
  uint8_t read_bytes_ = 0;
  uint8_t rx_array_index_ = 0;  // index for RX parsing buffer
  uint8_t rx_len_ = 0;          // RX packet length according to the packet
  uint8_t computed_checksum_ = 0;
};

template <typename DataType>
EasyTransfer<DataType>::EasyTransfer(DataType *data) : data_(data) {}

// Captures address and size of struct
template <typename DataType>
void EasyTransfer<DataType>::begin(Stream *stream) {
  stream_ = stream;
}

// Sends out struct in binary, with header, length info and checksum
template <typename DataType>
void EasyTransfer<DataType>::sendData() {
  uint8_t checksum = sizeof(DataType);
  stream_->write(0x06);
  stream_->write(0x85);
  stream_->write(sizeof(DataType));
  for (size_t i = 0; i < sizeof(DataType); i++) {
    checksum ^= *((uint8_t *)data_ + i);
    stream_->write(*((uint8_t *)data_ + i));
  }
  stream_->write(checksum);
}

template <typename DataType>
bool EasyTransfer<DataType>::receiveData() {
  while (stream_->available()) {
    switch (state_) {
      case ParserState::INIT:
        if (stream_->read() == 0x06) {
          state_ = ParserState::READ_PREAMBLE_ONE;
        }
        break;

      case ParserState::READ_PREAMBLE_ONE:
        if (stream_->read() == 0x85) {
          state_ = ParserState::READ_PREAMBLE_TWO;
        }
        break;

      case ParserState::READ_PREAMBLE_TWO: {
        uint8_t read = stream_->read();
        if (read == sizeof(DataType)) {
          state_ = ParserState::READ_SIZE;
          read_bytes_ = 0;
          computed_checksum_ = sizeof(DataType);
        } else if (read == 0x06) {
          state_ = ParserState::READ_PREAMBLE_ONE;
        } else {
          state_ = ParserState::INIT;
        }
        break;
      }

      case ParserState::READ_SIZE:
        rx_buffer_[read_bytes_] = stream_->read();
        computed_checksum_ ^= rx_buffer_[read_bytes_];
        read_bytes_++;
        if (read_bytes_ == sizeof(DataType) + 1) {
          // Last byte is the checksum, and X ^ X = 0
          if (computed_checksum_ == 0) {
            memcpy(data_, rx_buffer_, sizeof(DataType));
            state_ = ParserState::INIT;
            return true;
          } else {
            state_ = ParserState::INIT;
            return false;
          }
        }
        break;
    }
  }
  return false;
}
