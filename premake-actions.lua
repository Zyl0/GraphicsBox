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

-- Sample projects
newoption {
   trigger = "sample-scenes",
   description = "Download sample scene and content for demonstration"
}

newoption {
    trigger = "shaderc",
    description = "Use shaderc to compile glsl shaders to Spir-V shaders. Compiling shaders from source code will require the shaderc compiler that would be downloaded in setup phase"    
}

newoption {
   trigger = "breakpoints",
   description = "Enable assertions throwing breakpoints"
}

newoption {
   trigger = "dependencies-toolset",
   value = "compiler",
   description = "Toolset used to compile dependencies. \"unset\" by default",
   allowed = {
      { "unset",  "Will use vs2022 or clang-21 depending on if host is windows or linux" },
      { "vs2022",  "(Windows) Microsoft Visual Studio 2022" },
      { "vs2026",  "(Windows) Microsoft Visual Studio 2026" },
      { "gcc",  "(Linux) GNU C Compiler. Using platform's default version" },
      { "clang",  "LLVM CLang Compiler. Using platform's default version" },
      { "clang-21",  "GNU C Compiler version 21" },
   },
   default = "unset"
}

local clang_matching_compilers = {
    ["gcc"] = {
        c = "gcc",
        cxx = "g++"
    },
    ["clang"] = {
        c = "clang",
        cxx = "clang++"
    },
    ["clang-21"] = {
        c = "clang-21",
        cxx = "clang++-21"
    },
}

function GetCoreCount()
    if os.host() == "windows" then
        return tonumber(os.getenv("NUMBER_OF_PROCESSORS")) or 1
    elseif os.host() == "linux" then
        local pipe = io.popen("nproc")
        local count = tonumber(pipe:read("*l"))
        pipe:close()
        return count or 1
    end

    return 1
end

function GetDependenciesProjectPath()
    depPath = nil
    
    if (_ACTION == "setup" or _ACTION:startswith("update")) then
        if _OPTIONS["dependencies-toolset"] == "unset" then
            error("[path] ERROR: unsupportd platform. Please specify a valid toolset using -dependencies-toolset=<...>. See help for more details.")
        end
    
        -- if os.host() == "windows" then
        --     if _OPTIONS["unset"] then
        --         depPath = gb_IntermediatesDepsDir
        --     end
        -- elseif os.host() == "linux" then
        --     depPath = gb_IntermediatesDepsDir
        -- else
        --     error("[path] ERROR: unsupportd platform")
        -- end
        
        local res, err = os.mkdir(gb_IntermediatesDepsDir)
        if res == nil then
            error(err)
        end
    
        res, err = os.mkdir(path.join(gb_IntermediatesDepsDir, _OPTIONS["dependencies-toolset"]))
        if res == nil then
            error(err)
        end
    
        depPath = path.join(gb_IntermediatesDepsDir, _OPTIONS["dependencies-toolset"])
        
    -- Project generation
    elseif _ACTION:startswith("vs") then
        depPath = path.join(gb_IntermediatesDepsDir, _ACTION)
    elseif _ACTION:startswith("gmake") then
        depPath = path.join(gb_IntermediatesDepsDir, _gb_linux_toolset)
    elseif _ACTION == "cmake" then
        depPath = path.join(gb_IntermediatesDepsDir, gb_linux_toolset)
    else
        error("[path] ERROR: Unsupported action")
    end

    return depPath
end

