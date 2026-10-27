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
constexpr uint32_t kGamutInputDownscaleScale = 16u;
constexpr float kMinLuminance = 0.0001f;
constexpr float kMaxLuminance = 10000.0f;

#define GAMUT_DRAW_COLORED_POINTS
// #define GAMUT_DOWN_SAMPLE_AS_AVERAGE
#define LUMINANCE_LOGARITHMIC_HDR

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
        m_SrcEncoding = Image::Linear;

        std::filesystem::path SrcAbsImage;
        AssertOrError(GetAbsoluteFilePath(SrcImage, SrcAbsImage), "No such file or directory")
        m_WorkingImage.emplace(ImageLoad(SrcAbsImage, Image::Float));        
        m_LuminanceGraphHeatMap.resize(kLuminanceGraphSizeX * kLuminanceGraphSizeY);
        m_GamutX.clear();
        m_GamutY.clear();
        m_GamutX.reserve((m_WorkingImage->Width() + kGamutInputDownscaleScale - 1) * (m_WorkingImage->Height() + kGamutInputDownscaleScale - 1) / kGamutInputDownscaleScale);
        m_GamutY.reserve((m_WorkingImage->Width() + kGamutInputDownscaleScale - 1) * (m_WorkingImage->Height() + kGamutInputDownscaleScale - 1) / kGamutInputDownscaleScale);
        
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

            Matrix3f RGBToXYZ, XYZToRec709RGB;

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
                    RGBToXYZ = ArriLogC::CSArriLogCToCSXYZMat;
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
                    sample.xyz() = XYZToRec709RGB * (RGBToXYZ * sample.xyz());
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
        m_DecodedLuminance = DecodeLuminance;
        m_DecodedPaperWhiteLuminance = DecodedPaperwhiteLuminance;
        
        // Luminance map
        {
            const float kMinLogLuminance = std::log10(kMinLuminance);
            const float kMaxLogLuminance = std::log10(kMaxLuminance);
            Matrix3f RGBToXYZ;
            
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
                RGBToXYZ = ArriLogC::CSArriLogCToCSXYZMat;
                break;
                
            case ColorState::CS_Count:
                SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported gamut")
                }

            ImageBuffer<Vector4f> WorkingBuffer(m_WorkingImage.value());
            uint32_t width = m_WorkingImage->Width(), height = m_WorkingImage->Height();
            
            // dbug

            double sumLum = 0.0;
            float minLumObserved = FLT_MAX;
            float maxLumObserved = 0.0f;

            size_t count = 0;

            for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
            {
                Vector4f sample = WorkingBuffer.Read(x, y);
                const float lum = RGB::RGBToLuminance(sample.xyz(), RGBToXYZ);

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
            
            std::fill(m_LuminanceGraphHeatMap.begin(), m_LuminanceGraphHeatMap.end(), 0.0f);
            for (uint32_t y = 0; y < height; y++)
            for (uint32_t x = 0; x < width; x++)
            {
                float luma;
                Vector4f sample = WorkingBuffer.Read(x, y);
                // sample.x *= DecodeLuminance;
                // sample.y *= DecodeLuminance;
                // sample.z *= DecodeLuminance;
                luma = RGB::RGBToLuminance(sample.xyz(), RGBToXYZ);
                
                                
                // ITU-R BT.709 luminance coefficients
                luma = sample.x * 0.2126f + sample.y * 0.7152f + sample.z * 0.0722f;
                
#ifdef LUMINANCE_LOGARITHMIC_HDR
                uint32_t bx = x * kLuminanceGraphSizeX / width;
                
                bx = std::clamp(bx, 0u, kLuminanceGraphSizeX - 1u);
                
                float t_log = std::log10(std::clamp(luma, kMinLuminance, kMaxLuminance));
                float t = (t_log - kMinLogLuminance) / (kMaxLogLuminance - kMinLogLuminance);
                
                // Invert Y so low luminance is at the bottom (row sizeY - 1) and peak is at the top (row 0)
                uint32_t by = static_cast<uint32_t>((1.0f - t) * (kLuminanceGraphSizeY - 1u));
                by = std::clamp(by, 0u, kLuminanceGraphSizeY - 1u);
                
                m_LuminanceGraphHeatMap[by * kLuminanceGraphSizeX + bx] += 1.0f;
#else // LUMINANCE_LOGARITHMIC_HDR
                // Horizontal bin (Image X)
                uint32_t bx = x * kLuminanceGraphSizeX / width;
                bx = std::clamp(bx, 0u, kLuminanceGraphSizeX - 1u);
                
                // Linear normalized luminance in [0, 1]
                float t = std::clamp(luma / kMaxLuminance, 0.0f, 1.0f);
                
                // Invert Y so that luma = 0 is at the bottom (row sizeY - 1) 
                // and luma = 250 is at the top (row 0)
                uint32_t by = static_cast<uint32_t>((1.0f - t) * (kLuminanceGraphSizeY - 1u));
                by = std::clamp(by, 0u, kLuminanceGraphSizeY - 1u);
                
                m_LuminanceGraphHeatMap[by * kLuminanceGraphSizeX + bx] += 1.0f;
#endif // !LUMINANCE_LOGARITHMIC_HDR
            }
        }

        // Gamut
        {
            Matrix3f RGBToXYZ;
            
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
                RGBToXYZ = ArriLogC::CSArriLogCToCSXYZMat;
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
#ifdef GAMUT_DOWN_SAMPLE_AS_AVERAGE
                for (uint32_t cy = 0; cy < kGamutInputDownscaleScale; cy++)
                for (uint32_t cx = 0; cx < kGamutInputDownscaleScale; cx++)
                {
                    uint32_t sx = x + cx;
                    uint32_t sy = y + cy;
                    if (sx < width && sy < height)
                        sample += Vector4f(WorkingBuffer.Read(sx, sy).xyz(), 1.f);
                }
#else // GAMUT_DOWN_SAMPLE_AS_AVERAGE
                sample += Vector4f(WorkingBuffer.Read(std::min(x + kGamutInputDownscaleScale / 2,  width - 1), std::min(y + kGamutInputDownscaleScale / 2, height - 1)).xyz(), 1.f);
#endif // !GAMUT_DOWN_SAMPLE_AS_AVERAGE

                sample /= sample.w;

                Vector3f XYZ = RGBToXYZ * (sample.xyz() / DecodedPaperwhiteLuminance);
                float sum = XYZ.x + XYZ.y + XYZ.z;

                if (sum > 1e-6f)
                {
                    m_GamutX.push_back(XYZ.x / sum);
                    m_GamutY.push_back(XYZ.y / sum);
                }
                else
                {
                    m_GamutX.push_back( 0.f );
                    m_GamutY.push_back( 0.f );
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
#ifdef LUMINANCE_LOGARITHMIC_HDR
            const double minLog = std::log10(0.01);   // -2
            const double maxLog = std::log10(10000.0); // +4
            
            ImPlot::SetupAxesLimits(0, m_WorkingImage->Width(), minLog, maxLog, ImGuiCond_Always);
            
            // Custom tick labels displaying actual nits: 10^y
            static const double tickValues[] = { -2.0, -1.0, 0.0, 1.0, 2.0, 3.0, 4.0 };
            static const char* tickLabels[]  = { "0.01", "0.1", "1", "10", "100", "1000", "10000" };
            ImPlot::SetupAxisTicks(ImAxis_Y1, tickValues, 7, tickLabels);
            ImPlot::PlotHeatmap("Pixel density", m_LuminanceGraphHeatMap.data(),
                kLuminanceGraphSizeY, kLuminanceGraphSizeX,
                0.0, 10.0,
                nullptr,
                ImPlotPoint(0, minLog),                          // bottom-left
                ImPlotPoint(m_WorkingImage->Width(), maxLog)     // top-right
            );
            
            const float x_bounds[] = { 0.0f, static_cast<float>(m_WorkingImage->Width()) };
            
            const float y_sdr_min[] = { std::log10(0.02f), std::log10(0.02f) };
            ImPlot::PlotLine("SDR Min (0 nits)", x_bounds, y_sdr_min, 2);
            
            const float y_sdr_max[] = { std::log10(250.0f), std::log10(250.0f) };
            ImPlot::PlotLine("SDR Max (250 nits)", x_bounds, y_sdr_max, 2);
            
            const float y_paperwhite[] = { std::log10(m_DecodedPaperWhiteLuminance), std::log10(m_DecodedPaperWhiteLuminance) };
            ImPlot::PlotLine("Paper White", x_bounds, y_sdr_max, 2);
#else // LUMINANCE_LOGARITHMIC_HDR
            ImPlot::SetupAxes("Image X", "Luminance (nits)");
            ImPlot::SetupAxesLimits(0, m_WorkingImage->Width(), 0.0, m_DecodedPaperWhiteLuminance, ImGuiCond_Always);
            ImPlot::PlotHeatmap("Pixel density", m_LuminanceGraphHeatMap.data(),
                kLuminanceGraphSizeY, kLuminanceGraphSizeX,
                0.0, 10.0,  // Adjust colormap scale / density tuning as needed
                nullptr,
                ImPlotPoint(0, 0.0),                                             // bounds_min (bottom-left)
                ImPlotPoint(m_WorkingImage->Width(), m_DecodedPaperWhiteLuminance) // bounds_max (top-right)
            );
#endif // !LUMINANCE_LOGARITHMIC_HDR
            
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

                Vector3f xyz = {
                    m_GamutX[i] / m_GamutY[i],
                    1.f,
                    (1.0f - m_GamutX[i] - m_GamutY[i]) / m_GamutY[i]
                };
                Vector3f rgb = Rec2020::FromXYZ() * xyz;
                rgb.x = std::clamp(rgb.x, 0.f, 1.f);
                rgb.y = std::clamp(rgb.y, 0.f, 1.f);
                rgb.z = std::clamp(rgb.z, 0.f, 1.f);
                
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
    
    float m_DecodedLuminance;
    float m_DecodedPaperWhiteLuminance;
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
