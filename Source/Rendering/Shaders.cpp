#include <string>
#include <vector>

#include "Shared/Assertion.h"
#include "Shared/Annotations.h"
#include "Files/Files.h"
#include "Shaders.h"
#include <list>
#include <set>

class StringPtr
{
public:
    StringPtr(const std::string* String, size_t Offset) : m_String(String), m_Offset(Offset)
#ifdef CONFIG_DEBUG
    , m_At(0)
#endif // CONFIG_DEBUG
    {
        AssertOffset();
        
#ifdef CONFIG_DEBUG
        if (String != nullptr)
        {
            m_At = String->data();
            m_At += m_Offset;
        }
#endif // CONFIG_DEBUG
    }

    char operator*() const { return *(At()); }

    StringPtr& operator ++ () { AssertOffset(+1); Move(1); return *this; }
    StringPtr& operator -- () { AssertOffset(-1); Move(-1); return *this; }

    StringPtr& operator += (size_t offset) { AssertOffset(offset); Move(offset); return *this; }
    StringPtr& operator -= (size_t negativeOffset) { AssertOffset(-static_cast<ptrdiff_t>(negativeOffset)); Move(-static_cast<ptrdiff_t>(negativeOffset)); return *this; }

    StringPtr operator + (size_t offset) const { StringPtr tmp = *this; tmp += offset; return tmp; }
    StringPtr operator - (size_t negativeOffset) const { StringPtr tmp = *this; tmp -= negativeOffset; return tmp; }

    StringPtr& operator = (const StringPtr& Other)
    {
        if (this == &Other) return *this;

        m_String = Other.m_String;
        m_Offset = Other.m_Offset;
        
#ifdef CONFIG_DEBUG
        m_At = Other.m_At;
#endif // CONFIG_DEBUG

        AssertOffset();

        return *this;
    }

    bool operator == (const StringPtr& other) const { return m_String == other.m_String && m_Offset == other.m_Offset; }
    bool operator != (const StringPtr& other) const { return m_String != other.m_String && m_Offset != other.m_Offset; }

    bool operator >= (const StringPtr& other) const { AssertOrError(m_String == other.m_String, "To be comparable, StringPtr must share the same memory"); return m_Offset >= other.m_Offset; }
    bool operator <= (const StringPtr& other) const { AssertOrError(m_String == other.m_String, "To be comparable, StringPtr must share the same memory"); return m_Offset <= other.m_Offset; }
    bool operator >  (const StringPtr& other) const { AssertOrError(m_String == other.m_String, "To be comparable, StringPtr must share the same memory"); return m_Offset > other.m_Offset; }
    bool operator <  (const StringPtr& other) const { AssertOrError(m_String == other.m_String, "To be comparable, StringPtr must share the same memory"); return m_Offset < other.m_Offset; }

    static StringPtr Begin(const std::string* String)
    {
        if (String == nullptr) return StringPtr(nullptr, 0);

        return StringPtr(String, 0);
    }

    static StringPtr End(const std::string* String)
    {
        if (String == nullptr) return StringPtr(nullptr, 0);

        return StringPtr(String, String->size());
    }

    size_t Offset() const { return m_Offset; }

    const char* At() const { return ((const char*)m_String->data()) + m_Offset; }
    char* At() { return ((char*)m_String->data()) + m_Offset; }

    bool CanDisplace(ptrdiff_t Displacement) const
    {
        if (m_String == nullptr) return false;

        if (Displacement > 0 && (m_Offset + Displacement > m_String->size())) return false;
        if (Displacement < 0 && m_Offset < static_cast<uint64_t>(abs(Displacement))) return false;

        return true;
    }

    void Refresh()
    {
#ifdef CONFIG_DEBUG
        m_At = At();
#endif // CONFIG_DEBUG
    }
private:
    void AssertOffset(ptrdiff_t Displacement = 0) const
    {
        if (m_String == nullptr)
        {
            AssertOrError(m_Offset == 0 && Displacement == 0, "Offset or displacement over a null string pointer")
        }
        AssertOrError(Displacement < 0 || m_Offset + Displacement <= m_String->size(), "Displacement addition is out of bounds (up)")
        AssertOrError(Displacement > 0 ||  m_Offset >= static_cast<uint64_t>(abs(Displacement)), "Displacement subtraction is out of range (down)")
        AssertOrError(m_Offset <= m_String->size(), "String ptr offset is out of bounds")
    }

    void Move(ptrdiff_t Displacement)
    {
        m_Offset += Displacement;
#ifdef CONFIG_DEBUG
        m_At += Displacement;
#endif // CONFIG_DEBUG
    }

