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
Matrix4f CalculateRGBtoXYZConversionMat( Vector2f chromaRed, Vector2f chromaGreen, Vector2f chromaBlue, Vector2f chromaWhite = /*D65*/ Vector2f( 0.31270f, 0.32900f ) )
{
    // Convert xy to XYZ with Y=1
    auto toXYZ = [](float x, float y) -> Vector3f
    {
        return Vector3f{ x / y, 1.0, (1 - x - y) / y };
    };

    Vector3f X_r = toXYZ(chromaRed.x, chromaRed.y);
    Vector3f X_g = toXYZ(chromaGreen.x, chromaGreen.y);
    Vector3f X_b = toXYZ(chromaBlue.x, chromaBlue.y);
    Vector3f W = toXYZ(chromaWhite.x, chromaWhite.y);

    // Build matrix M from columns of primaries
    Matrix3f M = {
        X_r[0], X_g[0], X_b[0],
        X_r[1], X_g[1], X_b[1],
        X_r[2], X_g[2], X_b[2]
    };

    // Invert M matrix
    Matrix3f Minv = Inverse(M);

    // Solve for scaling factors: S = Minv * W
    Vector3f S = Minv * W;

    // Scale columns of M by S to get RGB to XYZ
    Matrix3f RGBtoXYZ = M * MakeScale(S);

    return Math::ToTransform4D(RGBtoXYZ);
}

// Calculates conversion matrix to XYZ coords from RGB coords defined respective to the gamut provided by the input chromaticity values for color primaries
// The same RGB coords will output different XYZ coords depending on the gamut the RGB space is defined in...
//Assuming the white point luminance Y value is 100 like standardised in D65 mid-day sunlight (which is also the standard white in REC 709)
Matrix4f CalculateRGBtoXYZConversionMatLuminanceAware( Vector2f chromaRed, Vector2f chromaGreen, Vector2f chromaBlue, float WhitePointLuminance = 100.0f, Vector2f chromaWhite = /*D65*/ Vector2f( 0.31270f, 0.32900f ) )
{
    //First with the additional assumption of white.Y == 100, calculate the XYZ coords of white // TODO check
    Vector3f chromaWhite_xyz = Vector3f( chromaWhite.x, chromaWhite.y, 1.0f - chromaWhite.x - chromaWhite.y );
    Vector3f whiteXYZ = chromaWhite_xyz * WhitePointLuminance / chromaWhite_xyz.y;

    //Having the proper white in XYZ coords we can proceed to calculate the remaining XYZ coords for color primaries
    Vector3f primaryX = Vector3f( chromaRed.x / chromaRed.y, chromaGreen.x / chromaGreen.y, chromaBlue.x / chromaBlue.y );
    Vector3f primaryY = Vector3f( 1, 1, 1 );
    Vector3f primaryZ = Vector3f(
        ( 1 - chromaRed.x - chromaRed.y ) / chromaRed.y,
        ( 1 - chromaGreen.x - chromaGreen.y ) / chromaGreen.y,
        ( 1 - chromaBlue.x - chromaBlue.y ) / chromaBlue.y );

    Matrix4f M( Vector4f( primaryX, 0.f ), Vector4f( primaryY, 0.f ), Vector4f( primaryZ, 0.f ), Vector4f( 0.f, 0.f, 0.f, 1.f ) );
    Math::Transpose(M); // we use row vectors in the engine Matrix ( mul defined as v * M, instead of M * v) 
    Matrix4f invM = Math::Inverse(M);

    Vector4f S = invM * Vector4f(whiteXYZ, 1.0f);
    S /= S.w;
    
    const Matrix4f RGBtoXYZ = ( Math::MakeHomogeneousScale(S.xyz()) * M );

    return RGBtoXYZ;
}