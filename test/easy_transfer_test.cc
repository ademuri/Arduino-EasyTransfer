#include <gtest/gtest.h>
#include <test_stream.h>

#include "EasyTransfer.h"

namespace {

struct TestMessage {
  uint8_t a;
  uint32_t b;
  float c;
};

TEST(EasyTransfer, Transmit) {
  Stream stream;
  TestMessage in;
  EasyTransfer<TestMessage> transfer_in(in);
  transfer_in.begin(&stream);

  TestMessage out;
  EasyTransfer<TestMessage> transfer_out(out);
  transfer_out.begin(&stream);

  out.a = 17;
  transfer_out.sendData();
  EXPECT_TRUE(transfer_in.receiveData());
  EXPECT_EQ(in.a, out.a);
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