    const std::string* m_String;
    size_t m_Offset = 0;

#ifdef CONFIG_DEBUG
    const char* m_At;
#endif // CONFIG_DEBUG
};

static std::set<std::filesystem::path> SearchPaths{};

void ShaderAddSearchPath(const std::filesystem::path& path)
{
    SearchPaths.insert(path);
}

// Basic preprocessor ifdef unaware
std::string ShaderFileToString(const std::filesystem::path& filename)
{
    std::string buff = {};
    StringPtr At = StringPtr::Begin(&buff);
    StringPtr End = StringPtr::End(&buff);

    struct IncludeFile
    {
        // For search paths
        std::filesystem::path ParentPath;

        StringPtr EndInBuffer;
    };
    std::list<IncludeFile> FileStack;

    auto Erase = [&buff, &FileStack, &End](StringPtr& At, size_t Count) -> void
    {
        // Update EOFs
        End -= Count; End.Refresh();
        for (auto& fileInStack : FileStack)
        {
            fileInStack.EndInBuffer -= Count;
            fileInStack.EndInBuffer.Refresh();
        }

        At.Refresh();
        buff.erase(At.Offset(), Count);
    };

    auto PushFile = [&buff, &FileStack, &At, &End](const std::filesystem::path& AbsolutePath) -> void
    {
        // Read file
        std::string file = FileToString(AbsolutePath);
        if (file.empty()) return;

        buff.insert(At.Offset(), file);
        At.Refresh();

        // Update EOFs
        End += file.size(); End.Refresh();
        for (auto& fileInStack : FileStack)
        {
            fileInStack.EndInBuffer += file.size();
            fileInStack.EndInBuffer.Refresh();
        }

        // Add file in stack
        FileStack.emplace_back(IncludeFile{ .ParentPath = AbsolutePath.parent_path(), .EndInBuffer = At + file.size() });
    };

    bool HasFound = false;
    for (const auto & search_path : SearchPaths)
    {
        if (exists(search_path / filename ))
        {
            PushFile(search_path / filename);
            HasFound = true;
            break;
        }
    }
    AssertOrErrorCallF(HasFound, return buff, "Could not find main shader file \"%s\"", filename.generic_string().c_str())

    bool SkipLineComment = false;
    bool SkipComment = false;
    bool CommentEndBegin = false;
    bool IsInPreprocessorDirective = false;

    StringPtr PreprocessorDirectiveBegin = StringPtr::Begin(&buff);
    StringPtr PreprocessorDirectiveEnd = StringPtr::End(&buff);

    while (At < End)
    {
        // Handle preprocessor directives
        if (IsInPreprocessorDirective)
        {
            std::filesystem::path Path{};

            // Skip empty spaces
            while (At < End && (*At == ' ' || *At == '\t')) ++At;

            // Check for include keyword
            {
                if (!At.CanDisplace(7 /*characters in 'include'*/)) goto exit_preporcessor;
                StringPtr SampleEnd = At + 7;
                std::string_view IncludeDirective(At.At(), SampleEnd.At());

                if (IncludeDirective != "include") goto exit_preporcessor;

                At += 7;
            }

            // Skip empty spaces
            while (At < End && (*At == ' ' || *At == '\t')) ++At;

            if (*At != '"' && *At != '<') goto exit_preporcessor;
            {
                ++At;
                StringPtr IncludeBegin = At;

                // Skip path
                while (At < End && *At != '\n' && *At != '"' && *At != '>')
                    ++At;

                if (*At != '"' && *At != '>') goto exit_preporcessor;
                StringPtr IncludeEnd = At;

                std::string_view IncludeName = std::string_view(IncludeBegin.At(), IncludeEnd.At());
                if (exists(FileStack.back().ParentPath / IncludeName))
                {
                    Path = FileStack.back().ParentPath / IncludeName;
                }
                else
                {
                    bool HasFound = false;
                    for (const auto & search_path : SearchPaths)
                    {
                        if (exists(search_path / IncludeName))
                        {
                            Path = search_path /IncludeName;
                            HasFound = true;
                            break;
                        }
                    }

                    if (!HasFound)
                    {
                        std::string sample = std::string(IncludeBegin.At(), IncludeEnd.At());
                        EngineLoggerErrorF("Could not find file \"%s\"", sample.c_str());
                        goto exit_preporcessor;
                    }
                }

                // Skip to the end
                while (At < End && *At != '\n') ++At;

                PreprocessorDirectiveEnd = At;
                At = PreprocessorDirectiveBegin;

                Erase(At, PreprocessorDirectiveEnd.Offset() - PreprocessorDirectiveBegin.Offset());

                goto perform_include;
            }

            // Perform include
        perform_include:
            PushFile(Path);

        exit_preporcessor:
            IsInPreprocessorDirective = false;
            goto next;
        }


        // Skip comments
        if (SkipLineComment)
        {
            // Check if comment ended
            if (*At == '\n') SkipLineComment = false;

            goto next;
        }
        if (SkipComment)
        {
            if (!CommentEndBegin && *At == '*') CommentEndBegin = true;
            else if (CommentEndBegin && *At == '/') SkipComment = false;
            else { CommentEndBegin = false; }

            goto next;
        }
        if (*At == '/')
        {
            StringPtr Next = At; ++Next;
            if (Next == End) goto next;

            if (*Next == '/') { SkipLineComment = true; ++At; goto next; }
            if (*Next == '*') { SkipComment = true; ++At; goto next; }
        }

        // Catch preprocessor directives
        if (*At == '#')
        {
            PreprocessorDirectiveBegin = At;
            IsInPreprocessorDirective = true;
        }

        // Increment
    next:
        ++At;

        // Check for EOF in current file
        if (At < End)
        {
            if (At >= FileStack.back().EndInBuffer) FileStack.pop_back();
        }
        continue;
    }

    return buff;
}

