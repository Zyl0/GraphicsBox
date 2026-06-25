#include "Functions.h"

uint64_t SetBoolAt(uint64_t src, uint8_t at, bool value)
{
    uint64_t placedValueToSet = ((value ? static_cast<uint64_t>(1) : static_cast<uint64_t>(0)) << at);

    //filter the number to remove the bit in place of the one that is to me set
    uint64_t mask = ~((static_cast<uint64_t>(1)) << at);
    src = src & mask;

    //Set bool value
    return src | placedValueToSet ;
}

uint32_t SetBoolAt(uint32_t src, uint8_t at, bool value)
{
    uint32_t placedValueToSet = ((value ? static_cast<uint32_t>(1) : static_cast<uint32_t>(0)) << at);

    //filter the number to remove the bit in place of the one that is to me set
    uint32_t mask = ~((static_cast<uint32_t>(1)) << at);
    src = src & mask;

    //Set bool value
    return src | placedValueToSet ;
}

uint16_t SetBoolAt(uint16_t src, uint8_t at, bool value)
{
    uint16_t placedValueToSet = ((value ? static_cast<uint16_t>(1) : static_cast<uint16_t>(0)) << at);

    //filter the number to remove the bit in place of the one that is to me set
    uint16_t mask = ~((static_cast<uint16_t>(1)) << at);
    src = src & mask;

    //Set bool value
    return src | placedValueToSet ;
}

uint8_t SetBoolAt(uint8_t src, uint8_t at, bool value)
{
    uint8_t placedValueToSet = ((value ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0)) << at);

    //filter the number to remove the bit in place of the one that is to me set
    uint8_t mask = ~((static_cast<uint8_t>(1)) << at);
    src = src & mask;

    //Set bool value
    return src | placedValueToSet ;
}

uint64_t SetBoolAtU64(uint64_t src, uint8_t at, bool value)
{
    uint64_t placedValueToSet = ((value ? static_cast<uint64_t>(1) : static_cast<uint64_t>(0)) << at);

    //filter the number to remove the bit in place of the one that is to me set
    uint64_t mask = ~((static_cast<uint64_t>(1)) << at);
    src = src & mask;

    //Set bool value
    return src | placedValueToSet ;
}

uint32_t SetBoolAtU32(uint32_t src, uint8_t at, bool value)
{
    uint32_t placedValueToSet = ((value ? static_cast<uint32_t>(1) : static_cast<uint32_t>(0)) << at);

    //filter the number to remove the bit in place of the one that is to me set
    uint32_t mask = ~((static_cast<uint32_t>(1)) << at);
    src = src & mask;

    //Set bool value
    return src | placedValueToSet ;
}

uint16_t SetBoolAtU16(uint16_t src, uint8_t at, bool value)
{
    uint16_t placedValueToSet = ((value ? static_cast<uint16_t>(1) : static_cast<uint16_t>(0)) << at);

    //filter the number to remove the bit in place of the one that is to me set
    uint16_t mask = ~((static_cast<uint16_t>(1)) << at);
    src = src & mask;

    //Set bool value
    return src | placedValueToSet ;
}

uint8_t SetBoolAtU8(uint8_t src, uint8_t at, bool value)
{
    uint8_t placedValueToSet = ((value ? static_cast<uint8_t>(1) : static_cast<uint8_t>(0)) << at);

    //filter the number to remove the bit in place of the one that is to me set
    uint8_t mask = ~((static_cast<uint8_t>(1)) << at);
    src = src & mask;

    //Set bool value
    return src | placedValueToSet ;
}

bool SampleBoolFromMask(uint8_t src, uint8_t at)
{
    return src >> at & 0x1u;
}

bool SampleBoolFromMask(uint16_t src, uint8_t at)
{
    return src >> at & 0x1u;
}

bool SampleBoolFromMask(uint32_t src, uint8_t at)
{
    return src >> at & 0x1u;
}

bool SampleBoolFromMask(uint64_t src, uint8_t at)
{
    return src >> at & 0x1u;
}

bool SampleBoolFromMask8(uint8_t src, uint8_t at)
{
    return src >> at & 0x1u;
}

bool SampleBoolFromMask16(uint16_t src, uint8_t at)
{
    return src >> at & 0x1u;
}

bool SampleBoolFromMask32(uint32_t src, uint8_t at)
{
    return src >> at & 0x1u;
}

bool SampleBoolFromMask64(uint64_t src, uint8_t at)
{
    return src >> at & 0x1u;
}

int8_t uint8_to_int8(uint8_t value)
{
    return static_cast<int8_t>(value ^ (1u << 7));
}

int16_t uint16_to_int16(uint16_t value)
{
    return static_cast<int16_t>(value ^ (1u << 15));
}

int32_t uint32_to_int32(uint32_t value)
{
    return static_cast<int32_t>(value ^ (1u << 31));
}

int64_t uint64_to_int64(uint64_t value)
{
    return static_cast<int64_t>(value ^ (1llu << 63));
}

uint8_t int8_to_uint8(int8_t value)
{
    return (uint8_t)(value ^ (1u << 7));
}

uint16_t int16_to_uint16(int16_t value)
{
    return (uint16_t)(value ^ (1u << 15));
}

uint32_t int32_to_uint32(int32_t value)
{
    return (uint32_t)(value ^ (1u << 31));
}

uint64_t int64_to_uint64(int64_t value)
{
    return (uint64_t)(value ^ (1llu << 63));
}
