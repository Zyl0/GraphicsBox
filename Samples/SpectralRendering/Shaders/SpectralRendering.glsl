#ifndef INCLUDE_GUARD_SPECTRAL_RENDERING
#define INCLUDE_GUARD_SPECTRAL_RENDERING

// Simple piecewise linear interpolation for CIE 1931 color matching functions
// Data points sampled approximately every 20nm from 380nm to 780nm
const int sampleCount = 21;

const float lambdaSamples[21] = float[]
(
    380.0, 400.0, 420.0, 440.0, 460.0,
    480.0, 500.0, 520.0, 540.0, 560.0,
    580.0, 600.0, 620.0, 640.0, 660.0,
    680.0, 700.0, 720.0, 740.0, 760.0, 780.0
);


// Corresponding x̅(λ), y̅(λ), z̅(λ) values sampled from CIE 1931 2° Standard Observer
const float xBarSamples[21] = float[]
(
    0.0014, 0.0143, 0.1344, 0.3483, 0.2908,
    0.0956, 0.0049, 0.0633, 0.2904, 0.5945,
    0.9163, 1.0622, 0.8544, 0.4177, 0.1752,
    0.0610, 0.0235, 0.0073, 0.0032, 0.0012, 0.0000
);
const float yBarSamples[21] = float[]
(
    0.0000, 0.0004, 0.0040, 0.0230, 0.0900,
    0.2177, 0.4776, 0.7570, 0.9540, 0.9950,
    0.8700, 0.6310, 0.3810, 0.1750, 0.0610,
    0.0170, 0.0041, 0.0010, 0.0002, 0.0000, 0.0000
);
const float zBarSamples[21] = float[]
(
    0.0065, 0.0679, 0.6456, 1.7471, 1.6692,
    1.2876, 0.8130, 0.4652, 0.2720, 0.1582,
    0.0782, 0.0422, 0.0203, 0.0087, 0.0039,
    0.0021, 0.0017, 0.0011, 0.0008, 0.0003, 0.0000
);

// Given a wavelength λ in nanometers, return XYZ color
vec3 WavelengthToXYZ(float wavelength) 
{
    // Clamp wavelength to visible range
    wavelength = clamp(wavelength, 380.0, 780.0);
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
    float X = mix(xBarSamples[index], xBarSamples[index+1], t);
    float Y = mix(yBarSamples[index], yBarSamples[index+1], t);
    float Z = mix(zBarSamples[index], zBarSamples[index+1], t);
    return vec3(X, Y, Z);
}

#endif // INCLUDE_GUARD_SPECTRAL_RENDERING