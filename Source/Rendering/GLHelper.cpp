#include "GLHelper.h"

#include <algorithm>

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        EngineLoggerErrorF("[GL error %d] when calling %s (file %s, line %d)", error, function, file, line);
        return false;
    }
    return true;
}

int miplevels(const int width, const int height)
{
    int w = width;
    int h = height;
    int levels = 1;
    while (w > 1 || h > 1)
    {
        w = std::max(1, w / 2);
        h = std::max(1, h / 2);
        levels = levels + 1;
    }

    return levels;
}

int miplevels(const int width, const int height, const int depth)
{
    int w = width;
    int h = height;
    int d = height;
    int levels = 1;
    while (w > 1 || h > 1 || d > 1)
    {
        w = std::max(1, w / 2);
        h = std::max(1, h / 2);
        d = std::max(1, d / 2);
        levels = levels + 1;
    }

    return levels;
}