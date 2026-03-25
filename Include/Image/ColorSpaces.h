#pragma once

#include <cstdint>

#include "Shared/Annotations.h"
#include "Math/RMath.h"

struct ColorState
{
    enum EColorSpace : uint8_t
    {
        CS_Rec709 = 0,

        // DCI-P3 with white point at D65
        CS_DciP3D65,
        
        CS_Rec2020,

        // Alexa wide gamut
        CS_ArriLogC,

        CS_Count
    } ColorSpace;

    enum EGammaType : uint8_t
    {
        // Signal that requires OETF before being processed
        G_LightSignal = 0,

        // Linear Data
        G_Linear,

        // LogC
        G_ArriLogC,

        // Gamma (pow (1 / GammaValue)
        G_sRGB,

        // PQ curve
        G_PQ,

        G_Count
    } GammaCurve;

    float m_GammaValue = 2.2f;
    float m_PaperWhite = 300.0f;

    uint32_t PackModes() const
    {
        uint32_t pack = 0;

        pack |= ((uint32_t)ColorSpace) << 0;
        pack |= ((uint32_t)GammaCurve) << 8;

        return pack;
    }

    // G_sRGB parameters
    float sRGBInvGamma() const
    {
        return 1 / m_GammaValue;
    }
    float sRGBGamma() const
    {
        return m_GammaValue;
    }

    float PaperWhite() const
    {
        return m_PaperWhite;
    }
    float PQLuminance() const
    {
        // max value on ST2084 curve
        const float ST2084_MAX = 10000.0;
        
        return m_PaperWhite / ST2084_MAX;
    }
};

extern const char* ColorSpacesNames[ColorState::CS_Count];
extern const char* GammaTypesNames[ColorState::G_Count];

// Calculates conversion matrix to XYZ coords from RGB coords defined respective to the gamut provided by the input chromaticity values for color primaries
// The same RGB coords will output different XYZ coords depending on the gamut the RGB space is defined in...
Math::Matrix4f CalculateRGBtoXYZConversionMat( Math::Vector2f chromaRed, Math::Vector2f chromaGreen, Math::Vector2f chromaBlue, Math::Vector2f chromaWhite = /*D65*/ Math::Vector2f( 0.31270f, 0.32900f ) );

// Calculates conversion matrix to XYZ coords from RGB coords defined respective to the gamut provided by the input chromaticity values for color primaries
// The same RGB coords will output different XYZ coords depending on the gamut the RGB space is defined in...
//Assuming the white point luminance Y value is 100 like standardised in D65 mid-day sunlight (which is also the standard white in REC 709)
Math::Matrix4f CalculateRGBtoXYZConversionMatLuminanceAware( Math::Vector2f chromaRed, Math::Vector2f chromaGreen, Math::Vector2f chromaBlue, float WhitePointLuminance = 100.0f, Math::Vector2f chromaWhite = /*D65*/ Math::Vector2f( 0.31270f, 0.32900f ) );

namespace D65
{
    INLINE Math::Vector2f WhitePoint()    {return Math::Vector2f(0.31270f, 0.32900f);}
}

namespace Rec709
{
    INLINE Math::Vector2f WhitePoint()    {return D65::WhitePoint();}
    INLINE Math::Vector2f Red()           {return Math::Vector2f(0.64f, 0.33f);}
    INLINE Math::Vector2f Green()         {return Math::Vector2f(0.30f, 0.60f);}
    INLINE Math::Vector2f Blue()          {return Math::Vector2f(0.15f, 0.06f);}

    // https://en.wikipedia.org/wiki/Rec._709
    INLINE Math::Matrix4f ToXYZ() 
    { 
        return CalculateRGBtoXYZConversionMat( Red(), Green(), Blue(), WhitePoint());
    }

    INLINE Math::Matrix4f FromXYZ() { return Inverse(ToXYZ()); }
}

