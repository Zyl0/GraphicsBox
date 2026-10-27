#include "ColorSpaces.h"

const char* ColorSpacesNames[ColorState::CS_Count] = {
    "Rec709",
    "DciP3D65",
    "Rec2020",
    "ArriLogC"
};

const char* GammaTypesNames[ColorState::G_Count] = {
    "LightSignal",
    "Linear",
    "ArriLogC",
    "sRGB",
    "PQ"
};

// Calculates conversion matrix to XYZ coords from RGB coords defined respective to the gamut provided by the input chromaticity values for color primaries
// The same RGB coords will output different XYZ coords depending on the gamut the RGB space is defined in...
Math::Matrix3f CalculateRGBtoXYZConversionMat( Math::Vector2f chromaRed, Math::Vector2f chromaGreen, Math::Vector2f chromaBlue, Math::Vector2f chromaWhite )
{
    // Convert xy to XYZ with Y=1
    auto toXYZ = [](float x, float y) -> Math::Vector3f
    {
        return Math::Vector3f{ x / y, 1.0, (1 - x - y) / y };
    };

    Math::Vector3f X_r = toXYZ(chromaRed.x, chromaRed.y);
    Math::Vector3f X_g = toXYZ(chromaGreen.x, chromaGreen.y);
    Math::Vector3f X_b = toXYZ(chromaBlue.x, chromaBlue.y);
    Math::Vector3f W = toXYZ(chromaWhite.x, chromaWhite.y);

    // Build matrix M from columns of primaries
    Math::Matrix3f M = {
        X_r[0], X_g[0], X_b[0],
        X_r[1], X_g[1], X_b[1],
        X_r[2], X_g[2], X_b[2]
    };

    // Invert M matrix
    Math::Matrix3f Minv = Inverse(M);

    // Solve for scaling factors: S = Minv * W
    Math::Vector3f S = Minv * W;

    // Scale columns of M by S to get RGB to XYZ
    Math::Matrix3f RGBtoXYZ = M * Math::Matrix3f::Scale(S);

    return RGBtoXYZ;
}

// Calculates conversion matrix to XYZ coords from RGB coords defined respective to the gamut provided by the input chromaticity values for color primaries
// The same RGB coords will output different XYZ coords depending on the gamut the RGB space is defined in...
//Assuming the white point luminance Y value is 100 like standardised in D65 mid-day sunlight (which is also the standard white in REC 709)
Math::Matrix3f CalculateRGBtoXYZConversionMatLuminanceAware( Math::Vector2f chromaRed, Math::Vector2f chromaGreen, Math::Vector2f chromaBlue, float WhitePointLuminance, Math::Vector2f chromaWhite )
{
    //First with the additional assumption of white.Y == 100, calculate the XYZ coords of white // TODO check
    Math::Vector3f chromaWhite_xyz = Math::Vector3f( chromaWhite.x, chromaWhite.y, 1.0f - chromaWhite.x - chromaWhite.y );
    Math::Vector3f whiteXYZ = chromaWhite_xyz * WhitePointLuminance / chromaWhite_xyz.y;

    //Having the proper white in XYZ coords we can proceed to calculate the remaining XYZ coords for color primaries
    Math::Vector3f primaryX = Math::Vector3f( chromaRed.x / chromaRed.y, chromaGreen.x / chromaGreen.y, chromaBlue.x / chromaBlue.y );
    Math::Vector3f primaryY = Math::Vector3f( 1, 1, 1 );
    Math::Vector3f primaryZ = Math::Vector3f(
        ( 1 - chromaRed.x - chromaRed.y ) / chromaRed.y,
        ( 1 - chromaGreen.x - chromaGreen.y ) / chromaGreen.y,
        ( 1 - chromaBlue.x - chromaBlue.y ) / chromaBlue.y );

    Math::Matrix3f M(  primaryX , primaryY ,  primaryZ );
    Math::Transpose(M); // we use row vectors in the engine Matrix ( mul defined as v * M, instead of M * v) 
    Math::Matrix3f invM = Math::Inverse(M);

    Math::Vector3f S = invM * whiteXYZ;
    
    const Math::Matrix3f RGBtoXYZ = ( Math::Matrix3f::Scale(S.xyz()) * M );

    return RGBtoXYZ;
}