function UpdateSampleScenes()
    local resourcesDir = path.join(gb_SamplesDir, "Scenes")
    
    print("[sample-assets] Checking Resources folder...")

    if not os.isdir(resourcesDir) then
        print("[sample-assets] Creating '" .. resourcesDir .. "' directory...")
        os.mkdir(resourcesDir)
    end

    for _, repo in ipairs(SampleScencesRepos) do
        local repoDir = resourcesDir .. "/" .. repo.name

        print("\n[sample-assets] Processing: " .. repo.name)

        if not os.isdir(repoDir .. "/.git") then
            print("[sample-assets] Cloning " .. repo.url .. " into " .. repoDir .. " ...")
            local result = os.execute("git clone " .. repo.url .. " " .. repoDir)
            print(result)
            if result ~= true then
                error("[sample-assets] ERROR: git clone failed for " .. repo.name)
            else
                print("[sample-assets] Clone complete: " .. repo.name)
            end
        else
            print("[sample-assets] Already cloned. Fetching updates for " .. repo.name .. "...")
            local fetch = os.execute("git -C " .. repoDir .. " fetch")
            if fetch ~= true then
                error("[sample-assets] ERROR: git fetch failed for " .. repo.name)
            end

            local pull = os.execute("git -C " .. repoDir .. " pull")
            if pull ~= true then
                error("[sample-assets] ERROR: git pull failed for " .. repo.name)
            end

            print("[sample-assets] Updated: " .. repo.name)
        end
    end

    print("\n[sample-assets] All repositories processed.")
end

