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
  DataType *const data_;  // address of struct

  Stream *stream_ = nullptr;
  uint8_t rx_buffer_[sizeof(DataType) + 1];
  uint8_t rx_array_index_ = 0;       // index for RX parsing buffer
  uint8_t rx_len_ = 0;               // RX packet length according to the packet
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
  // start off by looking for the header bytes. If they were already found in a
  // previous call, skip it.
  if (rx_len_ == 0) {
    // this size check may be redundant due to the size check below, but for now
    // I'll leave it the way it is.
    if (stream_->available() >= 3) {
      // this will block until a 0x06 is found or buffer size becomes less
      // then 3.
      while (stream_->read() != 0x06) {
        // This will trash any preamble junk in the serial buffer
        // but we need to make sure there is enough in the buffer to process
        // while we trash the rest if the buffer becomes too empty, we will
        // escape and try again on the next call
        if (stream_->available() < 3) return false;
      }
      if (stream_->read() == 0x85) {
        rx_len_ = stream_->read();
        // make sure the binary structs on both Arduinos are the same size.
        if (rx_len_ != sizeof(DataType)) {
          rx_len_ = 0;
          return false;
        }
      }
    }
  }

  // we get here if we already found the header bytes, the struct size matched
  // what we know, and now we are byte aligned.
  if (rx_len_ != 0) {
    while (stream_->available() && rx_array_index_ <= rx_len_) {
      rx_buffer_[rx_array_index_++] = stream_->read();
    }

    if (rx_len_ == (rx_array_index_ - 1)) {
      // seem to have got whole message
      // last uint8_t is CS
      uint8_t calculated_checksum = rx_len_;
      for (int i = 0; i < rx_len_; i++) {
        calculated_checksum ^= rx_buffer_[i];
      }

      if (calculated_checksum == rx_buffer_[rx_array_index_ - 1]) {  // CS good
        memcpy(data_, rx_buffer_, sizeof(DataType));
        rx_len_ = 0;
        rx_array_index_ = 0;
        return true;
      }

      else {
        // failed checksum, need to clear this out anyway
        rx_len_ = 0;
        rx_array_index_ = 0;
        return false;
      }
    }
  }

  return false;
}
