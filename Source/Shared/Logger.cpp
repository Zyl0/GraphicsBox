#include "Logger.h"

//#define USE_CSTYLE_VA_EXTRACTION

#include <stdarg.h>
#include <stdio.h>

#ifndef _LOGGER_UseHeaderSTDIO
#ifndef USE_CSTYLE_VA_EXTRACTION

#else
#include <alloca.h>
#include <string.h>
#include <malloc.h>
#endif

void NAMESPACE_ENGINE::Logger::Log(const char* file, unsigned int line, const char* level, const char* format, ...)
{
#ifndef USE_CSTYLE_VA_EXTRACTION
    printf("[%s : %s line %u] ", level, file, line);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf(".\n");
#else
    size_t size = strlen(format) + 512;
    char* log = (char*)alloca(size);
    va_list args;
    va_start(args, format);
    vsnprintf(log, size, format, args);
    va_end(args);
    printf("[%s : %s line %d] %s\n", level, file, line, log);
#endif
}
#endif