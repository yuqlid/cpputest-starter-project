#include "CppUTest/TestHarness.h"
#include "StructTypes.hpp"

TEST_GROUP(Int32Uint8) {
  Int32Uint8* int32Uint8;

  void setup() { int32Uint8 = new Int32Uint8(); }

  void teardown() { delete int32Uint8; }
};

TEST(Int32Uint8, DefaultConstructor) {
  CHECK_EQUAL(0, int32Uint8->getValue());
  CHECK_EQUAL(0, int32Uint8->getByte0());
  CHECK_EQUAL(0, int32Uint8->getByte1());
  CHECK_EQUAL(0, int32Uint8->getByte2());
  CHECK_EQUAL(0, int32Uint8->getByte3());
}

TEST(Int32Uint8, InitConstructor) {
  Int32Uint8 int32Uint8Init(0x12345678);
  CHECK_EQUAL(0x12345678, int32Uint8Init.getValue());
}

TEST(Int32Uint8, SetValue) {
  int32Uint8->setValue(static_cast<int32_t>(0x87654321));  // 明示的に型キャスト
  CHECK_EQUAL(static_cast<int32_t>(0x87654321),
              int32Uint8->getValue());  // 明示的に型キャスト
}

TEST(Int32Uint8, SetBytes) {
  int32Uint8->setByte0(0xAA);
  int32Uint8->setByte1(0xBB);
  int32Uint8->setByte2(0xCC);
  int32Uint8->setByte3(0xDD);

  CHECK_EQUAL(0xAA, int32Uint8->getByte0());
  CHECK_EQUAL(0xBB, int32Uint8->getByte1());
  CHECK_EQUAL(0xCC, int32Uint8->getByte2());
  CHECK_EQUAL(0xDD, int32Uint8->getByte3());

  // バイトを設定した後の値を確認
  int32_t expectedValue = (0xDD << 24) | (0xCC << 16) | (0xBB << 8) | 0xAA;
  CHECK_EQUAL(expectedValue, int32Uint8->getValue());
}

TEST_GROUP(FloatUint8) {
  FloatUint8* floatUint8;

  void setup() { floatUint8 = new FloatUint8(); }

  void teardown() { delete floatUint8; }
};

TEST(FloatUint8, DefaultConstructor) {
  CHECK_EQUAL(0.0f, floatUint8->getValue());
  CHECK_EQUAL(0, floatUint8->getByte0());
  CHECK_EQUAL(0, floatUint8->getByte1());
  CHECK_EQUAL(0, floatUint8->getByte2());
  CHECK_EQUAL(0, floatUint8->getByte3());
}

TEST(FloatUint8, InitConstructor) {
  FloatUint8 floatUint8Init(1.23f);
  CHECK_EQUAL(1.23f, floatUint8Init.getValue());
}

TEST(FloatUint8, SetValue) {
  floatUint8->setValue(4.56f);
  CHECK_EQUAL(4.56f, floatUint8->getValue());
}

TEST(FloatUint8, SetBytes) {
  floatUint8->setByte0(0xAA);
  floatUint8->setByte1(0xBB);
  floatUint8->setByte2(0xCC);
  floatUint8->setByte3(0xDD);

  CHECK_EQUAL(0xAA, floatUint8->getByte0());
  CHECK_EQUAL(0xBB, floatUint8->getByte1());
  CHECK_EQUAL(0xCC, floatUint8->getByte2());
  CHECK_EQUAL(0xDD, floatUint8->getByte3());

  // バイトを設定した後の値を確認
  uint32_t expectedValue = (0xDD << 24) | (0xCC << 16) | (0xBB << 8) | 0xAA;
  float* floatPtr = reinterpret_cast<float*>(&expectedValue);
  CHECK_EQUAL(*floatPtr, floatUint8->getValue());
}

TEST_GROUP(Uint16Uint8) {
  Uint16Uint8* uint16Uint8;

  void setup() { uint16Uint8 = new Uint16Uint8(); }

  void teardown() { delete uint16Uint8; }
};

TEST(Uint16Uint8, DefaultConstructor) {
  CHECK_EQUAL(0, uint16Uint8->getValue());
  CHECK_EQUAL(0, uint16Uint8->getByte0());
  CHECK_EQUAL(0, uint16Uint8->getByte1());
}