function UpdateShaderCompiler()
    local shadercCompilerRepo = "https://github.com/google/shaderc.git"
    local shadercDir = path.join(gb_ToolsDependencyDir, "shaderc")
    
    local valid = true
    
    -- 1 clone/update the repo
    if not os.isdir(shadercDir .. "/.git") then
        print("[shaderc] Cloning " .. shadercCompilerRepo .. " into " .. shadercDir .. " ...")
        local result = os.execute("git clone " .. shadercCompilerRepo .. " " .. shadercDir)
        print(result)
        if result ~= true then
            error("[shaderc] ERROR: git clone failed for " .. shadercCompilerRepo)
            valid = false
        else
            print("[shaderc] Clone complete: " .. shadercCompilerRepo)
        end
    else
        print("[shaderc] Already cloned. Fetching updates for " .. shadercCompilerRepo .. "...")
        local fetch = os.execute("git -C " .. shadercDir .. " fetch")
        if fetch ~= true then
            error("[shaderc] ERROR: git fetch failed for " .. shadercCompilerRepo)
            valid = false
        end

        local pull = os.execute("git -C " .. shadercDir .. " pull")
        if pull ~= true then
            error("[shaderc] ERROR: git pull failed for " .. shadercCompilerRepo)
            valid = false
        end

        print("[shaderc] Updated: " .. shadercCompilerRepo)
    end

    if valid ~= true then
        gbUseShaderc = false
        return
    end

    -- 2 setup the project
    os.chdir(shadercDir)
    print("[shaderc] Setting up project")
    if os.host() == "windows" then
       local getDeps = os.execute("python " .. path.join(path.getabsolute("."), "utils", "git-sync-deps"))
       if getDeps ~= true then
           error("[shaderc] ERROR: Could not get dependencies")
           gbUseShaderc = false
           os.chdir(gb_SolutionDir)
           return
       end
    elseif os.host() == "linux" then
        local getDeps = os.execute(path.join(path.getabsolute("."), "utils", "git-sync-deps"))
        if getDeps ~= true then
            error("[shaderc] ERROR: Could not get dependencies")
            gbUseShaderc = false
            os.chdir(gb_SolutionDir)
            return
        end
    elseif os.host() == "macosx" then
        error("[shaderc] ERROR: unsupportd platform")
        gbUseShaderc = false
        os.chdir(gb_SolutionDir)
        return
    end
    
    -- 3 Compile the project
    print("[shaderc] Building project")
    local targetProjectPath = path.join(GetDependenciesProjectPath(), "shaderc")
    if not os.isdir(targetProjectPath) then
        os.mkdir(targetProjectPath)
    end
    os.chdir(targetProjectPath)
    local Configs = { "Debug", "Development", "Release" }
    local CmakeConfigs = { "Debug", "RelWithDebInfo", "Release" }
    if not os.isdir(gb_OutputDir) then
        os.mkdir(gb_OutputDir)
    end
    for _, cfg in ipairs(Configs) do
        if not os.isdir(path.join(gb_OutputDir, cfg)) then
            os.mkdir(path.join(gb_OutputDir, cfg))
        end
    end
    if os.host() == "windows" then        
        local cmd = ""
        if _OPTIONS["dependencies-toolset"] == "vs2026" then
            cmd = "cmake -G " .. "\"Visual Studio 18 2026\"" .. " -S ".. shadercDir .. " -DSHADERC_ENABLE_SHARED_CRT=ON"
        elseif _OPTIONS["dependencies-toolset"] == "vs2022" then
            cmd = "cmake -G " .. "\"Visual Studio 17 2022\"" .. " -S ".. shadercDir .. " -DSHADERC_ENABLE_SHARED_CRT=ON"
        else
            error("[path] ERROR: Unsupported action")
        end
    
        print("[shaderc] Generatin CMake project >" .. cmd)
        local genCmake = os.execute(cmd)
        if genCmake ~= true then
            error("[shaderc] ERROR: Could not generate cmake project")
            gbUseShaderc = false
            os.chdir(gb_SolutionDir)
            return
        end
    
        for cfgIndex, cfg in ipairs(Configs) do
            cmd =  "cmake --build . --config ".. CmakeConfigs[cfgIndex] .. " -j " .. GetCoreCount() .. " --target shaderc"
            
            print("[shaderc] Building project shaderc for config " .. cfg ..  " >" .. cmd)
            local build = os.execute(cmd)
            if build ~= true then
                error("[shaderc] ERROR: Failed to compile/link project")
                gbUseShaderc = false
                os.chdir(gb_SolutionDir)
                return
            end
        
            cmd =  "cmake --build . --config ".. CmakeConfigs[cfgIndex] .. " -j " .. GetCoreCount() .. " --target shaderc_combined"
        
            print("[shaderc] Building project shaderc_combined for config " .. cfg ..  " >" .. cmd)
            build = os.execute(cmd)
            if build ~= true then
                error("[shaderc] ERROR: Failed to compile/link project")
                gbUseShaderc = false
                os.chdir(gb_SolutionDir)
                return
            end
        
            local src = path.join(targetProjectPath, "libshaderc", CmakeConfigs[cfgIndex], "shaderc.lib")
            local target = path.join(gb_OutputDir, cfg, "shaderc.lib")
            print("[shaderc] Copying lib from " .. src .. " to ".. target)
            res, err = os.copyfile(src, target)
            if res == nil then
                error(err)
            end
        
            local src = path.join(targetProjectPath, "libshaderc", CmakeConfigs[cfgIndex], "shaderc_combined.lib")
            local target = path.join(gb_OutputDir, cfg, "shaderc_combined.lib")
            print("[shaderc] Copying lib from " .. src .. " to ".. target)
            res, err = os.copyfile(src, target)
            if res == nil then
                error(err)
            end
        
            if cfg ~= "Release"  then
                src = path.join(targetProjectPath, "libshaderc", CmakeConfigs[cfgIndex], "shaderc.pdb")
                target = path.join(gb_OutputDir, cfg, "shaderc.pdb")
                print("[shaderc] Copying symbols from " .. src .. " to ".. target)
                res, err = os.copyfile(src, target)
                if res == nil then
                    error(err)
                end
            end
        end
    elseif os.host() == "linux" then
        for cfgIndex, cfg in ipairs(Configs) do
            targetProjectPath = path.join(GetDependenciesProjectPath(), "shaderc", cfg)
            if not os.isdir(targetProjectPath) then
                os.mkdir(targetProjectPath)
            end
            os.chdir(targetProjectPath)

            local cmd = "cmake -G \"Unix Makefiles\""
                .. " -DCMAKE_C_COMPILER=\"" .. clang_matching_compilers[_OPTIONS["dependencies-toolset"]].c .. "\""
                .. " -DCMAKE_CXX_COMPILER=\"" .. clang_matching_compilers[_OPTIONS["dependencies-toolset"]].cxx .. "\""
                .. " -DCMAKE_BUILD_TYPE=" .. CmakeConfigs[cfgIndex] .. " " .. shadercDir
            
            print("[shaderc] Generatin CMake project for config " .. cfg .. ">" .. cmd)
            local genCmake = os.execute(cmd)
            if genCmake ~= true then
                error("[shaderc] ERROR: Could not generate cmake project")
                gbUseShaderc = false
                os.chdir(gb_SolutionDir)
                return
            end
                
            print("[shaderc] Building project >" .. "make shaderc -j " .. GetCoreCount())
            local build = os.execute("make shaderc -j " .. GetCoreCount())
            if build ~= true then
                error("[shaderc] ERROR: Failed to compile/link project")
                gbUseShaderc = false
            end
        
            print("[shaderc] Building project >" .. "make shaderc_combined -j " .. GetCoreCount())
            build = os.execute("make shaderc_combined -j " .. GetCoreCount())
            if build ~= true then
                error("[shaderc] ERROR: Failed to compile/link project")
                gbUseShaderc = false
            end
        
            local src = path.join(targetProjectPath, "libshaderc", "libshaderc.a")
            local target = path.join(gb_OutputDir, cfg, "libshaderc.a")
            print("[shaderc] Copying lib from " .. src .. " to ".. target)
            res, err = os.copyfile(src, target)
            if res == nil then
                error(err)
            end
        
            local src = path.join(targetProjectPath, "libshaderc", "libshaderc_combined.a")
            local target = path.join(gb_OutputDir, cfg, "libshaderc_combined.a")
            print("[shaderc] Copying lib from " .. src .. " to ".. target)
            res, err = os.copyfile(src, target)
            if res == nil then
                error(err)
            end
        end
    else
       error("[shaderc] ERROR: unsupportd platform")
        gbUseShaderc = false
    end

    os.chdir(gb_SolutionDir)
    
    gbUseShaderc = true
