#include <imgui.h>

#include "App.h"
#include "Core/Spec.h"

// Modules
#include "Modules/Window/Module.h"
#include "Modules/Rendering/Module.h"
#include "Modules/ImGui/Module.h"

#include "Files/Files.h"
#include "Image/ColorSpaces.h"
#include "Rendering/Rendering.h"

#include <implot.h>

#include "Image/ImageOps.h"

using namespace Math;

constexpr uint32_t kLuminanceGraphSizeX = 512;
constexpr uint32_t kLuminanceGraphSizeY = 256;
constexpr uint32_t kGamutInputDownscaleScale = 64u;
constexpr float kMinLuminance = 0.0001f;
constexpr float kMaxLuminance = 10000.0f;

#define GAMUT_DRAW_COLORED_POINTS

class AppModule : public Engine::IModule
{
public:
    AppModule() {}

    ~AppModule() override = default;
    
    void RegisterDependencies(Engine::Spec& spec) override
    {
        spec.Register<Window::Module>();
        spec.Register<Rendering::Module>();
        spec.Register<ImGui::Module>();
    }

    void Initialize() override
    {        
        std::filesystem::path SrcImage = "lenna.png";
        m_SrcGamut = ColorState::CS_Rec709;
        m_SrcEncoding = Image::sRGB;

        std::filesystem::path SrcAbsImage;
        AssertOrError(GetAbsoluteFilePath(SrcImage, SrcAbsImage), "No such file or directory")
        m_WorkingImage.emplace(ImageLoad(SrcAbsImage, Image::Float));        
        m_LuminanceGraphHeatMap.resize(kLuminanceGraphSizeX * kLuminanceGraphSizeY);
        m_GamutX.resize(m_WorkingImage->Width() * m_WorkingImage->Height() / kGamutInputDownscaleScale);
        m_GamutY.resize(m_WorkingImage->Width() * m_WorkingImage->Height() / kGamutInputDownscaleScale);
        
        // Linearize
        float DecodeLuminance = 0.0f;
        float DecodedPaperwhiteLuminance = 0.0f;
        {
            ImageBuffer<Vector4f> WorkingBuffer = (m_WorkingImage.value());
            uint32_t ImageComponentCount = m_WorkingImage->ComponentCount(), width = m_WorkingImage->Width(), height = m_WorkingImage->Height();
            switch (m_SrcEncoding)
            {
            case Image::Linear:
                DecodeLuminance = 250;
                DecodedPaperwhiteLuminance = 250;
                for (uint32_t y = 0; y < height; y++)
                for (uint32_t x = 0; x < width; x++)
                {
                    Vector4f sample = WorkingBuffer.Read(x, y);
                    sample.x *= DecodeLuminance;
                    sample.y *= DecodeLuminance;
                    sample.z *= DecodeLuminance;
                    WorkingBuffer.Write(x, y, sample);
                }
                break;
                
            case Image::Unencoded:
                DecodeLuminance = 1;
                DecodedPaperwhiteLuminance = 1;
                break;
            
            case Image::sRGB:
                DecodeLuminance = 250;
                DecodedPaperwhiteLuminance = 250;
                for (uint32_t y = 0; y < height; y++)
                for (uint32_t x = 0; x < width; x++)
                {
                    Vector4f sample = WorkingBuffer.Read(x, y);
                    sample.x = sRGB::EOTF(sample.x) * DecodeLuminance;
                    sample.y = sRGB::EOTF(sample.y) * DecodeLuminance;
                    sample.z = sRGB::EOTF(sample.z) * DecodeLuminance;
                    WorkingBuffer.Write(x, y, sample);
                }
                break;
            
            case Image::PQ:
                DecodeLuminance = 100000.f;
                DecodedPaperwhiteLuminance = 300.f;
                for (uint32_t y = 0; y < height; y++)
                for (uint32_t x = 0; x < width; x++)
                {
                    Vector4f sample = WorkingBuffer.Read(x, y);
                    sample.x = PQ::EOTF(sample.x) * DecodeLuminance;
                    sample.y = PQ::EOTF(sample.y) * DecodeLuminance;
                    sample.z = PQ::EOTF(sample.z) * DecodeLuminance;
                    WorkingBuffer.Write(x, y, sample);
                }
                break;
            
            case Image::LogC:
                // TODO DecodeLuminance, DecodedPaperwhiteLuminance
                EngineLoggerWarn("Unimplemented peak signal luminance");
                for (uint32_t y = 0; y < height; y++)
                for (uint32_t x = 0; x < width; x++)
                {
                    Vector4f sample = WorkingBuffer.Read(x, y);
                    sample.x = ArriLogC::GArriLogCToGLinear(sample.x) * DecodeLuminance;
                    sample.y = ArriLogC::GArriLogCToGLinear(sample.y) * DecodeLuminance;
                    sample.z = ArriLogC::GArriLogCToGLinear(sample.z) * DecodeLuminance;
                    WorkingBuffer.Write(x, y, sample);
                }
                break;

            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported encoding")
            }
        }

        // Preview image
        {
            Image UploadImage(m_WorkingImage.value());

            ImageBuffer<Vector4f> WorkingBuffer(m_WorkingImage.value());
            ImageBuffer<Vector4f> UploadBuffer(UploadImage);
            uint32_t ImageComponentCount = m_WorkingImage->ComponentCount(), width = m_WorkingImage->Width(), height = m_WorkingImage->Height();

            Matrix4f RGBToXYZ, XYZToRec709RGB;

            if (m_SrcGamut != ColorState::CS_Rec709)
            {
                XYZToRec709RGB = Rec709::FromXYZ();
                
                switch (m_SrcGamut)
                {
                case ColorState::CS_DciP3D65:
                    RGBToXYZ = DciP3D65::ToXYZ();
                    break;
                
                case ColorState::CS_Rec2020:
                    RGBToXYZ = Rec2020::ToXYZ();
                    break;
                
                case ColorState::CS_ArriLogC:
                    RGBToXYZ = ToTransform4(ArriLogC::CSArriLogCToCSXYZMat);
                    break;
                
                case ColorState::CS_Rec709:
                case ColorState::CS_Count:
                SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported gamut")
                }
            }
            
            for (uint32_t y = 0; y < height; y++)
            for (uint32_t x = 0; x < width; x++)
            {
                Vector4f sample = WorkingBuffer.Read(x, y);
                sample.x /= DecodedPaperwhiteLuminance;
                sample.y /= DecodedPaperwhiteLuminance;
                sample.z /= DecodedPaperwhiteLuminance;

                if (m_SrcGamut != ColorState::CS_Rec709)
                {
                    sample = XYZToRec709RGB * (RGBToXYZ * sample);
                }

                // Encode to sRGB
                sample.x = sRGB::OETF(std::clamp(sample.x, 0.f, 1.f));
                sample.y = sRGB::OETF(std::clamp(sample.y, 0.f, 1.f));
                sample.z = sRGB::OETF(std::clamp(sample.z, 0.f, 1.f));
                
                UploadBuffer.Write(x, (height - 1) - y, sample);
            }

            m_DisplayTexture.emplace(UploadImage);
            m_DisplayFrameBuffer.emplace(FrameBuffer::Attachment(UploadImage, {}));
        }
        
        // Luminance map
        {            
            const float kMinLogLuminance = std::log10(kMinLuminance);
            const float kMaxLogLuminance = std::log10(kMaxLuminance);

            ImageBuffer<Vector4f> WorkingBuffer(m_WorkingImage.value());
            uint32_t ImageComponentCount = m_WorkingImage->ComponentCount(), width = m_WorkingImage->Width(), height = m_WorkingImage->Height();
            
            // dbug

            double sumLum = 0.0;
            float minLumObserved = FLT_MAX;
            float maxLumObserved = 0.0f;

            size_t count = 0;

            for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
            {
                Vector4f sample = WorkingBuffer.Read(x, y);
                const float lum = RGB::RGBToLuminance(sample.xyz());

                sumLum += lum;
                minLumObserved = std::min(minLumObserved, lum);
                maxLumObserved = std::max(maxLumObserved, lum);
                ++count;
            }

            printf("Luminance: min=%f, avg=%f, max=%f nits\n",
                   minLumObserved,
                   count ? sumLum / count : 0.0,
                   maxLumObserved);
            
            // dbug end
            
            for (uint32_t y = 0; y < height; y++)
            for (uint32_t x = 0; x < width; x++)
            {
                float luma;
                Vector4f sample = WorkingBuffer.Read(x, y);
                // sample.x *= DecodeLuminance;
                // sample.y *= DecodeLuminance;
                // sample.z *= DecodeLuminance;
                luma = RGB::RGBToLuminance(sample.xyz());

                // Horizontal position bin.
                uint32_t bx = x * kLuminanceGraphSizeX / width;
                bx = std::clamp(bx, 0u, kLuminanceGraphSizeX - 1);

                // Logarithmic luminance bin.
                float t_log = std::log10(std::max(luma, kMinLuminance));
                float t = (t_log - kMinLogLuminance) / (kMaxLogLuminance - kMinLogLuminance);

                uint32_t by = static_cast<int>(t * kLuminanceGraphSizeY);
                by = std::clamp(by, 0u, kLuminanceGraphSizeY - 1u);

                m_LuminanceGraphHeatMap[by * kLuminanceGraphSizeX + bx] += 1.0f;
            }
        }

        // Gamut
        {
            Matrix4f RGBToXYZ;
            
            switch (m_SrcGamut)
            {
            case ColorState::CS_Rec709:
                RGBToXYZ = Rec709::ToXYZ();
                break;
                
            case ColorState::CS_DciP3D65:
                RGBToXYZ = DciP3D65::ToXYZ();
                break;
                
            case ColorState::CS_Rec2020:
                RGBToXYZ = Rec2020::ToXYZ();
                break;
                
            case ColorState::CS_ArriLogC:
                RGBToXYZ = ToTransform4(ArriLogC::CSArriLogCToCSXYZMat);
                break;
                
            case ColorState::CS_Count:
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported gamut")
            }
            
            ImageBuffer<Vector4f> WorkingBuffer = (m_WorkingImage.value());
            uint32_t ImageComponentCount = m_WorkingImage->ComponentCount(), width = m_WorkingImage->Width(), height = m_WorkingImage->Height();
            
            for (uint32_t y = 0; y < height; y += kGamutInputDownscaleScale)
            for (uint32_t x = 0; x < width; x += kGamutInputDownscaleScale)
            {
                Vector4f sample{};
                
                // Downscale
                for (uint32_t cy = 0; cy < kGamutInputDownscaleScale; cy++)
                for (uint32_t cx = 0; cx < kGamutInputDownscaleScale; cx++)
                {
                    uint32_t sx = x + cx;
                    uint32_t sy = y + cy;
                    if (sx < width && sy < height)
                        sample += Vector4f(WorkingBuffer.Read(sx, sy).xyz(), 1.f);
                }

                sample /= sample.w;

                Vector3f XYZ = (RGBToXYZ * Vector4f(Vector3f(sample.xyz() / DecodedPaperwhiteLuminance), 1.f)).xyz();
                float sum = XYZ.x + XYZ.y + XYZ.z;

                if (sum > 1e-8f)
                {
                    m_GamutX[y * width / kGamutInputDownscaleScale + x/ kGamutInputDownscaleScale] = XYZ.x / sum;
                    m_GamutY[y * width/ kGamutInputDownscaleScale + x/ kGamutInputDownscaleScale] = XYZ.y / sum;
                }
                else
                {
                    m_GamutX[y * width/ kGamutInputDownscaleScale + x/ kGamutInputDownscaleScale] = 0.f;
                    m_GamutY[y * width/ kGamutInputDownscaleScale + x/ kGamutInputDownscaleScale] = 0.f;
                }
            }
            
        }
    }