TEST(Uint16Uint8, InitConstructor) {
  Uint16Uint8 uint16Uint8Init(0x1234);
  CHECK_EQUAL(0x1234, uint16Uint8Init.getValue());
}

TEST(Uint16Uint8, SetValue) {
  uint16Uint8->setValue(0x5678);
  CHECK_EQUAL(0x5678, uint16Uint8->getValue());
}

TEST(Uint16Uint8, SetBytes) {
  uint16Uint8->setByte0(0xAA);
  uint16Uint8->setByte1(0xBB);

  CHECK_EQUAL(0xAA, uint16Uint8->getByte0());
  CHECK_EQUAL(0xBB, uint16Uint8->getByte1());

  // バイトを設定した後の値を確認
  uint16_t expectedValue = (0xBB << 8) | 0xAA;
  CHECK_EQUAL(expectedValue, uint16Uint8->getValue());
}

TEST_GROUP(Int16Uint8) {
  Int16Uint8* int16Uint8;

  void setup() { int16Uint8 = new Int16Uint8(); }

  void teardown() { delete int16Uint8; }
};

TEST(Int16Uint8, DefaultConstructor) {
  CHECK_EQUAL(0, int16Uint8->getValue());
  CHECK_EQUAL(0, int16Uint8->getByte0());
  CHECK_EQUAL(0, int16Uint8->getByte1());
}

TEST(Int16Uint8, InitConstructor) {
  Int16Uint8 int16Uint8Init(0x1234);
  CHECK_EQUAL(0x1234, int16Uint8Init.getValue());
}

TEST(Int16Uint8, SetValue) {
  int16Uint8->setValue(static_cast<int16_t>(0x5678));  // 明示的に型キャスト
  CHECK_EQUAL(static_cast<int16_t>(0x5678),
              int16Uint8->getValue());  // 明示的に型キャスト
}

TEST(Int16Uint8, SetBytes) {
  int16Uint8->setByte0(0xAA);
  int16Uint8->setByte1(0xBB);

  CHECK_EQUAL(0xAA, int16Uint8->getByte0());
  CHECK_EQUAL(0xBB, int16Uint8->getByte1());

  // バイトを設定した後の値を確認
  int16_t expectedValue = (0xBB << 8) | 0xAA;
  CHECK_EQUAL(expectedValue, int16Uint8->getValue());
}

TEST_GROUP(Uint32Uint8) {
  Uint32Uint8* uint32Uint8;

  void setup() { uint32Uint8 = new Uint32Uint8(); }

  void teardown() { delete uint32Uint8; }
};

TEST(Uint32Uint8, DefaultConstructor) {
  CHECK_EQUAL(0, uint32Uint8->getValue());
  CHECK_EQUAL(0, uint32Uint8->getByte0());
  CHECK_EQUAL(0, uint32Uint8->getByte1());
  CHECK_EQUAL(0, uint32Uint8->getByte2());
  CHECK_EQUAL(0, uint32Uint8->getByte3());
}

TEST(Uint32Uint8, InitConstructor) {
  Uint32Uint8 uint32Uint8Init(0x12345678);
  CHECK_EQUAL(0x12345678, uint32Uint8Init.getValue());
}

TEST(Uint32Uint8, SetValue) {
  uint32Uint8->setValue(0x87654321);
  CHECK_EQUAL(0x87654321, uint32Uint8->getValue());
}

TEST(Uint32Uint8, SetBytes) {
  uint32Uint8->setByte0(0xAA);
  uint32Uint8->setByte1(0xBB);
  uint32Uint8->setByte2(0xCC);
  uint32Uint8->setByte3(0xDD);

  CHECK_EQUAL(0xAA, uint32Uint8->getByte0());
  CHECK_EQUAL(0xBB, uint32Uint8->getByte1());
  CHECK_EQUAL(0xCC, uint32Uint8->getByte2());
  CHECK_EQUAL(0xDD, uint32Uint8->getByte3());

  // バイトを設定した後の値を確認
  uint32_t expectedValue = (0xDD << 24) | (0xCC << 16) | (0xBB << 8) | 0xAA;
  CHECK_EQUAL(expectedValue, uint32Uint8->getValue());
}