#include <gtest/gtest.h>
#include <test_stream.h>

#include "EasyTransfer.h"

namespace {

struct TestMessage {
  uint8_t a = 0;
  uint32_t b = 0;
  float c = 0;
};

TEST(EasyTransfer, Transmit) {
  Stream stream;
  TestMessage in;
  EasyTransfer<TestMessage> transfer_in(&in);
  transfer_in.begin(&stream);

  TestMessage out;
  EasyTransfer<TestMessage> transfer_out(&out);
  transfer_out.begin(&stream);

  out.a = 17;
  out.b = 0xDEADBEEF;
  out.c = 3.14159;
  transfer_out.sendData();
  EXPECT_TRUE(transfer_in.receiveData());
  EXPECT_EQ(in.a, out.a);
  EXPECT_EQ(in.b, out.b);
  EXPECT_EQ(in.c, out.c);
}

TEST(EasyTransfer, IgnoresGarbage) {
  Stream stream;
  TestMessage out;
  EasyTransfer<TestMessage> transfer(&out);
  transfer.begin(&stream);
  EXPECT_EQ(out.a, 0);
  EXPECT_EQ(out.b, 0);
  EXPECT_EQ(out.c, 0);

  stream.write(0);
  transfer.receiveData();
  EXPECT_FALSE(transfer.receiveData());

  stream.write(0x6);
  stream.write(0x6);
  transfer.receiveData();
  EXPECT_FALSE(transfer.receiveData());

  stream.write(0x6);
  stream.write(0x85);
  stream.write(0x0);
  stream.write(0x0);
  transfer.receiveData();
  EXPECT_FALSE(transfer.receiveData());
}

TEST(EasyTransfer, IgnoresSpuriousPreamble) {
  Stream stream;
  TestMessage out;
  EasyTransfer<TestMessage> transfer(&out);
  transfer.begin(&stream);

  stream.write(0x0);
  transfer.sendData();
  EXPECT_TRUE(transfer.receiveData());

  stream.write(0x6);
  stream.write(0x1);
  transfer.sendData();
  EXPECT_FALSE(transfer.receiveData());
  EXPECT_TRUE(transfer.receiveData());

  stream.write(0x6);
  transfer.sendData();
  EXPECT_FALSE(transfer.receiveData());
  EXPECT_TRUE(transfer.receiveData());
}

};  // namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  // if you plan to use GMock, replace the line above with
  // ::testing::InitGoogleMock(&argc, argv);

  if (RUN_ALL_TESTS())
    ;

  // Always return zero-code and allow PlatformIO to parse results
  return 0;
}
