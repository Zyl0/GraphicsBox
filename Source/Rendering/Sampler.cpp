#include "Sampler.h"

static
GLint ToGLFilter(Sampler::Filter filter)
{
    switch (filter)
    {
    case Sampler::F_Nearest:    return GL_NEAREST;
    case Sampler::F_Linear:     return GL_LINEAR;

    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported filter type")
    }
}

static
GLint ToGLWarpMode(Sampler::WarpMode mode)
{
    switch (mode)
    {
    case Sampler::W_Repeat:             return GL_REPEAT;
    case Sampler::W_MirrorRepeat:       return GL_MIRRORED_REPEAT;
    case Sampler::W_ClampToEdge:        return GL_CLAMP_TO_EDGE;
    case Sampler::W_ClampToBorder:      return GL_CLAMP_TO_BORDER;

    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported filter type")
    }
}


Sampler::Sampler(const Params& params)
{
    GLCall(glGenSamplers(1, &m_Sampler))

    Data(params);
}

Sampler::~Sampler()
{
    GLCall(glDeleteSamplers(1, &m_Sampler))
}

void Sampler::Data(const Params& params)
{
    if (params.MipMode == M_NoMip)
    {
        glSamplerParameteri(m_Sampler, GL_TEXTURE_MIN_FILTER, ToGLFilter(params.Minification));
    }
    else if (params.MipMode == M_Nearest)
    {
        if (params.Minification == Sampler::F_Nearest)
        {
            glSamplerParameteri(m_Sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        }
        else if (params.Minification == Sampler::F_Linear)
        {
            glSamplerParameteri(m_Sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        }
        else
        {
            ENUM_OUT_OF_RANGE("Unsupported filter type")
        }
            
    }
    else if (params.MipMode == M_Linear)
    {
        if (params.Minification == Sampler::F_Nearest)
        {
            glSamplerParameteri(m_Sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        }
        else if (params.Minification == Sampler::F_Linear)
        {
            glSamplerParameteri(m_Sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }
        else
        {
            ENUM_OUT_OF_RANGE("Unsupported filter type")
        }
    }
    else
    {
        ENUM_OUT_OF_RANGE("Unsupported mip mode")
    }
    glSamplerParameteri(m_Sampler, GL_TEXTURE_MAG_FILTER, ToGLFilter(params.Magnification));
    glSamplerParameteri(m_Sampler, GL_TEXTURE_WRAP_S, ToGLWarpMode(params.WarpModeU));
    glSamplerParameteri(m_Sampler, GL_TEXTURE_WRAP_T, ToGLWarpMode(params.WarpModeV));
    glSamplerParameteri(m_Sampler, GL_TEXTURE_WRAP_R, ToGLWarpMode(params.WarpModeW));
    glSamplerParameterf(m_Sampler, GL_TEXTURE_MIN_LOD, params.LodMin);
    glSamplerParameterf(m_Sampler, GL_TEXTURE_MAX_LOD, params.LodMax);
    glSamplerParameterf(m_Sampler, GL_TEXTURE_LOD_BIAS, params.LodBias);
 
}