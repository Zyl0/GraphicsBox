#include "ImageOps.h"

#include "ColorSpaces.h"
#include "Image.h"
#include "Math/Functons.h"
#include "Memory/Functions.h"
#include "Shared/Assertion.h"

#include "_TexelOps.h"

void GenerateMips(const Image& image)
{
    switch (image.ComponentLayout())
    {
    case Image::R:
        switch (image.ComponentType())
        {
        case Image::UnsignedByte:
            {
                ImageBuffer<uint8_t> buffer = {image};
                GenerateMips(buffer);
            }
            break;
            
        case Image::Byte:
            {
                ImageBuffer<int8_t> buffer = {image};
                GenerateMips(buffer);
            }
            break;
            
        case Image::UnsignedShort:
            {
                ImageBuffer<uint16_t> buffer = {image};
                GenerateMips(buffer);
            }
            break;
            
        case Image::Short:
            {
                ImageBuffer<int16_t> buffer = {image};
                GenerateMips(buffer);
            }
            break;
            
        case Image::UnsignedInt:
            {
                ImageBuffer<uint32_t> buffer = {image};
                GenerateMips(buffer);
            }
            break;
            
        case Image::Int:
            {
                ImageBuffer<int32_t> buffer = {image};
                GenerateMips(buffer);
            }
            break;
            
        case Image::Float:
            {
                ImageBuffer<float> buffer = {image};
                GenerateMips(buffer);
            }
            break;
            
        case Image::Double:
            {
                ImageBuffer<double> buffer = {image};
                GenerateMips(buffer);
            }
            break;
        }
        break;
        
    case Image::RG:
        switch (image.ComponentType())
        {
        case Image::UnsignedByte:
            {
                ImageBuffer<Math::Vector2t<uint8_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::Byte:
            {
                ImageBuffer<Math::Vector2t<int8_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::UnsignedShort:
            {
                ImageBuffer<Math::Vector2t<uint16_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::Short:
            {
                ImageBuffer<Math::Vector2t<int16_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::UnsignedInt:
            {
                ImageBuffer<Math::Vector2t<uint32_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::Int:
            {
                ImageBuffer<Math::Vector2t<int32_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::Float:
            {
                ImageBuffer<Math::Vector2t<float>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::Double:
            {
                ImageBuffer<Math::Vector2t<double>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
        }
        break;
        
    case Image::RGB:
    case Image::BGR:
        switch (image.ComponentType())
        {
        case Image::UnsignedByte:
            {
                ImageBuffer<Math::Vector3t<uint8_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::Byte:
            {
                ImageBuffer<Math::Vector3t<int8_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::UnsignedShort:
            {
                ImageBuffer<Math::Vector3t<uint16_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::Short:
            {
                ImageBuffer<Math::Vector3t<int16_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::UnsignedInt:
            {
                ImageBuffer<Math::Vector3t<uint32_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::Int:
            {
                ImageBuffer<Math::Vector3t<int32_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::Float:
            {
                ImageBuffer<Math::Vector3t<float>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::Double:
            {
                ImageBuffer<Math::Vector3t<double>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
        }
        break;
        
    case Image::RGBA:
    case Image::ARGB:
    case Image::ABGR:
        switch (image.ComponentType())
        {
        case Image::UnsignedByte:
            {
                ImageBuffer<Math::Vector4t<uint8_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::Byte:
            {
                ImageBuffer<Math::Vector4t<int8_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::UnsignedShort:
            {
                ImageBuffer<Math::Vector4t<uint16_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::Short:
            {
                ImageBuffer<Math::Vector4t<int16_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::UnsignedInt:
            {
                ImageBuffer<Math::Vector4t<uint32_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::Int:
            {
                ImageBuffer<Math::Vector4t<int32_t>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::Float:
            {
                ImageBuffer<Math::Vector4t<float>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
            
        case Image::Double:
            {
                ImageBuffer<Math::Vector4t<double>> buffer = {image};
                GenerateMips(buffer);
            }
        break;
        }
        break;
    }
}

Math::Vector4f SampleImage(const Image& image, const ImageSampler& sampler, Math::Vector2f uvs, uint32_t mip)
{
    Math::Vector2t<uint32_t> size = image.MipSize(mip);
    
    Math::Vector4f sample{};
    uvs.x = _Image::ApplyWarping(uvs.x, sampler.WarpU);
    uvs.y = _Image::ApplyWarping(uvs.y, sampler.WarpV);
    
    uint32_t x, y;
    switch (sampler.BaseFilter)
    {
    case ImageSampler::F_Nearest:
        x = std::min(size.x - 1, static_cast<uint32_t>(std::round(uvs.x * static_cast<float>(size.x))));
        y = std::min(size.y - 1, static_cast<uint32_t>(std::round(uvs.y * static_cast<float>(size.y))));
        switch (image.ComponentLayout())
        {
        case Image::R:
            switch (image.ComponentType())
            {
            case Image::UnsignedByte:
                {
                    ImageBuffer<uint8_t> buffer = {image};
                    sample.x = ReadBuffer(buffer, x, y, mip);
                }
                break;
                
            case Image::Byte:
                {
                    ImageBuffer<int8_t> buffer = {image};
                    sample.x = ReadBuffer(buffer, x, y, mip);
                }
                break;
                
            case Image::UnsignedShort:
                {
                    ImageBuffer<uint16_t> buffer = {image};
                    sample.x = ReadBuffer(buffer, x, y, mip);
                }
                break;
                
            case Image::Short:
                {
                    ImageBuffer<int16_t> buffer = {image};
                    sample.x = ReadBuffer(buffer, x, y, mip);
                }
                break;
                
            case Image::UnsignedInt:
                {
                    ImageBuffer<uint32_t> buffer = {image};
                    sample.x = ReadBuffer(buffer, x, y, mip);
                }
                break;
                
            case Image::Int:
                {
                    ImageBuffer<int32_t> buffer = {image};
                    sample.x = ReadBuffer(buffer, x, y, mip);
                }
                break;
                
            case Image::Float:
                {
                    ImageBuffer<float> buffer = {image};
                    sample.x = ReadBuffer(buffer, x, y, mip);
                }
                break;
                
            case Image::Double:
                {
                    ImageBuffer<double> buffer = {image};
                    sample.x = ReadBuffer(buffer, x, y, mip);
                }
                break;
                
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported image type")
            }
            break;
            
        case Image::RG:
            switch (image.ComponentType())
            {
            case Image::UnsignedByte:
                {
                    ImageBuffer<Math::Vector2t<uint8_t>> buffer = {image};
                    Math::Vector2f s = ReadBuffer(buffer, x, y, mip);
                    sample.x = s.x;
                    sample.y = s.y;
                }
            break;
                
            case Image::Byte:
                {
                    ImageBuffer<Math::Vector2t<int8_t>> buffer = {image};
                    Math::Vector2f s = ReadBuffer(buffer, x, y, mip);
                    sample.x = s.x;
                    sample.y = s.y;
                }
            break;
                
            case Image::UnsignedShort:
                {
                    ImageBuffer<Math::Vector2t<uint16_t>> buffer = {image};
                    Math::Vector2f s = ReadBuffer(buffer, x, y, mip);
                    sample.x = s.x;
                    sample.y = s.y;
                }
            break;
                
            case Image::Short:
                {
                    ImageBuffer<Math::Vector2t<int16_t>> buffer = {image};
                    Math::Vector2f s = ReadBuffer(buffer, x, y, mip);
                    sample.x = s.x;
                    sample.y = s.y;
                }
            break;
                
            case Image::UnsignedInt:
                {
                    ImageBuffer<Math::Vector2t<uint32_t>> buffer = {image};
                    Math::Vector2f s = ReadBuffer(buffer, x, y, mip);
                    sample.x = s.x;
                    sample.y = s.y;
                }
            break;
                
            case Image::Int:
                {
                    ImageBuffer<Math::Vector2t<int32_t>> buffer = {image};
                    Math::Vector2f s = ReadBuffer(buffer, x, y, mip);
                    sample.x = s.x;
                    sample.y = s.y;
                }
            break;
                
            case Image::Float:
                {
                    ImageBuffer<Math::Vector2t<float>> buffer = {image};
                    Math::Vector2f s = ReadBuffer(buffer, x, y, mip);
                    sample.x = s.x;
                    sample.y = s.y;
                }
            break;
                
            case Image::Double:
                {
                    ImageBuffer<Math::Vector2t<double>> buffer = {image};
                    Math::Vector2f s = ReadBuffer(buffer, x, y, mip);
                    sample.x = s.x;
                    sample.y = s.y;
                }
            break;
                
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported image type")
            }
            break;
            
        case Image::RGB:
        case Image::BGR:
            switch (image.ComponentType())
            {
            case Image::UnsignedByte:
                {
                    ImageBuffer<Math::Vector3t<uint8_t>> buffer = {image};
                    sample.xyz() = ReadBuffer(buffer, x, y, mip);
                }
            break;
                
            case Image::Byte:
                {
                    ImageBuffer<Math::Vector3t<int8_t>> buffer = {image};
                    sample.xyz() = ReadBuffer(buffer, x, y, mip);
                }
            break;
                
            case Image::UnsignedShort:
                {
                    ImageBuffer<Math::Vector3t<uint16_t>> buffer = {image};
                    sample.xyz() = ReadBuffer(buffer, x, y, mip);
                }
            break;
                
            case Image::Short:
                {
                    ImageBuffer<Math::Vector3t<int16_t>> buffer = {image};
                    sample.xyz() = ReadBuffer(buffer, x, y, mip);
                }
            break;
                
            case Image::UnsignedInt:
                {
                    ImageBuffer<Math::Vector3t<uint32_t>> buffer = {image};
                    sample.xyz() = ReadBuffer(buffer, x, y, mip);
                }
            break;
                
            case Image::Int:
                {
                    ImageBuffer<Math::Vector3t<int32_t>> buffer = {image};
                    sample.xyz() = ReadBuffer(buffer, x, y, mip);
                }
            break;
                
            case Image::Float:
                {
                    ImageBuffer<Math::Vector3t<float>> buffer = {image};
                    sample.xyz() = ReadBuffer(buffer, x, y, mip);
                }
            break;
                
            case Image::Double:
                {
                    ImageBuffer<Math::Vector3t<double>> buffer = {image};
                    sample.xyz() = ReadBuffer(buffer, x, y, mip);
                }
            break;
                
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported image type")
            }
            break;
            
        case Image::RGBA:
        case Image::ARGB:
        case Image::ABGR:
            switch (image.ComponentType())
            {
            case Image::UnsignedByte:
                {
                    ImageBuffer<Math::Vector4t<uint8_t>> buffer = {image};
                    sample = ReadBuffer(buffer, x, y, mip);
                }
            break;
                
            case Image::Byte:
                {
                    ImageBuffer<Math::Vector4t<int8_t>> buffer = {image};
                    sample = ReadBuffer(buffer, x, y, mip);
                }
            break;
                
            case Image::UnsignedShort:
                {
                    ImageBuffer<Math::Vector4t<uint16_t>> buffer = {image};
                    sample = ReadBuffer(buffer, x, y, mip);
                }
            break;
                
            case Image::Short:
                {
                    ImageBuffer<Math::Vector4t<int16_t>> buffer = {image};
                    sample = ReadBuffer(buffer, x, y, mip);
                }
            break;
                
            case Image::UnsignedInt:
                {
                    ImageBuffer<Math::Vector4t<uint32_t>> buffer = {image};
                    sample = ReadBuffer(buffer, x, y, mip);
                }
            break;
                
            case Image::Int:
                {
                    ImageBuffer<Math::Vector4t<int32_t>> buffer = {image};
                    sample = ReadBuffer(buffer, x, y, mip);
                }
            break;
                
            case Image::Float:
                {
                    ImageBuffer<Math::Vector4t<float>> buffer = {image};
                    sample = ReadBuffer(buffer, x, y, mip);
                }
            break;
                
            case Image::Double:
                {
                    ImageBuffer<Math::Vector4t<double>> buffer = {image};
                    sample = ReadBuffer(buffer, x, y, mip);
                }
            break;
                
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported image type")
            }
            break;
        }

        break;
    case ImageSampler::F_Cubic:
    case ImageSampler::F_Linear:
        {
            float lin_x = std::min(uvs.x * static_cast<float>(size.x), (float)(size.x - 1));
            float lin_y = std::min(uvs.y * static_cast<float>(size.y), (float)(size.y - 1));
        
            float x0 = std::floor(lin_x);
            float y0 = std::floor(lin_y);
            float x1 = std::min(x0 + 1, (float)(size.x - 1));
            float y1 = std::min(y0 + 1, (float)(size.y - 1));
            
            float x_w = (sampler.BaseFilter == ImageSampler::F_Cubic) ? Math::SmoothStep(lin_x - x0) : lin_x - x0;
            float y_w = (sampler.BaseFilter == ImageSampler::F_Cubic) ? Math::SmoothStep(lin_y - y0) : lin_y - y0;
        
            switch (image.ComponentLayout())
            {
            case Image::R:
                {
                    float sampleA{}, sampleB{};
                    switch (image.ComponentType())
                    {
                    case Image::UnsignedByte:
                        {
                            ImageBuffer<uint8_t> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample.x = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample.x = Math::LinearInterpolate(sample.x, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::Byte:
                        {
                            ImageBuffer<int8_t> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample.x = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample.x = Math::LinearInterpolate(sample.x, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::UnsignedShort:
                        {
                            ImageBuffer<uint16_t> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample.x = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample.x = Math::LinearInterpolate(sample.x, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::Short:
                        {
                            ImageBuffer<int16_t> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample.x = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample.x = Math::LinearInterpolate(sample.x, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::UnsignedInt:
                        {
                            ImageBuffer<uint32_t> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);;
                            sample.x = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);;
                            sample.x = Math::LinearInterpolate(sample.x, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::Int:
                        {
                            ImageBuffer<int32_t> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample.x = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample.x = Math::LinearInterpolate(sample.x, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::Float:
                        {
                            ImageBuffer<float> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample.x = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample.x = Math::LinearInterpolate(sample.x, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::Double:
                        {
                            ImageBuffer<double> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample.x = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample.x = Math::LinearInterpolate(sample.x, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                        
                    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported image type")
                    }
                }break;
            
            case Image::RG:
                {
                    Math::Vector2f sampleA, sampleB, sampleC;
                    switch (image.ComponentType())
                    {
                    case Image::UnsignedByte:
                        {
                            ImageBuffer<Math::Vector2t<uint8_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sampleC = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sampleC = Math::LinearInterpolate(sampleC, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                            sample.x = sampleC.x;
                            sample.y = sampleC.y;
                        }
                    break;
                
                    case Image::Byte:
                        {
                            ImageBuffer<Math::Vector2t<int8_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sampleC = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sampleC = Math::LinearInterpolate(sampleC, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                            sample.x = sampleC.x;
                            sample.y = sampleC.y;
                        }
                    break;
                
                    case Image::UnsignedShort:
                        {
                            ImageBuffer<Math::Vector2t<uint16_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sampleC = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sampleC = Math::LinearInterpolate(sampleC, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                            sample.x = sampleC.x;
                            sample.y = sampleC.y;
                        }
                    break;
                
                    case Image::Short:
                        {
                            ImageBuffer<Math::Vector2t<int16_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sampleC = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sampleC = Math::LinearInterpolate(sampleC, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                            sample.x = sampleC.x;
                            sample.y = sampleC.y;
                        }
                    break;
                
                    case Image::UnsignedInt:
                        {
                            ImageBuffer<Math::Vector2t<uint32_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sampleC = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sampleC = Math::LinearInterpolate(sampleC, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                            sample.x = sampleC.x;
                            sample.y = sampleC.y;
                        }
                    break;
                
                    case Image::Int:
                        {
                            ImageBuffer<Math::Vector2t<int32_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sampleC = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sampleC = Math::LinearInterpolate(sampleC, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                            sample.x = sampleC.x;
                            sample.y = sampleC.y;
                        }
                    break;
                
                    case Image::Float:
                        {
                            ImageBuffer<Math::Vector2t<float>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sampleC = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sampleC = Math::LinearInterpolate(sampleC, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                            sample.x = sampleC.x;
                            sample.y = sampleC.y;
                        }
                    break;
                
                    case Image::Double:
                        {
                            ImageBuffer<Math::Vector2t<double>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sampleC = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sampleC = Math::LinearInterpolate(sampleC, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                            sample.x = sampleC.x;
                            sample.y = sampleC.y;
                        }
                    break;
                        
                    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported image type")
                    }
                }break;
            
            case Image::RGB:
            case Image::BGR:
                {
                    Math::Vector3f sampleA, sampleB;
                    switch (image.ComponentType())
                    {
                    case Image::UnsignedByte:
                        {
                            ImageBuffer<Math::Vector3t<uint8_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample.xyz() = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample.xyz() = Math::LinearInterpolate(sample.xyz(), Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::Byte:
                        {
                            ImageBuffer<Math::Vector3t<int8_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample.xyz() = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample.xyz() = Math::LinearInterpolate(sample.xyz(), Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::UnsignedShort:
                        {
                            ImageBuffer<Math::Vector3t<uint16_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample.xyz() = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample.xyz() = Math::LinearInterpolate(sample.xyz(), Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::Short:
                        {
                            ImageBuffer<Math::Vector3t<int16_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample.xyz() = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample.xyz() = Math::LinearInterpolate(sample.xyz(), Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::UnsignedInt:
                        {
                            ImageBuffer<Math::Vector3t<uint32_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample.xyz() = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample.xyz() = Math::LinearInterpolate(sample.xyz(), Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::Int:
                        {
                            ImageBuffer<Math::Vector3t<int32_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample.xyz() = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample.xyz() = Math::LinearInterpolate(sample.xyz(), Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::Float:
                        {
                            ImageBuffer<Math::Vector3t<float>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample.xyz() = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample.xyz() = Math::LinearInterpolate(sample.xyz(), Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::Double:
                        {
                            ImageBuffer<Math::Vector3t<double>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample.xyz() = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample.xyz() = Math::LinearInterpolate(sample.xyz(), Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                        
                    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported image type")
                    }
                }break;
            
            case Image::RGBA:
            case Image::ARGB:
            case Image::ABGR:
                {
                    Math::Vector4f sampleA, sampleB;
                    switch (image.ComponentType())
                    {
                    case Image::UnsignedByte:
                        {
                            ImageBuffer<Math::Vector4t<uint8_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample = Math::LinearInterpolate(sample, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::Byte:
                        {
                            ImageBuffer<Math::Vector4t<int8_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample = Math::LinearInterpolate(sample, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::UnsignedShort:
                        {
                            ImageBuffer<Math::Vector4t<uint16_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample = Math::LinearInterpolate(sample, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::Short:
                        {
                            ImageBuffer<Math::Vector4t<int16_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample = Math::LinearInterpolate(sample, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::UnsignedInt:
                        {
                            ImageBuffer<Math::Vector4t<uint32_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample = Math::LinearInterpolate(sample, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::Int:
                        {
                            ImageBuffer<Math::Vector4t<int32_t>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample = Math::LinearInterpolate(sample, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::Float:
                        {
                            ImageBuffer<Math::Vector4t<float>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample = Math::LinearInterpolate(sample, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                
                    case Image::Double:
                        {
                            ImageBuffer<Math::Vector4t<double>> buffer = {image};
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y0, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y0, mip);
                            sample = Math::LinearInterpolate(sampleA, sampleB, x_w);
                        
                            sampleA = ReadBuffer(buffer, (uint32_t)x0, (uint32_t)y1, mip);
                            sampleB = ReadBuffer(buffer, (uint32_t)x1, (uint32_t)y1, mip);
                            sample = Math::LinearInterpolate(sample, Math::LinearInterpolate(sampleA, sampleB, x_w), y_w);
                        }
                    break;
                        
                    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported image type")
                    }
                }break;
            }
        }
        break;
    }
    
    return sample;
}

Math::Vector4f SampleImage(const Image& image, const SurfaceSampler& surface, const ImageSampler& sampler, Math::Vector2f uvs)
{
    float dUdx = surface.dudx * static_cast<float>(image.Width());
    float dVdx = surface.dvdx * static_cast<float>(image.Height());
    float dUdy = surface.dudy * static_cast<float>(image.Width());
    float dVdy = surface.dvdy * static_cast<float>(image.Height());
    
    float rho_x = std::sqrt(dUdx * dUdx + dVdx * dVdx);
    float rho_y = std::sqrt(dUdy * dUdy + dVdy * dVdy);
    float rho = std::max(rho_x, rho_y);
    float lambda = (rho == 0.0f) ? -1000.0f : std::log2(rho);
    
    if (lambda <= 0.0f)
    {
        ImageSampler sampler2 = sampler;
        sampler2.BaseFilter = sampler.Magnification;
        return SampleImage(image, sampler2, uvs);
    }
    else
    {
        ImageSampler sampler2 = sampler;
        sampler2.BaseFilter = sampler.Minification;
        
        lambda = Math::Clamp(lambda, 0.0f, static_cast<float>(image.MipCount() - 1));
        
        uint32_t level0 = static_cast<int>(std::floor(lambda));
        uint32_t level1 = std::min(level0 + 1u, image.MipCount() - 1);
        
        float l0_w = lambda - static_cast<float>(level0), l1_w = 1 - lambda;
        
        switch (sampler.MipMode)
        {
        case ImageSampler::F_Nearest:
            return SampleImage(image, sampler2, uvs, l0_w < 0.5 ? level0 : level1);
                
        case ImageSampler::F_Linear:
        case ImageSampler::F_Cubic:
            {
                Math::Vector4f a = SampleImage(image, sampler2, uvs, level0), b = SampleImage(image, sampler2, uvs, level1);
                return Math::LinearInterpolate(
                   a,
                   b,
                   sampler.MipMode == ImageSampler::F_Cubic ? Math::SmoothStep(l0_w) : l0_w
                   );
            }
            
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported mip mode")
        }
    }
}
