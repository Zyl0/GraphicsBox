#pragma once

#include <stdlib.h>

#include "Logger.h"

#define _UNSUPPORTED_FEATURE static_assert(false, "Unsupported platform. You are most likely using the wrong platform header.")

// TODO mode to premake
#define BREAKPOINT_ENABLE 1

#if BREAKPOINT_ENABLE == 0
#define EngineRuntimeBREAKPOINT 
#endif // BREAKPOINT_ENABLE == 0

#ifdef PLATFORM_LINUX
#if defined(__GNUC__) || defined(__clang__)

#if BREAKPOINT_ENABLE == 1
#include <signal.h>

/**
* @brief trigger a breakpoint in debug mode.
* @warning
* On linux platform this will close the application outside of debug mode.
*/
#define EngineRuntimeBREAKPOINT raise(SIGINT);
#endif // BREAKPOINT_ENABLE == 1

/**
 * Mark code branch as unreachable (for compiler optimisations).
 */
#define PLATFORM_UNREACHABLE  __builtin_unreachable()

// TODO document
#define PLATFORM_INLINE inline __attribute__((always_inline))

/**
 * Allocate heap aligned memory
 * @param Type Pointer type
 * @param MemPtr Pointer that will hold the allocated memory
 * @param Alignment Memory alignment in bytes
 * @param Size Allocation desired size. Must be a multiple of Alignment
 */
#define PLATFORM_ALIGNED_MALLOC(Type, MemPtr, Alignment, Size) MemPtr = (Type*)aligned_alloc(Size, Alignment);

/**
 * Re heap allocate aligned memory
 * @param MemPtr Pointer that holds the allocated memory
 * @param Alignment Memory alignment in bytes
 * @param Size Allocation desired size. Must be a multiple of Alignment
 */
#define PLATFORM_ALIGNED_REALLOC(MemPtr, Alignment, Size) realloc(MemPtr, Size);

/**
 * Free heap aligned memory
 * @param MemPtr Pointer that holds the allocated memory
 * @param Alignment Memory alignment in bytes
 * @param Size Allocation desired size. Must be a multiple of Alignment
 */
#define PLATFORM_ALIGNED_FREE(MemPtr) free(MemPtr);

#define PLATFORM_LIB_EXPORT 
#define PLATFORM_LIB_IMPORT 

#endif //__GNUC__ || __clang__
#endif //PLATFORM_LINUX

#ifdef PLATFORM_WINDOWS
#if defined(_MSC_VER)

#if BREAKPOINT_ENABLE == 1
/**
* @brief trigger a breakpoint in debug mode.
*/
#define EngineRuntimeBREAKPOINT (void) fflush(stdout); __debugbreak();
#endif // BREAKPOINT_ENABLE == 1

/**
 * Mark code branch as unreachable (for compiler optimisations).
 */
#define PLATFORM_UNREACHABLE __assume(false)

// TODO document
#define PLATFORM_INLINE __forceinline 

#ifdef CONFIG_DEBUG
/**
 * Allocate heap aligned memory
 * @param Type Pointer type
 * @param MemPtr Pointer that will hold the allocated memory
 * @param Alignment Memory alignment in bytes
 * @param Size Allocation desired size. Must be a multiple of Alignment
 */
#define PLATFORM_ALIGNED_MALLOC(Type, MemPtr, Alignment, Size) MemPtr = (Type*)_aligned_malloc_dbg(Size, Alignment, __FILE__, __LINE__);

/**
 * Re heap allocate aligned memory
 * @param MemPtr Pointer that holds the allocated memory
 * @param Alignment Memory alignment in bytes
 * @param Size Allocation desired size. Must be a multiple of Alignment
 */
#define PLATFORM_ALIGNED_REALLOC(MemPtr, Alignment, Size) _aligned_realloc_dbg(MemPtr, Size, Alignment, __FILE__, __LINE__);

/**
 * Free heap aligned memory
 * @param MemPtr Pointer that holds the allocated memory
 * @param Alignment Memory alignment in bytes
 * @param Size Allocation desired size. Must be a multiple of Alignment
 */
#define PLATFORM_ALIGNED_FREE(MemPtr) _aligned_free_dbg(MemPtr);

#else // CONFIG_DEBUG

/**
 * Allocate heap aligned memory
 * @param Type Pointer type
 * @param MemPtr Pointer that will hold the allocated memory
 * @param Alignment Memory alignment in bytes
 * @param Size Allocation desired size. Must be a multiple of Alignment
 */
#define PLATFORM_ALIGNED_MALLOC(Type, MemPtr, Alignment, Size) MemPtr = (Type*)_aligned_malloc(Size, Alignment);

/**
 * Re heap allocate aligned memory
 * @param MemPtr Pointer that holds the allocated memory
 * @param Alignment Memory alignment in bytes
 * @param Size Allocation desired size. Must be a multiple of Alignment
 */
#define PLATFORM_ALIGNED_REALLOC(MemPtr, Alignment, Size) _aligned_realloc(MemPtr, Size, Alignment);

/**
 * Free heap aligned memory
 * @param MemPtr Pointer that holds the allocated memory
 * @param Alignment Memory alignment in bytes
 * @param Size Allocation desired size. Must be a multiple of Alignment
 */
#define PLATFORM_ALIGNED_FREE(MemPtr) _aligned_free(MemPtr);
#endif // !CONFIG_DEBUG

#define PLATFORM_LIB_EXPORT __declspec(dllexport)
#define PLATFORM_LIB_IMPORT __declspec(dllimport)

#endif //_MSC_VER
#endif //PLATFORM_WINDOWS

/**
 * @brief trigger a breakpoint, print an error message and close the program
 * @param error error message
*/
#define EngineRuntimeCrash(error) \
    EngineLoggerFatal(error); \
    EngineRuntimeBREAKPOINT\
    exit(EXIT_FAILURE);

/**
 * @brief trigger a breakpoint, print an error message and close the program
 * @param error error message
*/
#define EngineRuntimeCrashF(error,...) \
    EngineLoggerFatalF(error, __VA_ARGS__); \
    EngineRuntimeBREAKPOINT\
    exit(EXIT_FAILURE);
