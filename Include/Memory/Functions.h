#pragma once

#include <cstdint>

uint64_t SetBoolAt(uint64_t src, uint8_t at, bool value);

uint32_t SetBoolAt(uint32_t src, uint8_t at, bool value);

uint16_t SetBoolAt(uint16_t src, uint8_t at, bool value);

uint8_t SetBoolAt(uint8_t src, uint8_t at, bool value);

uint64_t SetBoolAtU64(uint64_t src, uint8_t at, bool value);

uint32_t SetBoolAtU32(uint32_t src, uint8_t at, bool value);

uint16_t SetBoolAtU16(uint16_t src, uint8_t at, bool value);

uint8_t SetBoolAtU8(uint8_t src, uint8_t at, bool value);

bool SampleBoolFromMask(uint8_t src, uint8_t at);

bool SampleBoolFromMask(uint16_t src, uint8_t at);

bool SampleBoolFromMask(uint32_t src, uint8_t at);

bool SampleBoolFromMask(uint64_t src, uint8_t at);

bool SampleBoolFromMask8(uint8_t src, uint8_t at);

bool SampleBoolFromMask16(uint16_t src, uint8_t at);

bool SampleBoolFromMask32(uint32_t src, uint8_t at);

bool SampleBoolFromMask64(uint64_t src, uint8_t at);


// TODO verify
// uint8_t [0, 255] → int8_t [-128, 127]
int8_t uint8_to_int8(uint8_t value);

// TODO verify
// uint16_t [0, 65535] → int16_t [-32768, 32767]
int16_t uint16_to_int16(uint16_t value);

// TODO verify
// uint32_t [0, 4294967295] → int32_t [-2147483648, 2147483647]
int32_t uint32_to_int32(uint32_t value);

// TODO verify
// uint64_t → int64_t
int64_t uint64_to_int64(uint64_t value);

// TODO verify
// int8_t [-128, 127] → uint8_t [0, 255]
uint8_t int8_to_uint8(int8_t value);

// TODO verify
// int16_t [-32768, 32767] → uint16_t [0, 65535]
uint16_t int16_to_uint16(int16_t value);

// TODO verify
// int32_t [-2147483648, 2147483647] → uint32_t [0, 4294967295]
uint32_t int32_to_uint32(int32_t value);

// TODO verify
// int64_t → uint64_t
uint64_t int64_to_uint64(int64_t value);
