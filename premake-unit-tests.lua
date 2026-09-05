group "Unit Tests"
    for i, name in ipairs(UnitTestProjects) do
        project (name)
            language "C++"
            kind "ConsoleApp"
            
            defines {
                "RESOURCES_GLOBAL=\"" .. path.join(gb_SamplesDir, "MiniEngine", "Resources") .. "\"",
                "SHADERS_GLOBAL=\"" .. path.join(gb_SamplesDir, "MiniEngine", "Shaders") .. "\"",
                "RESOURCES_PROJECT=\"" .. path.join(gb_SamplesDir, name, "Resources") .. "\"",
                "SHADERS_PROJECT=\"" .. path.join(gb_SamplesDir, name, "Shaders") .. "\"",
                "TEMP_DIR=\"" .. path.join(gb_TempDir, name) .. "\"",
                "TEMP_BAKED=\"" .. path.join(gb_TempDir, "Baked") .. "\"",
                "TEMP_BAKED_SCENES=\"" .. path.join(gb_TempDir,  "Baked", "Scenes") .. "\"",
            }
        
            if gbUseSampleScenes then
                defines {
                    "RESOURCES_SAMPLE_SCENES=\"" .. path.join(gb_SamplesDir, "Scenes") .. "\"",
                }
            end
            
            -- Solution file
            location (path.join(gb_SolutionProjectDir, "Unit Tests"))
    
            -- Project includes
            includedirs {
                gb_IncludeDir,
                path.join(gb_UnitTestsDir, name),
                path.join(gb_SourceDependencyDir, "ctti", "include"),
                path.join(gb_SourceDependencyDir, "imgui"),
                path.join(gb_CompiledDependencyDir, "glew-2.3.1", "include"),
                path.join(gb_SourceDependencyDir, "Catch2", "src"),
                path.join(gb_IntermediatesDir, "generated", "Catch2")
            }
    
            -- Project files
            files {
                path.join(gb_UnitTestsDir, name, "**.h"),
                path.join(gb_UnitTestsDir, name, "**.hpp"),
                path.join(gb_UnitTestsDir, name, "**.c"),
                path.join(gb_UnitTestsDir, name, "**.cpp"),
                path.join(gb_UnitTestsDir, name, "**.glsl"),
                
                path.join(gb_SourceDependencyDir, "Catch2", "src", "catch2", "internal", "catch_main.cpp")
            }
        
            -- Window specific 
            if gbWindowAPI== "glfw" then
                defines ("WINDOW_GLFW")
                filter { "system:windows" }
                    includedirs (path.join(gb_CompiledDependencyDir, "GLFW", "include"))
                filter {}
            end
            if gbWindowAPI== "sdl2" then
                defines ("WINDOW_SDL2")
                includedirs (path.join(gb_CompiledDependencyDir, "SDL2", "include"))
            end
            if gbWindowAPI== "sdl3" then
                defines ("WINDOW_SDL3")
                includedirs (path.join(gb_CompiledDependencyDir, "SDL3", "include"))
            end

                    links {
            "Camera",
            "Shared",
            "Files",
            "Image",
            "Importers",
            "Math",
            "MathSimt",
            "Memory",
            "Modeling",
            "RayTracing",
            "Rendering",
            "TinyGLTF3",
            "ImGUI",
            "Catch2"
        }

        dependson {
            "Camera",
            "Shared",
            "Files",
            "Image",
            "Importers",
            "Math",
            "MathSimt",
            "Memory",
            "Modeling",
            "RayTracing",
            "Rendering",
            "TinyGLTF3",
            "ImGUI",
            "Catch2"
        }
        
            if gbUseSpirV then
                links { "shaderc_combined" }
            end
            
            filter {"system:linux", "action:gmake"}
                -- OpenGL, GLFW and GLEW includes are provided by the system
                links { "GLEW", "GL" }
            
                links {
                    "stb_image",
                    "stb_image_write",
                }
    
                if gbUseShaderc then
                    links { "shaderc_combined" }
                end
    
                -- Window specific 
                if gbWindowAPI== "glfw" then
                    -- OpenGL, GLFW and GLEW includes are provided by the system -- TODO
                    links { "glfw" }
    
                end
                if gbWindowAPI== "sdl2" then
                    -- OpenGL, GLFW and GLEW includes are provided by the system
                    -- todo links { "glfw", "GLEW", "GL" }
                        
                end
                if gbWindowAPI== "sdl3" then
                    -- OpenGL, GLFW and GLEW includes are provided by the system
                    -- todo links { "glfw", "GLEW", "GL" }
                end
            filter { "" }
    end