Shader ShaderFromFile(Shader::Type type, const std::filesystem::path& filename, Shader::DefinesView Defines)
{
    std::string shaderCode = ShaderFileToString(filename);

    return Shader(type, shaderCode, Defines);
}

static 
const char *GetShaderTypeDefineName( const GLenum type )
{
    switch (type)
    {
    case GL_VERTEX_SHADER: return "VERTEX_SHADER";
    case GL_FRAGMENT_SHADER: return "FRAGMENT_SHADER";
    case GL_GEOMETRY_SHADER: return "GEOMETRY_SHADER";
#ifdef GL_VERSION_4_0
    case GL_TESS_CONTROL_SHADER: return "TESSELATION_CONTROL";
    case GL_TESS_EVALUATION_SHADER: return "EVALUATION_CONTROL";
#endif
#ifdef GL_VERSION_4_3
    case GL_COMPUTE_SHADER: return "COMPUTE_SHADER";
#endif

    case Shader::__Count:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported shader type")
    }
}

static 
const char *GetShaderTypeDefineName( Shader::Type type )
{
    switch (type)
    {
    case Shader::VERTEX_SHADER:
        return "VERTEX_SHADER";
    case Shader::FRAGMENT_SHADER:
        return "FRAGMENT_SHADER";
    case Shader::GEOMETRY_SHADER:
        return "GEOMETRY_SHADER";
        
#ifdef GL_VERSION_4_0
    case Shader::TESSELATION_CONTROL_SHADER:
        return "TESSELATION_CONTROL";
    case Shader::TESSELATION_EVALUATION_SHADER:
        return "EVALUATION_CONTROL";
#endif
        
#ifdef GL_VERSION_4_3
    case Shader::COMPUTE_SHADER:
        return "COMPUTE_SHADER";
#endif

    case Shader::__Count:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported shader type")
    }
}

static 
GLenum GetRawShaderType( Shader::Type type )
{
    switch (type)
    {
    case Shader::VERTEX_SHADER:
        return GL_VERTEX_SHADER;
    case Shader::FRAGMENT_SHADER:
        return GL_FRAGMENT_SHADER;
    case Shader::GEOMETRY_SHADER:
        return GL_GEOMETRY_SHADER;
        
#ifdef GL_VERSION_4_0
    case Shader::TESSELATION_CONTROL_SHADER:
        return GL_TESS_CONTROL_SHADER;
    case Shader::TESSELATION_EVALUATION_SHADER:
        return GL_TESS_EVALUATION_SHADER;
#endif
        
#ifdef GL_VERSION_4_3
    case Shader::COMPUTE_SHADER:
        return GL_COMPUTE_SHADER;
#endif

    case Shader::__Count:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported shader type")
    }
}

