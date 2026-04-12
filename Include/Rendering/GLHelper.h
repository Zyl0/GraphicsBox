#pragma once

#include "Shared/Assertion.h"

#include <GL/glew.h>

#define GL_CATCH_ERRORS 1

#if GL_CATCH_ERRORS == 1

#ifndef CONFIG_DEBUG
#define GL_NO_Breakpoint
#endif // CONFIG_DEBUG

#ifdef GL_NO_Breakpoint
#define GLCall(x)                                                   \
{                                                                   \
GLClearError();                                                     \
x;                                                                  \
GLLogCall(#x, __FILE__, __LINE__);                                  \
}
#else
#define GLCall(x)                                                   \
{                                                                   \
GLClearError();                                                     \
x;                                                                  \
Assert(GLLogCall(#x, __FILE__, __LINE__));                          \
}
#endif

#else
#define GLCall(x) x;
#endif

#define GlBufferNone 0


void GLClearError();

bool GLLogCall(const char* function, const char* file, int line);

int miplevels(const int width, const int height);

int miplevels(const int width, const int height, const int depth);

// Mark a feature unimplemented with current API
// Crash the program if reached
#define UNIMPLEMENTED_FEATURE           EngineRuntimeCrash("Unimplemented feature for current API")

// Mark a feature unimplemented with current API
// Log an error if reached
#define UNIMPLEMENTED_FEATURE_AS_ERR    EngineLoggerError("Unimplemented feature for current API");
