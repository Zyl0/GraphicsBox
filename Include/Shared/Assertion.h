#pragma once

#include "Logger.h"
#include "Platform.h"

#ifdef CONFIG_DEBUG
//#define USE_BREAKPOINT_ASSERT_WARNING
#define USE_BREAKPOINT_ASSERT_ERROR
// #define USE_BREAKPOINT_ASSERT_FATAL
#endif // CONFIG_DEBUG
#ifdef CONFIG_DEVELOPMENT
//#define USE_BREAKPOINT_ASSERT_WARNING
#define USE_BREAKPOINT_ASSERT_ERROR
// #define USE_BREAKPOINT_ASSERT_FATAL
#endif // CONFIG_DEVELOPMENT
#ifdef CONFIG_RELEASE
//#define USE_BREAKPOINT_ASSERT_WARNING
//#define USE_BREAKPOINT_ASSERT_ERROR
// #define USE_BREAKPOINT_ASSERT_FATAL
#endif // CONFIG_RELEASE


#ifdef USE_BREAKPOINT_ASSERT_WARNING
#define _AssertWarnBreak EngineRuntimeBREAKPOINT
#else // USE_BREAKPOINT_ASSERT_WARNING
#define _AssertWarnBreak 
#endif // !USE_BREAKPOINT_ASSERT_WARNING

#ifdef USE_BREAKPOINT_ASSERT_ERROR
#define _AssertErrorBreak EngineRuntimeBREAKPOINT
#else // USE_BREAKPOINT_ASSERT_ERROR
#define _AssertErrorBreak 
#endif // !USE_BREAKPOINT_ASSERT_ERROR

// #ifdef USE_BREAKPOINT_ASSERT_FATAL
// #define _AssertFatalBreak EngineRuntimeBREAKPOINT
// #else // USE_BREAKPOINT_ASSERT_FATAL
// #define _AssertFatalBreak 
// #endif // !USE_BREAKPOINT_ASSERT_FATAL

namespace engine::assertion
{
    /**
         * @brief assert something by performing a test on a condition. If the test fail, triggers a breakpoint.
         * @param test
        * @param onFailed a piece of code to run in case the test failed
        */
#define _AssertOrExec(test, onFailed)\
			if (!(test))\
			{\
				onFailed;\
			}

    /**
         * @brief assert something by performing a test on a condition. If the test fail, triggers a breakpoint.
         * @param test
        */
#define Assert(test) _AssertOrExec(test, EngineRuntimeCrashF("Assertion (%s) failed", #test))

    /**
         * @brief assert something by performing a test on a condition. If the test fail, triggers a breakpoint.
         * @param test
         * @param exec code to run in cas assertion failed
        */
#define AssertOrCall(test, exec) _AssertOrExec(test, EngineLoggerErrorF("Assertion (%s) failed", #test); exec)

    /**
         * @brief assert something by performing a test on a condition. If the test fail, triggers a breakpoint.
         * @param test
         * @param exec code to run in cas assertion failed
         * @param message error message to print on assertion failed
        */
#define AssertOrErrorCall(test, exec, message) _AssertOrExec(test, EngineLoggerErrorF("Assertion (%s) failed - " #message, #test);  _AssertErrorBreak exec)
    /**
         * @brief assert something by performing a test on a condition. If the test fail, triggers a breakpoint.
         * @param test
         * @param exec code to run in cas assertion failed
         * @param message error message to print on assertion failed
        */
#define AssertOrErrorCallF(test, exec, message,...) _AssertOrExec(test, EngineLoggerErrorF("Assertion (%s) failed - " #message, #test, __VA_ARGS__); _AssertErrorBreak exec)

    /**
         * @brief assert something by performing a test on a condition. If the test fail, triggers a breakpoint and crash the program.
         * @param test
         * @param message error message to print on assertion failed
        */
#define AssertOrError(test, message) _AssertOrExec(test, EngineRuntimeCrashF("Assertion (%s) failed - " #message, #test);)
    /**
         * @brief assert something by performing a test on a condition. If the test fail, triggers a breakpoint and crash the program.
         * @param test
         * @param message error message to print on assertion failed
        */
#define AssertOrErrorF(test, message,...) _AssertOrExec(test, EngineRuntimeCrashF("Assertion (%s) failed - " #message, #test, __VA_ARGS__);)


    /**
         * @brief assert something by performing a test on a condition. If the test fail, triggers a breakpoint.
         * @param test
         * @param exec code to run in cas assertion failed
         * @param message warning to print on assertion failed
        */
#define AssertOrWarnCall(test, exec, message) _AssertOrExec(test, EngineLoggerWarnF("Assertion (%s) failed - " #message, #test); _AssertWarnBreak exec)
    /**
         * @brief assert something by performing a test on a condition. If the test fail, triggers a breakpoint.
         * @param test
         * @param exec code to run in cas assertion failed
         * @param message warning to print on assertion failed
        */
#define AssertOrWarnCallF(test, exec, message,...) _AssertOrExec(test, EngineLoggerWarnF("Assertion (%s) failed - " #message, #test, __VA_ARGS__); _AssertWarnBreak exec)

    /**
         * @brief assert something by performing a test on a condition. If the test fail, triggers a breakpoint.
         * @param test
         * @param exec code to run in cas assertion failed
         * @param message log to print on assertion failed
        */
#define AssertOrLogCall(test, exec, message) _AssertOrExec(test, EngineLoggerLogF("Assertion (%s) failed - " #message, #test); exec)
    /**
         * @brief assert something by performing a test on a condition. If the test fail, triggers a breakpoint.
         * @param test
         * @param exec code to run in cas assertion failed
         * @param message log to print on assertion failed
        */
#define AssertOrLogCallF(test, exec, message,...) _AssertOrExec(test, EngineLoggerLogF("Assertion (%s) failed - " #message, #test, __VA_ARGS__); exec)
}
