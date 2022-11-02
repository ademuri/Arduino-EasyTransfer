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
#include "test_stream.h"
#endif

#include <cstdint>
#include <cstdlib>
#include <cstring>

// make it a little prettier on the front end.
#define details(name) (uint8_t *)&name, sizeof(name)

template <typename DataType>
class EasyTransfer {
 public:
  EasyTransfer(DataType &data);
  void begin(Stream *theStream);
  void sendData();
  bool receiveData();

 private:
  DataType &data;      // address of struct

  Stream *_stream;
  DataType rx_buffer;    // address for temporary storage and parsing buffer
  uint8_t rx_array_inx;  // index for RX parsing buffer
  uint8_t rx_len;        // RX packet length according to the packet
  uint8_t calc_CS;       // calculated checksum
};

template <typename DataType>
EasyTransfer<DataType>::EasyTransfer(DataType &data) : data(data) {

}

// Captures address and size of struct
template <typename DataType>
void EasyTransfer<DataType>::begin(Stream *stream) {
  _stream = stream;
}

// Sends out struct in binary, with header, length info and checksum
template <typename DataType>
void EasyTransfer<DataType>::sendData() {
  uint8_t CS = sizeof(DataType);
  _stream->write(0x06);
  _stream->write(0x85);
  _stream->write(sizeof(DataType));
  for (int i = 0; i < sizeof(DataType); i++) {
    CS ^= *((uint8_t *)&data + i);
    _stream->write(*((uint8_t *)&data + i));
  }
  _stream->write(CS);
}

template <typename DataType>
bool EasyTransfer<DataType>::receiveData() {
  // start off by looking for the header bytes. If they were already found in a
  // previous call, skip it.
  if (rx_len == 0) {
    // this size check may be redundant due to the size check below, but for now
    // I'll leave it the way it is.
    if (_stream->available() >= 3) {
      // this will block until a 0x06 is found or buffer size becomes less
      // then 3.
      while (_stream->read() != 0x06) {
        // This will trash any preamble junk in the serial buffer
        // but we need to make sure there is enough in the buffer to process
        // while we trash the rest if the buffer becomes too empty, we will
        // escape and try again on the next call
        if (_stream->available() < 3) return false;
      }
      if (_stream->read() == 0x85) {
        rx_len = _stream->read();
        // make sure the binary structs on both Arduinos are the same size.
        if (rx_len != sizeof(DataType)) {
          rx_len = 0;
          return false;
        }
      }
    }
  }

  // we get here if we already found the header bytes, the struct size matched
  // what we know, and now we are byte aligned.
  if (rx_len != 0) {
    while (_stream->available() && rx_array_inx <= rx_len) {
      ((uint8_t*)&rx_buffer)[rx_array_inx++] = _stream->read();
    }

    if (rx_len == (rx_array_inx - 1)) {
      // seem to have got whole message
      // last uint8_t is CS
      calc_CS = rx_len;
      for (int i = 0; i < rx_len; i++) {
        calc_CS ^= ((uint8_t*)&rx_buffer)[i];
      }

      if (calc_CS == ((uint8_t*)&rx_buffer)[rx_array_inx - 1]) {  // CS good
        memcpy(&data, &rx_buffer, sizeof(DataType));
        rx_len = 0;
        rx_array_inx = 0;
        return true;
      }

      else {
        // failed checksum, need to clear this out anyway
        rx_len = 0;
        rx_array_inx = 0;
        return false;
      }
    }
  }

  return false;
}