float PQ::OETF(float linear)
{
    // Constants from SMPTE ST 2084-2014
    const float m1 = 0.1593017578125; // = 2610. / 4096. * .25;
    const float m2 = 78.84375; // = 2523. / 4096. *  128;
    const float c1 = 0.8359375; // = 2392. / 4096. * 32 - 2413./4096.*32 + 1;
    const float c2 = 18.8515625; // = 2413. / 4096. * 32;
    const float c3 = 18.6875; // = 2392. / 4096. * 32;
    const float C = 1.0f;

    float L = linear / C;
    float Lm = pow(L, m1);
    float N1 = (c1 + c2 * Lm);
    float N2 = (1.0f + c3 * Lm);
    float N = N1 / N2;
    float P = pow(N, m2);

    return P;
}

float PQ::EOTF(float PQ)
{
    // Constants from SMPTE ST 2084-2014
    const float m1 = 0.1593017578125; // = 2610. / 4096. * .25;
    const float m2 = 78.84375; // = 2523. / 4096. *  128;
    const float c1 = 0.8359375; // = 2392. / 4096. * 32 - 2413./4096.*32 + 1;
    const float c2 = 18.8515625; // = 2413. / 4096. * 32;
    const float c3 = 18.6875; // = 2392. / 4096. * 32;
    const float C = 1.0f;

    float Np = pow(PQ, 1.f / m2);
    float L = Np - c1;
    L = std::max(0.f, L);
    L = L / (c2 - c3 * Np);
    L = pow(L, 1.f / m1);
    float P = L * C;

    return P;
}

double PQ::OETF(double linear)
{
    // Constants from SMPTE ST 2084-2014
    const double m1 = 0.1593017578125; // = 2610. / 4096. * .25;
    const double m2 = 78.84375; // = 2523. / 4096. *  128;
    const double c1 = 0.8359375; // = 2392. / 4096. * 32 - 2413./4096.*32 + 1;
    const double c2 = 18.8515625; // = 2413. / 4096. * 32;
    const double c3 = 18.6875; // = 2392. / 4096. * 32;
    const double C = 1.0f;

    double L = linear / C;
    double Lm = pow(L, m1);
    double N1 = (c1 + c2 * Lm);
    double N2 = (1.0 + c3 * Lm);
    double N = N1 / N2;
    double P = pow(N, m2);

    return P;
}

double PQ::EOTF(double PQ)
{
    // Constants from SMPTE ST 2084-2014
    const float m1 = 0.1593017578125; // = 2610. / 4096. * .25;
    const float m2 = 78.84375; // = 2523. / 4096. *  128;
    const float c1 = 0.8359375; // = 2392. / 4096. * 32 - 2413./4096.*32 + 1;
    const float c2 = 18.8515625; // = 2413. / 4096. * 32;
    const float c3 = 18.6875; // = 2392. / 4096. * 32;
    const float C = 1.0f;

    double Np = pow(PQ, 1. / m2);
    double L = Np - c1;
    L = std::max(0., L);
    L = L / (c2 - c3 * Np);
    L = pow(L, 1. / m1);
    double P = L * C;

    return P;
}

float sRGB::OETF(float linear)
{
    constexpr float alpha = 0.055;
    constexpr float gamma = 2.4;
    return linear > 0.0031308f ?
        (1.f + alpha) * pow(linear, 1.f / gamma) - alpha :
        12.95f * linear;
}

float sRGB::OETF(float linear, float gamma, float alpha)
{
    return linear > 0.0031308f ?
        (1.f + alpha) * pow(linear, 1.f / gamma) - alpha :
        12.95f * linear;
}

float sRGB::EOTF(float sRGB)
{
    constexpr float alpha = 0.055;
    constexpr float gamma = 2.4;
    return sRGB > 0.04045f ?
        pow((sRGB + alpha) / (1.f + alpha),  gamma):
        sRGB / 12.92;
}