    void Tick(double deltaTime) override
    {
        DebugScopeMarker scope("Copy decoded image");
        
        Window::Module* window = Engine::GetModule<Window::Module>(Context());
        AssertOrError(window, "Invalid window")

        uint32_t width, height;
        window->GetFrameBufferSize(width, height);
        
        // Copy to screen
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_DisplayFrameBuffer->Handle());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(
            0, 0, m_WorkingImage->Width(), m_WorkingImage->Height(),
            0, 0, width, height,
            GL_COLOR_BUFFER_BIT,
            GL_LINEAR
        );
    }

    void Shutdown() override
    {
        m_WorkingImage.reset();
        m_DisplayFrameBuffer.reset();
        m_DisplayTexture.reset();
    }

    void EditorUI() override
    {
        ImVec2 size = ImGui::GetWindowSize();
        size.x -= 20.f;
        
        if (ImPlot::BeginPlot("Luminance (nits)", ImVec2(size.x, size.x)))
        {
            ImPlot::SetupAxes("Image X", "Luminance (nits, log scale)");
            
            ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
            ImPlot::PlotHeatmap("Pixel density", m_LuminanceGraphHeatMap.data(),
                kLuminanceGraphSizeY, kLuminanceGraphSizeX,
                0.0, 10.0,  // Tune to your density distribution.
                nullptr,
                ImPlotPoint(0, kMinLuminance),
                ImPlotPoint(m_WorkingImage->Width(), kMaxLuminance)
                );
            
            ImPlot::EndPlot();
        }

        if (ImPlot::BeginPlot("Image Gamut", ImVec2(size.x, size.x)))
            {
            ImPlot::SetupAxes("x", "y");
            ImPlot::SetupAxisLimits(
                ImAxis_X1, 0.0, 0.8, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(
                ImAxis_Y1, 0.0, 0.9, ImGuiCond_Always);

            float sRGB_x[] = {Rec709::Red().x, Rec709::Green().x, Rec709::Blue().x, Rec709::Red().x};
            float sRGB_y[] = {Rec709::Red().y, Rec709::Green().y, Rec709::Blue().y, Rec709::Red().y};
            ImPlot::PlotLine("Rec709", sRGB_x, sRGB_y, 4);

            float DciP3_x[] = {DciP3D65::Red().x, DciP3D65::Green().x, DciP3D65::Blue().x, DciP3D65::Red().x};
            float DciP3_y[] = {DciP3D65::Red().y, DciP3D65::Green().y, DciP3D65::Blue().y, DciP3D65::Red().y};
            ImPlot::PlotLine("Display P3", DciP3_x, DciP3_y, 4);

            float Rec2020_x[] = {Rec2020::Red().x, Rec2020::Green().x, Rec2020::Blue().x, Rec2020::Red().x};
            float Rec2020_y[] = {Rec2020::Red().y, Rec2020::Green().y, Rec2020::Blue().y, Rec2020::Red().y};
            ImPlot::PlotLine("Rec2020", Rec2020_x, Rec2020_y, 4);

#ifdef GAMUT_DRAW_COLORED_POINTS
            ImDrawList* draw_list = ImPlot::GetPlotDrawList();

            for (size_t i = 0; i < m_GamutX.size(); ++i)
            {
                ImVec2 pos = ImPlot::PlotToPixels(m_GamutX[i], m_GamutY[i]);

                Vector4f xyz = {
                    m_GamutX[i] / m_GamutY[i],
                    1.f,
                    (1.0f - m_GamutX[i] - m_GamutY[i]) / m_GamutY[i],
                    1.f
                };
                Vector4f rgb = Rec2020::FromXYZ() * xyz;
                
                ImU32 color = IM_COL32( rgb.x * 255.f, rgb.y * 255.f, rgb.z * 255.f, 255);

                draw_list->AddCircleFilled(pos, 2.0f, color);
            }
#else // GAMUT_DRAW_COLORED_POINTS
            ImPlot::PlotScatter(
                "Image pixels",
                m_GamutX.data(), m_GamutY.data(),
                static_cast<int>(m_GamutX.size()));
#endif // !GAMUT_DRAW_COLORED_POINTS

            ImPlot::EndPlot();
        }
    }
    
private:
    ColorState::EColorSpace m_SrcGamut;
    Image::Encoding m_SrcEncoding;
    std::optional<Image> m_WorkingImage;
    std::optional<Texture2D> m_DisplayTexture;
    std::optional<FrameBuffer> m_DisplayFrameBuffer;
    std::vector<float> m_LuminanceGraphHeatMap;
    std::vector<float> m_GamutX;
    std::vector<float> m_GamutY;
};


int main(int argc, char* argv[])
{
    // Search paths
    AddSearchPath(RESOURCES_GLOBAL);
    AddSearchPath(RESOURCES_PROJECT);
    ShaderAddSearchPath(SHADERS_GLOBAL);
    ShaderAddSearchPath(SHADERS_PROJECT);
    
    Engine::Spec Specification;
    Specification.Register<Window::Module>();
    Specification.Register<Rendering::Module>();
    Specification.Register<ImGui::Module>();
    Specification.Register<ImPlot::Module>();
    Specification.Register<AppModule>();
    
    Engine::App App(std::move(Specification));
    
    App.Run();
}
