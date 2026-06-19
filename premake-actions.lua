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
    trigger = "SpirV",
    description = "Use SpirV for shaders. Compiling shaders from source code will require a shader compiler that would be downloaded in setup phase"    
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
        gbUseSpirV = false
        return
    end

    -- 2 setup the project
    os.chdir(shadercDir)
    print("[shaderc] Setting up project")
    if os.host() == "windows" then
       local getDeps = os.execute("python " .. path.join(path.getabsolute("."), "utils", "git-sync-deps"))
       if getDeps ~= true then
           error("[shaderc] ERROR: Could not get dependencies")
           gbUseSpirV = false
           os.chdir(gb_SolutionDir)
           return
       end
    elseif os.host() == "linux" then
        local getDeps = os.execute(path.join(path.getabsolute("."), "utils", "git-sync-deps"))
        if getDeps ~= true then
            error("[shaderc] ERROR: Could not get dependencies")
            gbUseSpirV = false
            os.chdir(gb_SolutionDir)
            return
        end
    elseif os.host() == "macosx" then
        error("[shaderc] ERROR: unsupportd platform")
        gbUseSpirV = false
        os.chdir(gb_SolutionDir)
        return
    end
    
    -- Ensure file exist
    print("[shaderc] creating bin folder")
    os.mkdir(gb_OutputDir)

    -- 3 Compile the project
    print("[shaderc] Building project")
    local Configs = { "Debug", "Development", "Release" }
    local CmakeConfigs = { "Debug", "RelWithDebInfo", "Release" }
    for cfgIndex, cfg in ipairs(Configs) do
        
        print("[shaderc] creating bin/cfg folder")
        local res, err = os.mkdir(path.join(gb_OutputDir, cfg))
        if res == nil then
            error(err)
        end
        print("[shaderc] creating target bin folder")
        os.mkdir(path.join(gb_OutputDir, cfg, "shaderc"))
        
        os.chdir(path.join(gb_OutputDir, cfg, "shaderc"))

        if os.host() == "windows" then
            print("[shaderc] Generatin CMake project >" .. "cmake ".. shadercDir)
            local genCmake = os.execute("cmake ".. shadercDir)
            if genCmake ~= true then
                error("[shaderc] ERROR: Could not generate cmake project")
                gbUseSpirV = false
                os.chdir(gb_SolutionDir)
                return
            end
        
            print("[shaderc] Building project for config " .. cfg ..  " >" .. "cmake --build . --config ".. CmakeConfigs[cfgIndex] .. " -j " .. GetCoreCount() .. " --target shaderc")
            local build = os.execute("cmake --build . --config ".. CmakeConfigs[cfgIndex] .. " -j " .. GetCoreCount() ..  " --target shaderc")
            if build ~= true then
                error("[shaderc] ERROR: Failed to compile/link project")
                gbUseSpirV = false
                os.chdir(gb_SolutionDir)
                return
            end
        
            print("[shaderc] Building project for config " .. cfg ..  " >" .. "cmake --build . --config ".. CmakeConfigs[cfgIndex] .. " -j " .. GetCoreCount() .. " --target shaderc_combined")
            build = os.execute("cmake --build . --config ".. CmakeConfigs[cfgIndex] .. " -j " .. GetCoreCount() ..  " --target shaderc_combined")
            if build ~= true then
                error("[shaderc] ERROR: Failed to compile/link project")
                gbUseSpirV = false
                os.chdir(gb_SolutionDir)
                return
            end
        
        elseif os.host() == "linux" then
            print("[shaderc] Generatin CMake project for config " .. cfg ..  " >" .. "cmake -G \"Unix Makefiles\" -DCMAKE_BUILD_TYPE=" .. CmakeConfigs[cfgIndex] .. " " .. shadercDir)
            local genCmake = os.execute("cmake -G \"Unix Makefiles\" -DCMAKE_BUILD_TYPE=" .. CmakeConfigs[cfgIndex] .. " " .. shadercDir)
            if genCmake ~= true then
                error("[shaderc] ERROR: Could not generate cmake project")
                gbUseSpirV = false
                os.chdir(gb_SolutionDir)
                return
            end
        
            print("[shaderc] Building project >" .. "make shaderc -j " .. GetCoreCount())
            local build = os.execute("make shaderc -j " .. GetCoreCount())
            if build ~= true then
                error("[shaderc] ERROR: Failed to compile/link project")
                gbUseSpirV = false
            end
        
            print("[shaderc] Building project >" .. "make shaderc_combined -j " .. GetCoreCount())
            build = os.execute("make shaderc_combined -j " .. GetCoreCount())
            if build ~= true then
                error("[shaderc] ERROR: Failed to compile/link project")
                gbUseSpirV = false
            end

        elseif os.host() == "macosx" then
            error("[shaderc] ERROR: unsupportd platform")
            gbUseSpirV = false
        end
    
        -- local res, err = os.copyfile(
        --     path.join(shadercDir, "libshaderc", CmakeConfigs[cfgIndex], "shaderc.lib"),
        --     path.join(gb_OutputDir, cfg, "shaderc.lib")
        -- )
        local src = path.join(gb_OutputDir, cfg, "shaderc", "libshaderc", CmakeConfigs[cfgIndex], "shaderc.lib")
        local target = path.join(gb_OutputDir, cfg, "shaderc.lib")
        print("[shaderc] Copying lib from " .. src .. " to ".. target)
        res, err = os.copyfile(src, target)
        if res == nil then
            error(err)
        end
    
        local src = path.join(gb_OutputDir, cfg, "shaderc", "libshaderc", CmakeConfigs[cfgIndex], "shaderc_combined.lib")
        local target = path.join(gb_OutputDir, cfg, "shaderc_combined.lib")
        print("[shaderc] Copying lib from " .. src .. " to ".. target)
        res, err = os.copyfile(src, target)
        if res == nil then
            error(err)
        end
    
        if (os.host() == "windows" and cfg ~= "Release" ) then
            src = path.join(gb_OutputDir, cfg, "shaderc", "libshaderc", CmakeConfigs[cfgIndex], "shaderc.pdb")
            target = path.join(gb_OutputDir, cfg, "shaderc.pdb")
            print("[shaderc] Copying symbols from " .. src .. " to ".. target)
            res, err = os.copyfile(src, target)
            if res == nil then
                error(err)
            end
        end
        os.chdir(gb_SolutionDir)
    end
    
    gbUseSpirV = true
end

function UpdateConfig()
    local f = io.open("premake-config.lua", "w")
    
    f:write("gbUseSamples = " .. tostring(gbUseSamples) .. "\n")
    f:write("gbUseSampleScenes = " .. tostring(gbUseSampleScenes) .. "\n")
    f:write("gbUseSpirV = " .. tostring(gbUseSpirV) .. "\n")
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
    trigger = "update-SpirV",
    description = "project setup",
    execute = function ()        
        if gbUseSpirV == true then
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
        gbUseSpirV = _OPTIONS["SpirV"] ~= nil;
        gbWindowAPI = _OPTIONS["window"]
        
        if gbUseSampleScenes == true then
            UpdateSampleScenes()
        end
        if gbUseSpirV == true then
            UpdateShaderCompiler()
        end
        
        UpdateConfig()
        
        print("Setup complete")
    end
}