end

function UpdateConfig()
    local f = io.open("premake-config.lua", "w")
    
    f:write("gbUseSamples = " .. tostring(gbUseSamples) .. "\n")
    f:write("gbUseSampleScenes = " .. tostring(gbUseSampleScenes) .. "\n")
    f:write("gbUseShaderc = " .. tostring(gbUseShaderc) .. "\n")
    f:write("gbUseBreakpoints = " .. tostring(gbUseBreakpoints) .. "\n")
    f:write("gbWindowAPI = \"" .. gbWindowAPI .. "\"\n")
    
    f:close();
end

newaction {
    trigger = "update-sample-scenes",
    description = "project setup",
    execute = function ()        
        if gbUseSampleScenes == true then
            UpdateSampleScenes()
        end
    
        UpdateConfig()
    end
}

newaction {
    trigger = "update-shaderc",
    description = "project setup",
    execute = function ()        
        if gbUseShaderc == true then
            UpdateShaderCompiler()
        end
    
        UpdateConfig()
    end
}

newaction {
    trigger = "setup",
    description = "project setup",
    execute = function ()        
        gbUseSamples = _OPTIONS["samples"] ~= nil;
        gbUseSampleScenes = _OPTIONS["sample-scenes"] ~= nil;
        gbUseShaderc = _OPTIONS["shaderc"] ~= nil;
        gbUseBreakpoints = _OPTIONS["breakpoints"] ~= nil;
        gbWindowAPI = _OPTIONS["window"]
        
        if gbUseSampleScenes == true then
            UpdateSampleScenes()
        end
        if gbUseShaderc == true then
            UpdateShaderCompiler()
        end
        
        UpdateConfig()
        
        print("Setup complete")
    end
}