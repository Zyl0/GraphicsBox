#pragma once
#include <cstdint>

struct ImageSampler
{
    enum Filter : uint8_t
    {
        F_Nearest,
        F_Linear,
        F_Cubic
    };

    enum WarpMode : uint8_t
    {
        WM_Repeat,
        WM_MirrorRepeat,
        WM_ClampToEdge,
        WM_ClampToBorder
    };
        
    Filter BaseFilter =     F_Nearest;
    Filter Magnification =  F_Linear;
    Filter Minification =   F_Linear;
    Filter MipMode =        F_Linear;
        
    WarpMode WarpU = WM_Repeat;
    WarpMode WarpV = WM_Repeat;
    WarpMode WarpW = WM_Repeat;
};
