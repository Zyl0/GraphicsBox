#pragma once

#include "Platform.h"

#ifdef CONFIG_DEBUG
// Run an instruction only in debug config. (Here enabled)
#define DEBUG_INSTRUCTION(exec) exec
#else
// Run an instruction only in debug config. (Here disabled)
#define DEBUG_INSTRUCTION(exec)
#endif

// Mark a section of code unreachable
#define UNREACHABLE  PLATFORM_UNREACHABLE

// Mark a symbol to be inlined
#define INLINE  PLATFORM_INLINE

// Ignore the returned value of a call
#define IGNORE_RETURN (void)

// Ignore the error code returned by a call
#define IGNORE_ERROR_CODE IGNORE_RETURN

#ifdef CONFIG_DEBUG
// Crash the program when you are using an enum value that is not supported. Prompt an error message.
#define ENUM_OUT_OF_RANGE(error)  EngineRuntimeCrash(error)

// Crash the program when you are using an enum value that is not supported.Prompt an error message with a format.
#define ENUM_OUT_OF_RANGEF(format, ...)  EngineRuntimeCrashF(format, __VA_ARGS__)

#else
// Mark the default statement as unreachable. Enum switches should be tested in debug mode beforehand to ensure they are well managed
#define ENUM_OUT_OF_RANGE(error) UNREACHABLE;

// Mark the default statement as unreachable. Enum switches should be tested in debug mode beforehand to ensure they are well managed
#define ENUM_OUT_OF_RANGEF(format, ...)  UNREACHABLE;
#endif

#ifdef CONFIG_DEBUG
// Crash the program if you are using an enum value that is not managed by a switch statement. Prompt an error message.
#define SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE(error) \
    default: EngineRuntimeCrash(error)

// Crash the program if you are using an enum value that is not managed by a switch statement.Prompt an error message with a format.
#define SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGEF(format, ...) \
    default: EngineRuntimeCrashF(format, __VA_ARGS__)

#else
// Mark the default statement as unreachable. Enum switches should be tested in debug mode beforehand to ensure they are well managed
#define SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE(error) \
    default: UNREACHABLE;

// Mark the default statement as unreachable. Enum switches should be tested in debug mode beforehand to ensure they are well managed
#define SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGEF(format, ...) \
    default: UNREACHABLE;
#endif


// todo  maybe move to platform headers.
#ifndef NO_DISCARD
    #if defined(__GNUC__) || defined(__clang__)
        #define NO_DISCARD __attribute__((warn_unused_result))
    #elif defined(__cplusplus) && __cplusplus >= 201703L
        #define NO_DISCARD [[nodiscard]]
    #elif defined(_MSC_VER)
        #define NO_DISCARD _Check_return_
    #else
        #define NO_DISCARD
    #endif
#endif

//todo  maybe to platform header
#if defined(__GNUC__) || defined(__clang__)
#define ALIGNED(Alignment) __attribute__((aligned(Alignment)))
#elif defined(_MSC_VER)
// todo find MSVC dedicated memory alignment annotation
#define ALIGNED(Alignment) 
//alignas(Alignment)
#else
#define ALIGNED(Alignment)
//alignas(Alignment)
#endif

#define DYNAMIC_LIB_EXPORT PLATFORM_LIB_EXPORT
#define DYNAMIC_LIB_IMPORT PLATFORM_LIB_IMPORT