Shader::Shader(Type type, std::string_view SourceCode, Shader::DefinesView Defines)
{
    const size_t ShaderVersionBegin = SourceCode.find("#version");
    AssertOrError(ShaderVersionBegin < std::string::npos, "Impossible to compile shader. No shader version specified.")
    std::string_view shaderCode = SourceCode.substr(ShaderVersionBegin, SourceCode.size() - ShaderVersionBegin);

    const size_t ShaderCodeBegin = shaderCode.find('\n');
    AssertOrError(ShaderCodeBegin < std::string::npos, "Impossible to compile shader. No shader source code found.")
    const std::string_view version = SourceCode.substr(ShaderVersionBegin, ShaderCodeBegin + 1);
    
    shaderCode = {shaderCode.begin() + ShaderCodeBegin + 1, shaderCode.end()};
    AssertOrError(!(shaderCode.find("#version") < std::string::npos), "Impossible to compile shader. Multiple shader version defined.")
    
    //Assemble shader source file.
    std::string shaderFinalSource;
    shaderFinalSource.append(version);
    shaderFinalSource.append(std::string("#define ").append(GetShaderTypeDefineName(type)).append("\n"));
    for (const auto& Define : Defines)
    {
        if (Define.second.empty())
        {
            shaderFinalSource.append(std::string("#define ").append(Define.first).append("\n"));
        }
    }
    shaderFinalSource.append(shaderCode);
    
    GLCall(m_Shader = glCreateShader(GetRawShaderType(type)))
    const char *sources = shaderFinalSource.data();
    const GLsizei size = static_cast<GLsizei>(shaderFinalSource.size());
    GLCall(glShaderSource(m_Shader, 1, &sources, &size))
    GLCall(glCompileShader(m_Shader))

    // Handles compilation error
    GLint value;
    GLCall(glGetShaderiv(m_Shader, GL_COMPILE_STATUS, &value))
    if(value == GL_FALSE)
    {
        // Gathering shader compiler errors
        glGetShaderiv(m_Shader, GL_INFO_LOG_LENGTH, &value);
        std::vector<char>log(value+1, 0);
        glGetShaderInfoLog(m_Shader, static_cast<GLsizei>(log.size()), nullptr, &log.front());

        // Gathering source code
        glGetShaderiv(m_Shader, GL_SHADER_SOURCE_LENGTH, &value);
        std::vector<char> source(value+1, 0);
        glGetShaderSource(m_Shader, static_cast<GLsizei>(source.size()), nullptr, &source.front());

        glGetShaderiv(m_Shader, GL_SHADER_TYPE, &value);
        EngineLoggerErrorF("Failed to compile shader of type %s.", GetShaderTypeDefineName(value));

        std::vector<char> errorLogBuffer;
        unsigned int line = 0, srcIndex = 0;
        for(unsigned int i= 0; log[i] != '\0'; i++)
        {
            unsigned int string_id= 0, line_id= 0, position= 0;
#ifdef PLATFORM_WINDOWS
            if(sscanf_s(&log[i], "%u ( %u ) : %n", &string_id, &line_id, &position) == 2  // nvidia syntax
            || sscanf_s(&log[i], "%u : %u (%*u) : %n", &string_id, &line_id, &position) == 2  // mesa syntax
            || sscanf_s(&log[i], "ERROR : %u : %u : %n", &string_id, &line_id, &position) == 2  // ati syntax
            || sscanf_s(&log[i], "WARNING : %u : %u : %n", &string_id, &line_id, &position) == 2)  // ati syntax
#else // PLATFORM_WINDOWS
            if(sscanf(&log[i], "%u ( %u ) : %n", &string_id, &line_id, &position) == 2  // nvidia syntax
            || sscanf(&log[i], "%u : %u (%*u) : %n", &string_id, &line_id, &position) == 2  // mesa syntax
            || sscanf(&log[i], "ERROR : %u : %u : %n", &string_id, &line_id, &position) == 2  // ati syntax
            || sscanf(&log[i], "WARNING : %u : %u : %n", &string_id, &line_id, &position) == 2)  // ati syntax
#endif // !PLATFORM_WINDOWS
            {
                if(line_id > line)
                for(; source[srcIndex] != '\0'; srcIndex++)
                {
                    errorLogBuffer.push_back(source[srcIndex]);
                    if(source[srcIndex] == '\n')
                        line++;

                    if(line_id == line)
                    {
                        srcIndex++;
                        break;
                    }
                }

                errorLogBuffer.push_back('-');
                errorLogBuffer.push_back('>');
                for(; log[i] != '\n' && log[i] != '\0'; i++)
                {
                    errorLogBuffer.push_back(log[i]);
                }
                errorLogBuffer.push_back('\n');
            }
        }
        for(; source[srcIndex] != '\0'; srcIndex++)
        {
            errorLogBuffer.push_back(source[srcIndex]);
            if(source[srcIndex] == '\n')
                line++;
        }
        errorLogBuffer.push_back('\0');
        printf("%s\n", errorLogBuffer.data());
    }
    else
    {
        EngineLoggerLogF("Compiled shader of type %s successfully", GetShaderTypeDefineName(type));
    }

}

Shader::~Shader()
{
    if (m_Shader == 0) return;

    glDeleteShader(m_Shader);
}

bool Shader::IsComplete() const
{
    if (m_Shader == 0) return false;

    GLint value;
    glGetShaderiv(m_Shader, GL_COMPILE_STATUS, &value);
    if (value == GL_FALSE)
        return false;

    return true;
}
