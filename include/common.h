#ifndef EMURPC_COMMON_H
#define EMURPC_COMMON_H

#ifdef __cplusplus
#include <cstdint>
typedef std::uint8_t u8;   ///< 8-bit unsigned byte
typedef std::uint16_t u16; ///< 16-bit unsigned short
typedef std::uint32_t u32; ///< 32-bit unsigned word
typedef std::uint64_t u64; ///< 64-bit unsigned int

typedef std::int8_t s8;   ///< 8-bit signed byte
typedef std::int16_t s16; ///< 16-bit signed short
typedef std::int32_t s32; ///< 32-bit signed word
typedef std::int64_t s64; ///< 64-bit signed int
#else
#include <stdbool.h>
#include <stdint.h>
typedef uint8_t u8;   ///< 8-bit unsigned byte
typedef uint16_t u16; ///< 16-bit unsigned short
typedef uint32_t u32; ///< 32-bit unsigned word
typedef uint64_t u64; ///< 64-bit unsigned int

typedef int8_t s8;   ///< 8-bit signed byte
typedef int16_t s16; ///< 16-bit signed short
typedef int32_t s32; ///< 32-bit signed word
typedef int64_t s64; ///< 64-bit signed int
#endif

typedef float f32;  ///< 32-bit floating point
typedef double f64; ///< 64-bit floating point

#ifdef _MSC_VER
#ifndef __func__
#define __func__ __FUNCTION__
#endif
#endif

#endif
