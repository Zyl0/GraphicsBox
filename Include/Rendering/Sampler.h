#pragma once

#include <cstdint>

#include "Shared/Annotations.h"
#include "GLHelper.h"

class Sampler
{
public:
    enum Filter : uint8_t
    {
        F_Nearest,
        F_Linear
    };

    enum WarpMode : uint8_t
    {
        W_Repeat,
        W_MirrorRepeat,
        W_ClampToEdge,
        W_ClampToBorder
    };

    enum MipMode : uint8_t
    {
        M_NoMip,
        M_Nearest,
        M_Linear
    };

    struct Params
    {
        Filter Magnification = Filter::F_Linear;
        Filter Minification = Filter::F_Linear;

        WarpMode WarpModeU = W_Repeat;
        WarpMode WarpModeV = W_Repeat;
        WarpMode WarpModeW = W_Repeat;

        MipMode MipMode = MipMode::M_Linear;
        float LodBias = 0.0f;
        float LodMin = -1000.0f;
        float LodMax = 1000.0f;
    };

    Sampler(const Params& params);
    ~Sampler();

    void Data(const Params& params);

    INLINE GLuint Handle() const {return m_Sampler;}
    
private:
    GLuint m_Sampler;
};