namespace DciP3D65
{
    INLINE Math::Vector2f WhitePoint()    {return D65::WhitePoint();}
    INLINE Math::Vector2f Red()           {return Math::Vector2f(0.680f, 0.320f);}
    INLINE Math::Vector2f Green()         {return Math::Vector2f(0.265f, 0.690f);}
    INLINE Math::Vector2f Blue()          {return Math::Vector2f(0.150f, 0.060f);}

    // https://en.wikipedia.org/wiki/DCI-P3
    INLINE Math::Matrix4f ToXYZ() 
    { 
        return CalculateRGBtoXYZConversionMat( Red(), Green(), Blue(), WhitePoint());
    }

    INLINE Math::Matrix4f FromXYZ() { return Inverse(ToXYZ()); }
}

namespace Rec2020
{
    INLINE Math::Vector2f WhitePoint()    {return D65::WhitePoint();}
    INLINE Math::Vector2f Red()           {return Math::Vector2f(0.708f, 0.292f);}
    INLINE Math::Vector2f Green()         {return Math::Vector2f(0.170f, 0.797f);}
    INLINE Math::Vector2f Blue()          {return Math::Vector2f(0.131f, 0.046f);}

    // https://en.wikipedia.org/wiki/Rec._2020
    INLINE Math::Matrix4f ToXYZ() 
    { 
        return CalculateRGBtoXYZConversionMat( Red(), Green(), Blue(), WhitePoint());
    }

    INLINE Math::Matrix4f FromXYZ() { return Inverse(ToXYZ()); }
}

namespace Rec2100
{
    INLINE Math::Vector2f WhitePoint()    {return Rec2020::WhitePoint();}
    INLINE Math::Vector2f Red()           {return Rec2020::Red();}
    INLINE Math::Vector2f Green()         {return Rec2020::Green();}
    INLINE Math::Vector2f Blue()          {return Rec2020::Blue();}

    // https://en.wikipedia.org/wiki/Rec._2100
    INLINE Math::Matrix4f ToXYZ()   { return Rec2020::ToXYZ(); };
    INLINE Math::Matrix4f FromXYZ() { return Rec2020::FromXYZ(); };
}

namespace ArriLogC
{
        // http://strattoncamera.com/pdf/11-06-30_Alexa_LogC_Curve.pdf
    // For EI 800
    struct EI800
    {
        static constexpr double cut = 0.010591;
        static constexpr double a = 5.555556;
        static constexpr double b = 0.052272;
        static constexpr double c = 0.247190;
        static constexpr double d = 0.385537;
        static constexpr double e = 5.367655;
        static constexpr double f = 0.092809;
    };

    const Math::Matrix3f CSArriLogCToCSXYZMat
    {
        // Check Row/Column major
        0.638008f, 0.214704f, 0.097744f,
        0.291954f, 0.823841f, -0.115795f,
        0.002798f, -0.067034f, 1.153294f
    };

    const Math::Matrix3f CSXYZToCSArriLogCMat
    {
        // Check Row/Column major
        1.789066f, -0.482534f, -0.200076f,
        -0.639849f, 1.396400f, 0.194432f,
        -0.041532f, 0.082335f, 0.878868f
    };
    

    INLINE Math::Vector3f CSArriLogCToCSXYZ(Math::Vector3f color)
    {
        return CSArriLogCToCSXYZMat * color;
    }

    INLINE Math::Vector3f CSXYZToCSArriLogC(Math::Vector3f color)
    {
        return CSXYZToCSArriLogCMat * color;
    }

    template <typename EI = EI800>
    INLINE Math::Vector3f GLinearToGArriLogC(Math::Vector3f color)
    {
        return (color > EI::cut) ? EI::c * log10(EI::a * color + EI::b) + EI::d : EI::e * color + EI::f;
    }

    template <typename EI = EI800>
    INLINE Math::Vector3f GArriLogCToGLinear(Math::Vector3f color)
    {
        return (color > (EI::e * EI::cut + EI::f)) ? (pow(10, (color - EI::d) / EI::c) - 2) / EI::a : (color - EI::f) / EI::e;
    }
}

namespace Spectral
{
    // Given a wavelength λ in nanometers, return XYZ color
    Math::Vector3f WavelengthToXYZ(float wavelength);
}
