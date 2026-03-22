-- Parameters
-- C/C++ configuration
-- Windows/MSVC
gb_msvc_cpp_version =      "c++20"
gb_msvc_c_version =        "c11"
-- Linux/GCC or CLang
gb_gnuc_cpp_version =      "gnu++20"
gb_gnuc_c_version =        "gnu11"

-- Pathes
gb_SolutionDir =               path.getabsolute(".")
gb_DependencyDir =             path.join(gb_SolutionDir, "Dependencies")
    gb_CompiledDependencyDir = path.join(gb_DependencyDir, "Libs")
    gb_SourceDependencyDir =   path.join(gb_DependencyDir, "LibSources")
gb_IntermediatesDir =          path.join(gb_SolutionDir, "Intermediates")
gb_OutputDir =                 path.join(gb_SolutionDir, "Binaries")
gb_IncludeDir =                path.join(gb_SolutionDir, "Include")
gb_SrcDir =                    path.join(gb_SolutionDir, "Source")
    gb_LibsImplementDir =      path.join(gb_SrcDir, "Libs")
gb_SamplesDir =                path.join(gb_SolutionDir, "Samples")
gb_SolutionProjectDir =        path.join(gb_SolutionDir, "Solution")

-- Windowing default API, todo setup differently
newoption {
   trigger = "window",
   value = "API",
   description = "Windowing system used",
   allowed = {
      { "glfw",  "GLFW" },
      { "sdl2",  "Simple DIrectMedia Library 2" },
      { "sdl3",  "Simple DIrectMedia Library 3" }
   },
   default = "glfw"
}

-- Sample projects
newoption {
   trigger = "samples",
   description = "Generate sample projects. Download sample assets. Etc"
}