float sRGB::EOTF(float sRGB, float gamma, float alpha)
{
    return sRGB > 0.04045f ?
        pow((sRGB + alpha) / (1.f + alpha),  gamma):
        sRGB / 12.92;
}

double sRGB::OETF(double linear)
{
    constexpr float alpha = 0.055;
    constexpr float gamma = 2.4;
    return linear > 0.0031308f ?
        (1.f + alpha) * pow(linear, 1.f / gamma) - alpha :
        12.95f * linear;
}

double sRGB::OETF(double linear, double gamma, double alpha)
{
    return linear > 0.0031308f ?
    (1.f + alpha) * pow(linear, 1.f / gamma) - alpha :
    12.95f * linear;
}

double sRGB::EOTF(double sRGB)
{
    constexpr float alpha = 0.055;
    constexpr float gamma = 2.4;
    return sRGB > 0.04045f ?
        pow((sRGB + alpha) / (1.f + alpha),  gamma):
        sRGB / 12.92;
}

double sRGB::EOTF(double sRGB, double gamma, double alpha)
{
    return sRGB > 0.04045f ?
        pow((sRGB + alpha) / (1.f + alpha),  gamma):
        sRGB / 12.92;
}

Math::Vector3f Spectral::WavelengthToXYZ(float wavelength)
{
    // Simple piecewise linear interpolation for CIE 1931 color matching functions
    // Data points sampled approximately every 20nm from 380nm to 780nm
    const int sampleCount = 21;

    static const float lambdaSamples[21] =
    {
        380.0, 400.0, 420.0, 440.0, 460.0, 480.0, 500.0, 520.0, 540.0, 560.0, 580.0, 600.0, 620.0, 640.0, 660.0, 680.0, 700.0, 720.0, 740.0, 760.0, 780.0
    };


    // Corresponding x̅(λ), y̅(λ), z̅(λ) values sampled from CIE 1931 2° Standard Observer
    static const float xBarSamples[21] =
    {
        0.0014, 0.0143, 0.1344, 0.3483, 0.2908, 0.0956, 0.0049, 0.0633, 0.2904, 0.5945, 0.9163, 1.0622, 0.8544, 0.4177, 0.1752, 0.0610, 0.0235, 0.0073, 0.0032, 0.0012, 0.0000
    };
    static const float yBarSamples[21] =
    {
        0.0000, 0.0004, 0.0040, 0.0230, 0.0900, 0.2177, 0.4776, 0.7570, 0.9540, 0.9950, 0.8700, 0.6310, 0.3810, 0.1750, 0.0610, 0.0170, 0.0041, 0.0010, 0.0002, 0.0000, 0.0000
    };
    static const float zBarSamples[21] =
    {
        0.0065, 0.0679, 0.6456, 1.7471, 1.6692, 1.2876, 0.8130, 0.4652, 0.2720, 0.1582, 0.0782, 0.0422, 0.0203, 0.0087, 0.0039, 0.0021, 0.0017, 0.0011, 0.0008, 0.0003, 0.0000
    };

    
    // Clamp wavelength to visible range
    wavelength = Math::Clamp(wavelength, 380.f, 780.f);
    // Find the interval in the lambdaSamples
    int index = 0;
    for (int i = 0; i < sampleCount - 1; i++) 
    {
        if (wavelength >= lambdaSamples[i] && wavelength <= lambdaSamples[i+1]) 
        {
            index = i;
            break;
        }
    }
    float t = (wavelength - lambdaSamples[index]) / (lambdaSamples[index+1] - lambdaSamples[index]);
    float X = Math::LinearInterpolate<float, float>(xBarSamples[index], xBarSamples[index+1], t);
    float Y = Math::LinearInterpolate<float, float>(yBarSamples[index], yBarSamples[index+1], t);
    float Z = Math::LinearInterpolate<float, float>(zBarSamples[index], zBarSamples[index+1], t);
    return Math::Vector3f(X, Y, Z);
}
