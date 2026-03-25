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
Math::Matrix4f CalculateRGBtoXYZConversionMat( Math::Vector2f chromaRed, Math::Vector2f chromaGreen, Math::Vector2f chromaBlue, Math::Vector2f chromaWhite )
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
    Math::Matrix3f RGBtoXYZ = M * MakeScale(S);

    return Math::ToTransform4D(RGBtoXYZ);
}

// Calculates conversion matrix to XYZ coords from RGB coords defined respective to the gamut provided by the input chromaticity values for color primaries
// The same RGB coords will output different XYZ coords depending on the gamut the RGB space is defined in...
//Assuming the white point luminance Y value is 100 like standardised in D65 mid-day sunlight (which is also the standard white in REC 709)
Math::Matrix4f CalculateRGBtoXYZConversionMatLuminanceAware( Math::Vector2f chromaRed, Math::Vector2f chromaGreen, Math::Vector2f chromaBlue, float WhitePointLuminance, Math::Vector2f chromaWhite )
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

    Math::Matrix4f M( Math::Vector4f( primaryX, 0.f ), Math::Vector4f( primaryY, 0.f ), Math::Vector4f( primaryZ, 0.f ), Math::Vector4f( 0.f, 0.f, 0.f, 1.f ) );
    Math::Transpose(M); // we use row vectors in the engine Matrix ( mul defined as v * M, instead of M * v) 
    Math::Matrix4f invM = Math::Inverse(M);

    Math::Vector4f S = invM * Math::Vector4f(whiteXYZ, 1.0f);
    S /= S.w;
    
    const Math::Matrix4f RGBtoXYZ = ( Math::MakeHomogeneousScale(S.xyz()) * M );

    return RGBtoXYZ;
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