solution "GraphicsBox"
    configurations { "Debug", "Development", "Release" }
    debugdir ( gb_SolutionDir )
    objdir ( path.join(gb_IntermediatesDir, "objects") )
    
    -- Solution file
    location (gb_SolutionDir)

    -- Build Configurations setup
    filter "configurations:Debug"
        targetdir (path.join(gb_OutputDir, "Debug"))
        libdirs (path.join(gb_OutputDir, "Debug"))
        debugdir ( path.join(gb_OutputDir, "Debug") )
        symbols "on"
        defines ("CONFIG_DEBUG")
    filter "configurations:Development"
        targetdir (path.join(gb_OutputDir, "Development"))
        libdirs (path.join(gb_OutputDir, "Development"))
        debugdir ( path.join(gb_OutputDir, "Development") )
        optimize "speed"
        symbols "on"
        defines ("CONFIG_DEVELOPMENT")
    filter "configurations:Release"
        targetdir (path.join(gb_OutputDir, "Release"))
        libdirs (path.join(gb_OutputDir, "Release"))
        debugdir ( path.join(gb_OutputDir, "Release") )
        optimize "full"
        defines ("CONFIG_RELEASE")
    filter {}

    -- CPU Architecture
    architecture "x86_64"
    
    flags { "NoPCH" }

     -- Platforms specific setup
    filter "system:linux"
        -- TODO specify CLang version
        toolset "clang-21"
        cppdialect (gb_gnuc_cpp_version)
        cdialect (gb_gnuc_c_version)

        defines ("PLATFORM_LINUX")

        buildoptions { "-mtune=native -march=native" }
        buildoptions { "-W -Wall -Wextra -Wsign-compare -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable", "-pipe" }
        
        filter { "system:linux" , "configurations:Debug" }
            buildoptions { "-g", "-O0" } 
            linkoptions { "-g"}

        filter { "system:linux" , "configurations:Development" }
            buildoptions { "-g", "-O3", "-mavx" } 
            linkoptions { "-g"}
       
        filter { "system:linux" , "configurations:Release" }
            buildoptions { "-fopenmp -O3 -mavx" } -- TODO evaluate -flto optimisation
            linkoptions { "-fopenmp -O3 -mavx" }  -- TODO evaluate -flto optimisation
        
        filter {"system:linux", "action:cmake"}
            staticruntime "On"

    filter "system:windows"
        toolset "msc"

        cppdialect (gb_msvc_cpp_version)
        cdialect (gb_msvc_c_version)

        defines { "WIN32", "_USE_MATH_DEFINES" } --, "_CRT_SECURE_NO_WARNINGS" }
        defines { "NOMINMAX" } -- allow std::min() and std::max() in vc++ :(((

        defines ("PLATFORM_WINDOWS")

        --staticruntime "On"

        buildoptions{ "/Zc:__cplusplus"}
    
        filter { "system:windows" , "configurations:Debug" }
            runtime "Debug"

        filter { "system:windows" , "configurations:Development" }
            runtime "Debug"
            buildoptions { "/O2 /Oi /arch:AVX2" }

        filter { "system:windows" , "configurations:Release" }
            runtime "Release"
            buildoptions { "/O2 /Oi /arch:AVX2 /openmp:experimental" }
            -- ADD to not generate debugging symbols

        filter { "system:windows" , "action:vs*" }
            system "Windows"
            architecture "x64"
            disablewarnings { "4244", "4305" }
            flags { "MultiProcessorCompile", "NoMinimalRebuild" }

group "Dependencies"
    project "stb_image"
        language "C++"
        kind "StaticLib"
        
        -- Solution file
        location (path.join(gb_SolutionProjectDir, "Dependencies"))
        
        -- Project includes
        includedirs (path.join(gb_SourceDependencyDir, "stb"))

        -- Project files
        files {
            path.join(gb_LibsImplementDir, "stb_image.cpp")
        }

    project "stb_image_write"
        language "C++"
        kind "StaticLib"
        
        -- Solution file
        location (path.join(gb_SolutionProjectDir, "Dependencies"))
        
        -- Project includes
        includedirs (path.join(gb_SourceDependencyDir, "stb"))

        -- Project files
        files {
            path.join(gb_LibsImplementDir, "stb_image_write.cpp")
        }
    
    project "CTTI"
        language "C++"
        kind "StaticLib"
        
        -- Solution file
        location (path.join(gb_SolutionProjectDir, "Dependencies"))
        
        -- Project includes
        includedirs {
            path.join(gb_SourceDependencyDir, "ctti", "include")
        }

        -- Project files    
        files {
            path.join(gb_SourceDependencyDir, "ctti", "include", "**.hpp")
        }
    
    project "TinyGLTF"
        language "C++"
        kind "StaticLib"
        
        -- Solution file
        location (path.join(gb_SolutionProjectDir, "Dependencies"))
        
        -- Project includes
        includedirs {
            path.join(gb_SourceDependencyDir, "stb"),
            path.join(gb_SourceDependencyDir, "TinyGLTF")
        }

        -- Project files    
        files { 
            path.join(gb_SourceDependencyDir, "TinyGLTF", "*.cpp"),
            path.join(gb_SourceDependencyDir, "TinyGLTF", "*.c"), 
            path.join(gb_SourceDependencyDir, "TinyGLTF", "*.h"),
            path.join(gb_SourceDependencyDir, "TinyGLTF", "*.hpp"),
            path.join(gb_LibsImplementDir, "tiny_gltf.cpp")
        }
    
        dependson {"stb_image"}

    project "ImGUI"
        language "C++"
        kind "StaticLib"
        
        -- Solution file
        location (path.join(gb_SolutionProjectDir, "Dependencies"))
        
        project_dir = path.join(gb_SourceDependencyDir, "imgui")
        
        -- Project includes
        includedirs (project_dir)

        -- Project files
        files {
            path.join(project_dir, "*.cpp"),
            path.join(project_dir, "*.c"), 
            path.join(project_dir, "*.h"), 

            path.join(project_dir, "backends", "imgui_impl_opengl3.h"),
            path.join(project_dir, "backends", "imgui_impl_opengl3.cpp")
        }

        -- Window specific code
        filter { "options:window=glfw" }
            includedirs (path.join(gb_CompiledDependencyDir, "GLFW", "include"))
            files {
                path.join(project_dir, "backends", "imgui_impl_glfw.h"),
                path.join(project_dir, "backends", "imgui_impl_glfw.cpp")
            }
        filter { "options:window=sdl2" }
            includedirs (path.join(gb_CompiledDependencyDir, "SDL2", "include"))
            files {
                path.join(project_dir, "backends", "imgui_impl_sdl2.h"),
                path.join(project_dir, "backends", "imgui_impl_sdl2.cpp")
            }
        filter { "options:window=sdl3" }
            includedirs (path.join(gb_CompiledDependencyDir, "SDL3", "include"))
            files {
                path.join(project_dir, "backends", "imgui_impl_sdl3.h"),
                path.join(project_dir, "backends", "imgui_impl_sdl3.cpp")
            }
        filter { "" }
    
        defines { 
            "GLEW_STATIC"
        }
group "Utilites"
    project "Shared"
        language "C++"
        kind "StaticLib"
        
        -- Solution file
        location (path.join(gb_SolutionProjectDir, "Utilites"))

        -- Project includes
        includedirs {
            gb_IncludeDir,
            path.join(gb_IncludeDir, "Shared")
        }

        -- Project files
        files {
            path.join(gb_IncludeDir, "Shared", "**.h"),
            path.join(gb_IncludeDir, "Shared", "**.hpp"),
            path.join(gb_SrcDir, "Shared", "**.h"),
            path.join(gb_SrcDir, "Shared", "**.hpp"),
            path.join(gb_SrcDir, "Shared", "**.c"),
            path.join(gb_SrcDir, "Shared", "**.cpp"),
        }

    project "Math"
        language "C++"
        kind "StaticLib"
        
        -- Solution file
        location (path.join(gb_SolutionProjectDir, "Utilites"))

        -- Project includes
        includedirs {
            gb_IncludeDir,
            path.join(gb_IncludeDir, "Math")
        }

        -- Project files
        files {
            path.join(gb_IncludeDir, "Math", "**.h"),
            path.join(gb_IncludeDir, "Math", "**.hpp"),
            path.join(gb_SrcDir, "Math", "**.h"),
            path.join(gb_SrcDir, "Math", "**.hpp"),
            path.join(gb_SrcDir, "Math", "**.c"),
            path.join(gb_SrcDir, "Math", "**.cpp"),
        }

    project "Modeling"
        language "C++"
        kind "StaticLib"
        
        -- Solution file
        location (path.join(gb_SolutionProjectDir, "Utilites"))

        -- Project includes
        includedirs {            
            gb_IncludeDir,
            path.join(gb_IncludeDir, "Math")
        }

        -- Project files
        files {
            path.join(gb_IncludeDir, "Modeling", "**.h"),
            path.join(gb_IncludeDir, "Modeling", "**.hpp"),
            path.join(gb_SrcDir, "Modeling", "**.h"),
            path.join(gb_SrcDir, "Modeling", "**.hpp"),
            path.join(gb_SrcDir, "Modeling", "**.c"),
            path.join(gb_SrcDir, "Modeling", "**.cpp"),
        }

    project "Image"
        language "C++"
        kind "StaticLib"
        
        -- Solution file
        location (path.join(gb_SolutionProjectDir, "Utilites"))

        -- Project includes
        includedirs {
            gb_IncludeDir,
            path.join(gb_SourceDependencyDir, "stb"),
            path.join(gb_IncludeDir, "Image")
        }

        -- Project files
        files {
            path.join(gb_IncludeDir, "Image", "**.h"),
            path.join(gb_IncludeDir, "Image", "**.hpp"),
            path.join(gb_SrcDir, "Image", "**.h"),
            path.join(gb_SrcDir, "Image", "**.hpp"),
            path.join(gb_SrcDir, "Image", "**.c"),
            path.join(gb_SrcDir, "Image", "**.cpp"),
        }

        -- Dependencies
        dependson {
            "stb_image",
            "stb_image_write",
        }
        links {
            "stb_image",
            "stb_image_write"
        }
    
    project "Importers"
        language "C++"
        kind "StaticLib"
        
        -- Solution file
        location (path.join(gb_SolutionProjectDir, "Utilites"))

        -- Project includes
        includedirs {
            gb_IncludeDir,
            path.join(gb_SourceDependencyDir, "TinyGLTF"),
            path.join(gb_IncludeDir, "Importers"),
            path.join(gb_CompiledDependencyDir, "glew-2.3.1", "include"),
        }

        -- Project files
        files {
            path.join(gb_IncludeDir, "Importers", "**.h"),
            path.join(gb_IncludeDir, "Importers", "**.hpp"),
            path.join(gb_SrcDir, "Importers", "**.h"),
            path.join(gb_SrcDir, "Importers", "**.hpp"),
            path.join(gb_SrcDir, "Importers", "**.c"),
            path.join(gb_SrcDir, "Importers", "**.cpp"),
        }

        -- Dependencies
        dependson {
            "TinyGLTF",
        }
        links {
            "TinyGLTF",
        }

    project "Files"
        language "C++"
        kind "StaticLib"
        
        -- Solution file
        location (path.join(gb_SolutionProjectDir, "Utilites"))

        -- Project includes
        includedirs {
            gb_IncludeDir,
            path.join(gb_IncludeDir, "Files")
        }

        -- Project files
        files {
            path.join(gb_IncludeDir, "Files", "**.h"),
            path.join(gb_IncludeDir, "Files", "**.hpp"),
            path.join(gb_SrcDir, "Files", "**.h"),
            path.join(gb_SrcDir, "Files", "**.hpp"),
            path.join(gb_SrcDir, "Files", "**.c"),
            path.join(gb_SrcDir, "Files", "**.cpp"),
        }

    project "Memory"
        language "C++"
        kind "StaticLib"
        
        -- Solution file
        location (path.join(gb_SolutionProjectDir, "Utilites"))

        -- Project includes
        includedirs {
            gb_IncludeDir,
            path.join(gb_IncludeDir, "Memory")
        }

        -- Project files
        files {
            path.join(gb_IncludeDir, "Memory", "**.h"),
            path.join(gb_IncludeDir, "Memory", "**.hpp"),
            path.join(gb_SrcDir, "Memory", "**.h"),
            path.join(gb_SrcDir, "Memory", "**.hpp"),
            path.join(gb_SrcDir, "Memory", "**.c"),
            path.join(gb_SrcDir, "Memory", "**.cpp"),
        }
    
    project "Camera"
        language "C++"
        kind "StaticLib"
        
        -- Solution file
        location (path.join(gb_SolutionProjectDir, "Utilites"))

        -- Project includes
        includedirs {
            gb_IncludeDir,
            path.join(gb_IncludeDir, "Camera")
        }

        -- Project files
        files {
            path.join(gb_IncludeDir, "Camera", "**.h"),
            path.join(gb_IncludeDir, "Camera", "**.hpp"),
            path.join(gb_SrcDir, "Camera", "**.h"),
            path.join(gb_SrcDir, "Camera", "**.hpp"),
            path.join(gb_SrcDir, "Camera", "**.c"),
            path.join(gb_SrcDir, "Camera", "**.cpp"),
        }

    project "Rendering"
        language "C++"
        kind "StaticLib"
        
        -- Solution file
        location (path.join(gb_SolutionProjectDir, "Utilites"))

        -- Project includes
        includedirs {
            gb_IncludeDir,
            path.join(gb_CompiledDependencyDir, "glew-2.3.1", "include"),
            path.join(gb_IncludeDir, "Rendering")
        }

        -- Project files
        files {
            path.join(gb_IncludeDir, "Rendering", "**.h"),
            path.join(gb_IncludeDir, "Rendering", "**.hpp"),
            path.join(gb_SrcDir, "Rendering", "**.h"),
            path.join(gb_SrcDir, "Rendering", "**.hpp"),
            path.join(gb_SrcDir, "Rendering", "**.c"),
            path.join(gb_SrcDir, "Rendering", "**.cpp"),
        }

        
        filter {"system:linux"}
            -- OpenGL, GLFW and GLEW includes are provided by the system
            links { "GLEW", "GL" }

        filter {"system:windows"}
            -- Link directories for GLFW and GLEW libraries
            libdirs ( path.join(gb_CompiledDependencyDir, "glew-2.3.1", "lib", "Release", "x64") )

            cmd_dll_glew32 = path.join(gb_CompiledDependencyDir, "glew-2.3.1", "bin", "Release", "x64", "glew32.dll")

            -- Copy GLEW dll to bin path
            postbuildcommands {
                "{COPYFILE} %{cmd_dll_glew32} %{cfg.targetdir}"
            }

            -- Linking GLFW and GLEW libraries
            links { "opengl32", "glew32" }

if _OPTIONS["samples"] then
group "Samples"    
    project "MiniEngine"
        language "C++"
        kind "StaticLib"
        
        -- Solution file
        location (path.join(gb_SolutionProjectDir, "samples"))

        -- Project includes
        includedirs {
            gb_IncludeDir,
            path.join(gb_SamplesDir, "MiniEngine", "Include"),
            path.join(gb_SourceDependencyDir, "TinyGLTF"),
            path.join(gb_SourceDependencyDir, "imgui"),
            path.join(gb_SourceDependencyDir, "ctti", "include"),
            path.join(gb_CompiledDependencyDir, "glew-2.3.1", "include"),
        }

        -- Project files
        files {
            path.join(gb_SamplesDir, "MiniEngine", "Include", "**.h"),
            path.join(gb_SamplesDir, "MiniEngine", "Include", "**.hpp"),
            path.join(gb_SamplesDir, "MiniEngine", "Source", "**.h"),
            path.join(gb_SamplesDir, "MiniEngine", "Source", "**.hpp"),
            path.join(gb_SamplesDir, "MiniEngine", "Source", "**.c"),
            path.join(gb_SamplesDir, "MiniEngine", "Source", "**.cpp"),

            path.join(gb_SamplesDir, "MiniEngine", "Shaders", "**.glsl"),
        }

        links {
            "Camera",
            "Shared",
            "Files",
            "Image",
            "Importers",
            "Math",
            "Memory",
            "Modeling",
            "Rendering",
            "TinyGLTF",
            "ImGUI"
        }

        dependson {
            "Camera",
            "Shared",
            "Files",
            "Image",
            "Importers",
            "Math",
            "Memory",
            "Modeling",
            "Rendering",
            "TinyGLTF",
            "ImGUI"
        }

        -- Window specific 
        if _OPTIONS["window"] == "glfw" then
            defines ("WINDOW_GLFW")
            includedirs (path.join(gb_CompiledDependencyDir, "GLFW", "include"))
        end
        if _OPTIONS["window"] == "sdl2" then
            defines ("WINDOW_SDL2")
            includedirs (path.join(gb_CompiledDependencyDir, "SDL2", "include"))
        end
        if _OPTIONS["window"] == "sdl3" then
            defines ("WINDOW_SDL3")
            includedirs (path.join(gb_CompiledDependencyDir, "SDL3", "include"))
        end

        -- Window specific 
        if _OPTIONS["window"] == "glfw" then
            filter {"system:linux"}
                -- OpenGL, GLFW and GLEW includes are provided by the system -- TODO
                links { "glfw" }

            filter {"options:window=glfw", "system:windows"}
                -- Link directories for GLFW and GLEW libraries
                libdirs (path.join(gb_CompiledDependencyDir, "GLFW", "win64")) 

                -- Linking GLFW and GLEW libraries
                links { "glfw3" }
        end
        if _OPTIONS["window"] == "sdl2" then
            filter {"system:linux"}
                -- OpenGL, GLFW and GLEW includes are provided by the system
                -- todo links { "glfw", "GLEW", "GL" }

            filter {"options:window=sdl2", "system:windows"}
                -- Link directories for GLFW and GLEW libraries
                libdirs ( path.join(gb_CompiledDependencyDir, "SDL2", "win64") )

                cmd_dll_sdl2 = path.join(gb_CompiledDependencyDir, "SDL2", "win64", "SDL2.dll")

                -- Copy GLEW dll to bin path
                postbuildcommands {
                    "{COPYFILE} %{cmd_dll_sdl2} %{cfg.targetdir}"
                }

                -- Linking GLFW and GLEW libraries
                links { "SDL2" }
        end
        if _OPTIONS["window"] == "sdl3" then
            filter {"system:linux"}
                -- OpenGL, GLFW and GLEW includes are provided by the system
                -- todo links { "glfw", "GLEW", "GL" }

            filter {"options:window=sdl3", "system:windows"}
                -- Link directories for GLFW and GLEW libraries
                libdirs ( path.join(gb_CompiledDependencyDir, "SDL3", "win64") )

                cmd_dll_sdl3 = path.join(gb_CompiledDependencyDir, "SDL3", "win64", "SDL3.dll")

                -- Copy GLEW dll to bin path
                postbuildcommands {
                    "{COPYFILE} %{cmd_dll_sdl3} %{cfg.targetdir}"
                }

                -- Linking GLFW and GLEW libraries
                links { "SDL3" }
        end
    filter { "" }

    SampleProjects = {
        "GIMesh",
        "PBR",
        "SpectralRendering"
    }

    for i, name in ipairs(SampleProjects) do
        project (name)
            language "C++"
            kind "ConsoleApp"
            
            -- Solution file
            location (path.join(gb_SolutionProjectDir, "samples"))

            -- Project includes
            includedirs {
                gb_IncludeDir,
                path.join(gb_SamplesDir, "MiniEngine", "Include"),
                path.join(gb_SamplesDir, name),
                path.join(gb_SourceDependencyDir, "imgui"),
                path.join(gb_CompiledDependencyDir, "glew-2.3.1", "include"),
            }

            -- Project files
            files {
                path.join(gb_SamplesDir, name, "**.h"),
                path.join(gb_SamplesDir, name, "**.hpp"),
                path.join(gb_SamplesDir, name, "**.c"),
                path.join(gb_SamplesDir, name, "**.cpp"),

                --path.join(gb_SamplesDir, "MiniEngine", "Shaders", "**.glsl"),
                path.join(gb_SamplesDir, name, "**.glsl"),
            }

            links {
                "MiniEngine"
            }

            dependson {
                "MiniEngine",
            }
    end

end
