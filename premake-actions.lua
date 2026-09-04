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

-- Unit tests projects
newoption {
   trigger = "unit-tests",
   description = "Generate Unit tests projects"
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
   trigger = "simd-x86-sse",
   description = "Enable SSE instruction generation for x86"
}

newoption {
   trigger = "simd-x86-avx",
   description = "Enable AVX instruction generation for x86"
}

newoption {
   trigger = "simd-x86-avx512",
   description = "Enable AVX-512 instruction generation for x86"
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

local PrimitiveTypes =
{
    Float = {
        Name = "float",
        Size = 4,
        IsFloatingPoint = true,
        x86_SIMD_Suffix = "ps",
        x86_SIMD_Reg_Suffix = "",
    },
    Double = {
        Name = "double",
        Size = 8,
        IsFloatingPoint = true,
        x86_SIMD_Suffix = "pd",
        x86_SIMD_Reg_Suffix = "d",
    },
    Int32 = {
        Name = "int32_t",
        Size = 4,
        IsFloatingPoint = false,
        x86_SIMD_Suffix = "epi32",
        x86_SIMD_Reg_Suffix = "i",
    },
    UInt32 = {
        Name = "uint32_t",
        Size = 4,
        IsFloatingPoint = false,
        x86_SIMD_Suffix = "epi32",
        x86_SIMD_Reg_Suffix = "i",
    },
    Int8 = {
        Name = "int8_t",
        Size = 1,
        IsFloatingPoint = false,
        x86_SIMD_Suffix = "epi8",
        x86_SIMD_Reg_Suffix = "i",
    },
    UInt8 = {
        Name = "uint8_t",
        Size = 1,
        IsFloatingPoint = false,
        x86_SIMD_Suffix = "epi8",
        x86_SIMD_Reg_Suffix = "i"
    },
}

local function MakeX86_SIMD_ISA(PrimitiveType, X86_Register, RegisterSize)
    return {
        Type = PrimitiveType.Name,
        ElementCount = math.floor(RegisterSize / PrimitiveType.Size),
        Register = X86_Register .. PrimitiveType.x86_SIMD_Reg_Suffix,
        Suffix = PrimitiveType.x86_SIMD_Suffix,
        Alignment = RegisterSize,
        IsFloatingPoint = PrimitiveType.IsFloatingPoint,
        ElementSize = PrimitiveType.Size,
    }
end

local ISAs = {
    x86_SSE = {
        Float = MakeX86_SIMD_ISA(PrimitiveTypes.Float, "__m128", 16),
        Double = MakeX86_SIMD_ISA(PrimitiveTypes.Double, "__m128", 16),
        Int32 = MakeX86_SIMD_ISA(PrimitiveTypes.Int32, "__m128", 16),
        UInt32 = MakeX86_SIMD_ISA(PrimitiveTypes.UInt32, "__m128", 16),
        Int8 = MakeX86_SIMD_ISA(PrimitiveTypes.Int8, "__m128", 16),
        UInt8 = MakeX86_SIMD_ISA(PrimitiveTypes.UInt8, "__m128", 16),
    },
    x86_AVX = {
        Float = MakeX86_SIMD_ISA(PrimitiveTypes.Float, "__m256", 32),
        Double = MakeX86_SIMD_ISA(PrimitiveTypes.Double, "__m256", 32),
        Int32 = MakeX86_SIMD_ISA(PrimitiveTypes.Int32, "__m256", 32),
        UInt32 = MakeX86_SIMD_ISA(PrimitiveTypes.UInt32, "__m256", 32),
        Int8 = MakeX86_SIMD_ISA(PrimitiveTypes.Int8, "__m256", 32),
        UInt8 = MakeX86_SIMD_ISA(PrimitiveTypes.UInt8, "__m256", 32),
    }, 
    x86_AVX_512 = {
        Float = MakeX86_SIMD_ISA(PrimitiveTypes.Float, "__m512", 64),
        Double = MakeX86_SIMD_ISA(PrimitiveTypes.Double, "__m512", 64),
        Int32 = MakeX86_SIMD_ISA(PrimitiveTypes.Int32, "__m512", 64),
        UInt32 = MakeX86_SIMD_ISA(PrimitiveTypes.UInt32, "__m512", 64),
        Int8 = MakeX86_SIMD_ISA(PrimitiveTypes.Int8, "__m512", 64),
        UInt8 = MakeX86_SIMD_ISA(PrimitiveTypes.UInt8, "__m512", 64),
    }
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

local function GetDependenciesProjectPath()
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

local function UpdateSampleScenes()
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

local function UpdateShaderCompiler()
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

local function GenerateCatch2Config()
    local in_file = path.join(gb_SourceDependencyDir, "Catch2", "src", "catch2", "catch_user_config.hpp.in")
    local out_file = path.join(gb_IntermediatesDir, "generated", "Catch2", "catch2", "catch_user_config.hpp")
    
    if not os.isdir(path.join(gb_IntermediatesDir, "generated", "Catch2", "catch2")) then
        os.mkdir(path.join(gb_IntermediatesDir, "generated", "Catch2", "catch2"))
    end
    
    -- Check if it already exists so we don't regenerate it unnecessarily
    if os.isfile(out_file) then
        return
    end
    print("Generating catch_user_config.hpp...")
    
    local f = io.open(in_file, "r")
    if not f then
        print("Warning: Could not open " .. in_file)
        return
    end
    local content = f:read("*a")
    f:close()
    
    -- 1. Replace all `#cmakedefine VAR` with `/* #undef VAR */` 
    -- This handles the feature toggles by turning them off (Catch2's default behavior)
    content = content:gsub("#cmakedefine%s+([%w_]+)[^\r\n]*", "/* #undef %1 */")
    
    -- 2. Replace the specific mandatory variables with Catch2's default values
    content = content:gsub("@CATCH_CONFIG_DEFAULT_REPORTER@", "console")
    content = content:gsub("@CATCH_CONFIG_CONSOLE_WIDTH@", "80")
    
    local out = io.open(out_file, "w")
    if not out then
        print("Error: Could not write to " .. out_file)
        return
    end
    
    out:write(content)
    out:close()
end

local function WriteMathSIMTx86Specialization(f, f2, x86_ISA, x86_ISA_Limit, PrimitiveType, RegisterCount)
    local ISA;
    if PrimitiveType == PrimitiveTypes.Float then
        ISA = x86_ISA.Float
    elseif PrimitiveType == PrimitiveTypes.Double then
        ISA = x86_ISA.Double
    elseif PrimitiveType == PrimitiveTypes.Int32 then
        ISA = x86_ISA.Int32
    elseif PrimitiveType == PrimitiveTypes.UInt32 then
        ISA = x86_ISA.UInt32
    elseif PrimitiveType == PrimitiveTypes.Int8 then
        ISA = x86_ISA.Int8
    elseif PrimitiveType == PrimitiveTypes.UInt8 then
        ISA = x86_ISA.UInt8
    else
        error("Unsupported x86 ISA")
    end

    -- Intrinsics parts
    local intrinCat;
    local intrinZeroSuffix;
    if x86_ISA == ISAs.x86_SSE then 
        intrinCat = "mm"
        if ISA.IsFloatingPoint == true then 
            intrinZeroSuffix = ISA.Suffix 
        else 
            intrinZeroSuffix = "si128" 
        end
    elseif x86_ISA == ISAs.x86_AVX then 
        intrinCat = "mm256"
        if ISA.IsFloatingPoint == true then 
            intrinZeroSuffix = ISA.Suffix 
        else 
            intrinZeroSuffix = "si256" 
        end
    elseif x86_ISA == ISAs.x86_AVX_512 then 
        intrinCat = "mm512"
        if ISA.IsFloatingPoint == true then 
            intrinZeroSuffix = ISA.Suffix 
        else 
            intrinZeroSuffix = "si512" 
        end
    else
        error("Unsupported ISA for x86")
    end

    local intrinMulName;
    if not ISA.IsFloatingPoint then intrinMulName = "mullo" else intrinMulName = "mul"end
    local intrinDivAvailable = ISA.IsFloatingPoint
    local intrinMulAvailable = true
    local intrinLoadStoreAvailable = true
    local intrinLoadStoreAlignedAvailable = true
    local intrinLoadStoreRequireInt32Cast = false
    local intrinBitShiftingAvailable = true
    local intrinMaskedLoadStoreAvailable = true
    local intrinMovemask32bits = true
    local intrinIs8bit = false
    local intrinIs32bit = false
    local intrinIs32bitInteger = false
    local intrinIs64bit = false
    local intrinIsScatterAvailable = false
    local intrinIsPermuteAvailagle = true
    local intrinIsCompressAvailable = false
    if x86_ISA_Limit == ISAs.x86_AVX_512 then
        intrinDivAvailable = true
        intrinIsScatterAvailable = true
        intrinIsCompressAvailable = true
    end
    if x86_ISA_Limit == ISAs.x86_SSE then
        intrinIsPermuteAvailagle = false
    end
    if PrimitiveType == PrimitiveTypes.Int8 then
        intrinMulAvailable = false
        intrinLoadStoreAvailable = false
        intrinBitShiftingAvailable = false
        intrinMaskedLoadStoreAvailable = false
        intrinMovemask32bits = false
        intrinIs8bit = true
        intrinIsPermuteAvailagle = false
        intrinIsScatterAvailable = false
    elseif PrimitiveType == PrimitiveTypes.UInt8 then
        intrinMulAvailable = false
        intrinLoadStoreAvailable = false
        intrinBitShiftingAvailable = false
        intrinMaskedLoadStoreAvailable = false
        intrinMovemask32bits = false
        intrinIs8bit = true
        intrinIsPermuteAvailagle = false
        intrinIsScatterAvailable = false
    elseif PrimitiveType == PrimitiveTypes.Float then
        intrinIs32bit = true
        if x86_ISA == ISAs.x86_SSE then
            intrinIsPermuteAvailagle = false
        end
    elseif PrimitiveType == PrimitiveTypes.Double then
        intrinIs64bit = true
        if x86_ISA == ISAs.x86_SSE then
            intrinIsPermuteAvailagle = false
        end
    elseif PrimitiveType == PrimitiveTypes.Int32 then
        intrinIs32bit = true
        intrinIs32bitInteger = true
        if x86_ISA == ISAs.x86_SSE then
            intrinIsPermuteAvailagle = false
        end
    elseif PrimitiveType == PrimitiveTypes.UInt32 then
        intrinIs32bit = true
        intrinIs32bitInteger = true
        intrinLoadStoreRequireInt32Cast = true
        if x86_ISA == ISAs.x86_SSE then
            intrinIsPermuteAvailagle = false
        end
    end
    local intrinArithmeticAvailable = {
        ["+"] = true, 
        ["-"] = true, 
        ["/"] = intrinDivAvailable, 
        ["*"] = intrinMulAvailable
    }

    -- Generated intrinsics
    local intrinFuncZero = "_" .. intrinCat .. "_setzero_" .. intrinZeroSuffix
    local intrinFuncSet = "_" .. intrinCat .. "_set_" .. ISA.Suffix
    local intrinFuncSet1 = "_" .. intrinCat .. "_set1_" .. ISA.Suffix
    local intrinFuncLoadUnaligned = "_" .. intrinCat .. "_loadu_" .. ISA.Suffix
    local intrinFuncLoadAligned = string.gsub(intrinFuncLoadUnaligned, "loadu", "load")
    local intrinFuncStoreUnaligned = string.gsub(intrinFuncLoadUnaligned, "loadu", "storeu")
    local intrinFuncStoreAligned = string.gsub(intrinFuncLoadUnaligned, "loadu", "store")
    local intrinFuncMaskedLoadUnaligned = string.gsub(intrinFuncLoadUnaligned, "loadu", "maskload")
    local intrinFuncMaskedLoadAligned = intrinFuncMaskedLoadUnaligned
    local intrinFuncMaskedStoreUnaligned = string.gsub(intrinFuncLoadUnaligned, "loadu", "maskstore")
    local intrinFuncMaskedStoreAligned = intrinFuncMaskedStoreUnaligned
    if x86_ISA_Limit == ISAs.x86_AVX_512 then
        intrinFuncMaskedLoadUnaligned = string.gsub(intrinFuncLoadUnaligned, "loadu", "mask_loadu")
        intrinFuncMaskedLoadAligned = string.gsub(intrinFuncLoadUnaligned, "loadu", "mask_load")
        intrinFuncMaskedStoreUnaligned = string.gsub(intrinFuncLoadUnaligned, "loadu", "mask_storeu")
        intrinFuncMaskedStoreAligned = string.gsub(intrinFuncLoadUnaligned, "loadu", "mask_store")
    end
    local intrinFuncAdd = "_" .. intrinCat .. "_add_" .. ISA.Suffix
    local intrinFuncSub = "_" .. intrinCat .. "_sub_" .. ISA.Suffix
    local intrinFuncMul = "_" .. intrinCat .. "_" .. intrinMulName .. "_" .. ISA.Suffix
    local intrinFuncDiv = "_" .. intrinCat .. "_div_" .. ISA.Suffix
    local intrinFuncRound = "_" .. intrinCat .. "_floor_" .. ISA.Suffix
    if x86_ISA_Limit == ISAs.x86_AVX_512 then
        intrinFuncRound = "_" .. intrinCat .. "_roundscale_" .. ISA.Suffix
    end
    local intrinFuncBitwiseAnd = "_" .. intrinCat .. "_and_" .. intrinZeroSuffix
    local intrinFuncBitwiseOr = "_" .. intrinCat .. "_or_" .. intrinZeroSuffix
    local intrinFuncBitwiseXOr = "_" .. intrinCat .. "_xor_" .. intrinZeroSuffix
    local intrinFuncShiftLeft = "_" .. intrinCat .. "_slli_" .. ISA.Suffix
    local intrinFuncShiftRight = "_" .. intrinCat .. "_srai_" .. ISA.Suffix
    local intrinFuncCmpEqual = "_" .. intrinCat .. "_cmpeq_" .. ISA.Suffix
    local intrinFuncCmpNotEqual = intrinFuncCmpEqual
    local intrinFuncCmpGreaterOrEqual = "_" .. intrinCat .. "_cmpge_" .. ISA.Suffix
    local intrinFuncCmpGreater = "_" .. intrinCat .. "_cmpgt_" .. ISA.Suffix
    local intrinFuncCmpLessOrEqual = "_" .. intrinCat .. "_cmple_" .. ISA.Suffix
    local intrinFuncCmpLess = "_" .. intrinCat .. "_cmplt_" .. ISA.Suffix
    if ISA.IsFloatingPoint == true then
        intrinFuncCmpEqual = string.gsub(intrinFuncCmpEqual, "cmpeq", "cmp")
        intrinFuncCmpNotEqual = string.gsub(intrinFuncCmpNotEqual, "cmpeq", "cmp")
        intrinFuncCmpGreaterOrEqual = string.gsub(intrinFuncCmpGreaterOrEqual, "cmpge", "cmp")
        intrinFuncCmpGreater = string.gsub(intrinFuncCmpGreater, "cmpgt", "cmp")
        intrinFuncCmpLessOrEqual = string.gsub(intrinFuncCmpLessOrEqual, "cmple", "cmp")
        intrinFuncCmpLess = string.gsub(intrinFuncCmpLess, "cmplt", "cmp")
    end
    if x86_ISA_Limit == ISAs.x86_AVX_512 then 
        intrinFuncCmpEqual = intrinFuncCmpEqual .. "_mask"
        intrinFuncCmpGreaterOrEqual = intrinFuncCmpGreaterOrEqual .. "_mask"
        intrinFuncCmpGreater = intrinFuncCmpGreater .. "_mask"
        intrinFuncCmpLessOrEqual = intrinFuncCmpLessOrEqual .. "_mask"
        intrinFuncCmpLess = intrinFuncCmpLess .. "_mask"

        if ISA.IsFloatingPoint == false then
            intrinFuncCmpNotEqual = string.gsub(intrinFuncCmpEqual, "cmpeq", "cmpneq")
        else
            intrinFuncCmpNotEqual = intrinFuncCmpEqual
        end
    end
    local intrinFuncBlend = "_" .. intrinCat .. "_blendv_" .. ISA.Suffix
    if x86_ISA_Limit == ISAs.x86_AVX_512 then 
        intrinFuncBlend = "_" .. intrinCat .. "_mask_blend_" .. ISA.Suffix
    elseif intrinIs32bitInteger == true then
        intrinFuncBlend = "_" .. intrinCat .. "_blend_" .. ISA.Suffix
    elseif ISA.IsFloatingPoint == true then
        intrinFuncBlend = "_" .. intrinCat .. "_blend_" .. ISA.Suffix
    end
    local intrinFuncShuffle = "_" .. intrinCat .. "_shuffle_" .. ISA.Suffix
    local intrinFuncI32Gather = "_" .. intrinCat .. "_i32gather_" .. ISA.Suffix
    local intrinFuncI32Scatter = "_" .. intrinCat .. "_i32scatter_" .. ISA.Suffix
    local intrinFuncPermute = "_" .. intrinCat .. "_permutevar8x32_" .. ISA.Suffix
    if x86_ISA_Limit == ISAs.x86_AVX_512 then
        intrinFuncPermute = "_" .. intrinCat .. "_permutexvar_" .. ISA.Suffix
    elseif x86_ISA_Limit == ISAs.x86_AVX then
        if intrinIs32bit == true then
            intrinFuncPermute = "_" .. intrinCat .. "_permutevar8x32_" .. ISA.Suffix
        elseif intrinIs64bit == true then
            intrinFuncPermute = "_" .. intrinCat .. "_permute4x64_" .. ISA.Suffix
        end
    end
    local intrinFuncCompress = "_" .. intrinCat .. "_maskz_compress_" .. ISA.Suffix
    local intrinFuncCompressPassTrough = "_" .. intrinCat .. "_mask_compress_" .. ISA.Suffix
    local intrinFuncExpand = "_" .. intrinCat .. "_maskz_expand_" .. ISA.Suffix
    local intrinFuncExpandPassTrough = "_" .. intrinCat .. "_mask_expand_" .. ISA.Suffix
    local intrinFuncPopcountMask = "_mm_popcnt_u64"

    -- operators
    local opsArithmetical = { 
        {"+", "add", intrinFuncAdd}, 
        {"-", "sub", intrinFuncSub}, 
        {"*", "mul", intrinFuncMul}, 
        {"/", "div", intrinFuncDiv}
    }
    local opsLogicalTests = { 
        {"==", intrinFuncCmpEqual, "", "_CMP_EQ_OQ", "" }, 
        {"!=", intrinFuncCmpEqual, "~", "_CMP_NEQ_UQ", "" }, 
        {">", intrinFuncCmpGreater, "", "_CMP_GT_OQ", "" }, 
        {">=", intrinFuncCmpGreaterOrEqual, "", "_CMP_GE_OQ", "" }, 
        {"<", intrinFuncCmpLess, "", "_CMP_LT_OQ", "" }, 
        {"<=", intrinFuncCmpLessOrEqual, "", "_CMP_LE_OQ", "" }, 
    }

    -- AVX only defines equals and greater than operators for ints
    if x86_ISA_Limit == ISAs.x86_AVX then
        opsLogicalTests[1][5] = "" -- ==
        opsLogicalTests[2][5] = "!(this->operator==(other))" -- !=
        opsLogicalTests[3][5] = "" -- >
        opsLogicalTests[4][5] = "(this->operator>(other) | this->operator==(other))" -- >=
        opsLogicalTests[5][5] = "other.operator>(*this)" -- <
        opsLogicalTests[6][5] = "(this->operator>(other) | this->operator==(other))" -- <=
    elseif x86_ISA_Limit == ISAs.x86_SSE then
        opsLogicalTests[1][5] = "" -- ==
        opsLogicalTests[2][5] = "!(this->operator==(other))" -- !=
        opsLogicalTests[3][5] = "" -- >
        opsLogicalTests[4][5] = "(this->operator>(other) | this->operator==(other))" -- >=
        opsLogicalTests[5][5] = "other.operator>(*this)" -- <
        opsLogicalTests[6][5] = "(this->operator>(other) | this->operator==(other))" -- <=
    end

    -- Shared generated code
    local snipetTemplateSpecialisation = "<".. ISA.Type .. ", " .. tostring(ISA.ElementCount) .. ">"
    local snipetsScalarType = "Scalar".. snipetTemplateSpecialisation
    local snipetsPrepareMaskBypass = "        const ".. snipetsScalarType .. "::MaskType::Type& intrin_mask = mask.bits;\n"
    local snipetsPrepareMask = ""
    if x86_ISA_Limit == ISAs.x86_SSE then 
        if intrinIs8bit then
            snipetsPrepareMask = snipetsPrepareMask .. "        __m128i intrin_mask = _mm_set1_epi32(mask.bits);\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        const __m128i shuffle_mask = _mm_setr_epi8(\n"
            snipetsPrepareMask = snipetsPrepareMask .. "            0,0,0,0,0,0,0,0,\n"
            snipetsPrepareMask = snipetsPrepareMask .. "            1,1,1,1,1,1,1,1\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        );\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        intrin_mask = " .. intrinFuncShuffle .. "(intrin_mask, shuffle_mask);\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        __m128i bit_isolate = _mm_setr_epi8(\n"
            snipetsPrepareMask = snipetsPrepareMask .. "            1u<<0u, 1u<<1u, 1u<<2u, 1u<<3u, 1u<<4u, 1u<<5u, 1u<<6u, 1u<<7u,\n"
            snipetsPrepareMask = snipetsPrepareMask .. "            1u<<0u, 1u<<1u, 1u<<2u, 1u<<3u, 1u<<4u, 1u<<5u, 1u<<6u, 1u<<7u\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        );\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        intrin_mask = _mm_and_si128(intrin_mask, bit_isolate);\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        intrin_mask = _mm_cmpeq_epi8(intrin_mask, bit_isolate);\n"
        else
            snipetsPrepareMask = snipetsPrepareMask .. "        __m128i intrin_mask = _mm_set1_epi32(mask.bits);\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        __m128i bit_isolate = _mm_setr_epi32(1<<0, 1<<1, 1<<2, 1<<3);\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        intrin_mask = _mm_and_si128(intrin_mask, bit_isolate);\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        intrin_mask = _mm_cmpeq_epi32(intrin_mask, bit_isolate);\n"
        end
    elseif x86_ISA_Limit == ISAs.x86_AVX then 
        if intrinIs8bit then
            snipetsPrepareMask = snipetsPrepareMask .. "        __m256i intrin_mask = _mm256_set1_epi32(mask.bits);\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        const __m256i shuffle_mask = _mm256_setr_epi8(\n"
            snipetsPrepareMask = snipetsPrepareMask .. "            0,0,0,0,0,0,0,0,\n"
            snipetsPrepareMask = snipetsPrepareMask .. "            1,1,1,1,1,1,1,1,\n"
            snipetsPrepareMask = snipetsPrepareMask .. "            2,2,2,2,2,2,2,2,\n"
            snipetsPrepareMask = snipetsPrepareMask .. "            3,3,3,3,3,3,3,3\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        );\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        intrin_mask = " .. intrinFuncShuffle .. "(intrin_mask, shuffle_mask);\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        __m256i bit_isolate = _mm256_setr_epi8(\n"
            snipetsPrepareMask = snipetsPrepareMask .. "            1u<<0u, 1u<<1u, 1u<<2u, 1u<<3u, 1u<<4u, 1u<<5u, 1u<<6u, 1u<<7u,\n"
            snipetsPrepareMask = snipetsPrepareMask .. "            1u<<0u, 1u<<1u, 1u<<2u, 1u<<3u, 1u<<4u, 1u<<5u, 1u<<6u, 1u<<7u,\n"
            snipetsPrepareMask = snipetsPrepareMask .. "            1u<<0u, 1u<<1u, 1u<<2u, 1u<<3u, 1u<<4u, 1u<<5u, 1u<<6u, 1u<<7u,\n"
            snipetsPrepareMask = snipetsPrepareMask .. "            1u<<0u, 1u<<1u, 1u<<2u, 1u<<3u, 1u<<4u, 1u<<5u, 1u<<6u, 1u<<7u\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        );\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        intrin_mask = _mm256_and_si256(intrin_mask, bit_isolate);\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        intrin_mask = _mm256_cmpeq_epi32(intrin_mask, bit_isolate);\n"
        else
            snipetsPrepareMask = snipetsPrepareMask .. "        __m256i intrin_mask = _mm256_set1_epi32(mask.bits);\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        __m256i bit_isolate = _mm256_setr_epi32(1<<0, 1<<1, 1<<2, 1<<3, 1<<4, 1<<5, 1<<6, 1<<7);\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        intrin_mask = _mm256_and_si256(intrin_mask, bit_isolate);\n"
            snipetsPrepareMask = snipetsPrepareMask .. "        intrin_mask = _mm256_cmpeq_epi32(intrin_mask, bit_isolate);\n"
        end
    elseif x86_ISA_Limit == ISAs.x86_AVX_512 then
        snipetsPrepareMask = snipetsPrepareMask .. "        const ".. snipetsScalarType .. "::MaskType::Type& intrin_mask = mask.bits;\n"
    else
        error("Unsupported ISA for x86")
    end
    local snipetsPackMask = ""
    if x86_ISA_Limit == ISAs.x86_AVX_512 then
        snipetsPackMask = snipetsPackMask .. "res"
    elseif PrimitiveType == PrimitiveTypes.Double then
        snipetsPackMask = snipetsPackMask .. "_" .. intrinCat .. "_movemask_pd(res)"
    elseif intrinMovemask32bits then
        snipetsPackMask = snipetsPackMask .. "_" .. intrinCat .. "_movemask_ps("
        if ISA.IsFloatingPoint == false then
            snipetsPackMask = snipetsPackMask .. "_" .. intrinCat .. "_cast" .. intrinZeroSuffix .. "_ps("
        end
        snipetsPackMask = snipetsPackMask .. "res)"
        if ISA.IsFloatingPoint == false then
            snipetsPackMask = snipetsPackMask .. ")"
        end
    else
        snipetsPackMask = snipetsPackMask .. "_" .. intrinCat .. "_movemask_epi8(res)"
    end
    local snipetPack2OneBitsIndices = "    int intrin_indices = 0;\n"
    snipetPack2OneBitsIndices = snipetPack2OneBitsIndices .. "    intrin_indices |= (indices[0] & 1);\n"
    snipetPack2OneBitsIndices = snipetPack2OneBitsIndices .. "    intrin_indices |= (indices[1] & 1) << 1;\n"
    local snipetPack4TwoBitsIndices = "    int intrin_indices = 0;\n"
    snipetPack4TwoBitsIndices = snipetPack4TwoBitsIndices .. "    intrin_indices |= (indices[0] & 3);\n"
    snipetPack4TwoBitsIndices = snipetPack4TwoBitsIndices .. "    intrin_indices |= (indices[1] & 3) << (1 * 2);\n"
    snipetPack4TwoBitsIndices = snipetPack4TwoBitsIndices .. "    intrin_indices |= (indices[2] & 3) << (2 * 2);\n"
    snipetPack4TwoBitsIndices = snipetPack4TwoBitsIndices .. "    intrin_indices |= (indices[3] & 3) << (3 * 2);\n"

    f:write("// Scalar specialization for " .. ISA.Type .. " x " .. ISA.ElementCount .. "\n")
    f:write("template<> struct alignas(" .. ISA.Alignment .. ") " .. snipetsScalarType .. "\n")
    f:write("{\n")
    f:write("    using Type = ".. ISA.Type ..";\n")
    f:write("    using MaskType = Mask<".. ISA.ElementCount ..">;\n")
    f:write("    using IndexerType = Scalar<int32_t, ".. ISA.ElementCount ..">;\n")
    f:write("    \n")
    f:write("    static constexpr size_t kThreadCount = ".. ISA.ElementCount ..";\n")
    f:write("    static constexpr size_t kAlignment = ".. ISA.Alignment ..";\n")
    f:write("    \n")
    f:write("    static consteval size_t Size() {return kThreadCount;}\n")
    f:write("    \n")
    f:write("#if defined(__GNUC__) || defined(__clang__)\n")
    f:write("   Type ALIGNED_VECTOR(kAlignment, kAlignment) m;\n")
    f:write("#else\n")
    f:write("   union { " .. ISA.Register .. " reg; Type m[kThreadCount]; };\n")
    f:write("#endif\n")
    f:write("\n")
    f:write("    INLINE Scalar() : reg(" .. intrinFuncZero .. "()) {}\n")
    f:write("    INLINE Scalar(Type val) : reg(" .. intrinFuncSet1 .. "(val)) {}\n")
    f:write("    INLINE Scalar(" .. ISA.Register .. " registerVector) : reg(registerVector) {}\n")
    f:write("    INLINE Scalar(\n        ")
    for i = 1,(ISA.ElementCount - 1) do
        f:write("Type e"..i..", ")
        if i % 4 == 0 then
            f:write("\n        ")
        end
    end
    f:write("Type e"..ISA.ElementCount.."\n    ):\n        reg("..intrinFuncSet.."(\n            ")
    
    for i = 1,(ISA.ElementCount - 1) do
        f:write("e"..i..", ")
        if i % 4 == 0 then
            f:write("\n            ")
        end 
    end
    f:write("e"..ISA.ElementCount..")\n        ) \n    {}\n")
    f:write("    INLINE Scalar(std::initializer_list<Type> list)\n")
    f:write("    {\n")
    f:write("        Type tmp [kThreadCount] = {0}; size_t i = 0;\n")
    f:write("        \n")
    f:write("        for (auto v : list) if (i < kThreadCount) tmp[i++] = v;\n")
    f:write("        \n")
    f:write("        reg = " .. intrinFuncLoadUnaligned .. "(tmp);\n")
    f:write("    }\n")
    f:write("    INLINE Scalar(const Scalar& other) : reg(other.reg){}\n")
    f:write("    INLINE Scalar& operator = (Type val) {reg = " .. intrinFuncSet1 .. "(val); return *this;}\n")
    f:write("    INLINE Scalar& operator = (const Scalar& other) {reg = other.reg; return *this;}\n")
    f:write("    INLINE Scalar& operator = (std::initializer_list<Type> list)\n")
    f:write("    {\n")
    f:write("        Type tmp [kThreadCount] = {0}; size_t i = 0;\n")
    f:write("        \n")
    f:write("        for (auto v : list) if (i < kThreadCount) tmp[i++] = v;\n")
    f:write("        \n")
    f:write("        reg = " .. intrinFuncLoadUnaligned .. "(tmp);\n")
    f:write("        return *this;\n")
    f:write("    }\n")
    f:write("    \n")
    f:write("    INLINE Type& operator [] (size_t index) {return m[index];}\n")
    f:write("    INLINE const Type& operator [] (size_t index) const {return m[index];}\n")
    f:write("    \n")

    -- Arithmetical operators
    for _, op_info in ipairs(opsArithmetical) do
        local op, name, func = op_info[1], op_info[2], op_info[3]

        f:write("    INLINE Scalar& operator " .. op .. "= (const Scalar& V)\n")
        f:write("    {\n")
        if not intrinArithmeticAvailable[op] then
            f:write("        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(V.m, kAlignment)\n")
            f:write("        for (size_t i = 0; i < kThreadCount; i++)\n")
            f:write("        {\n")
            f:write("            m[i] = m[i] " .. op .. " V.m[i];\n")
            f:write("        }\n")
        else
            f:write("        reg = " .. func .. "(reg, V.reg);\n")
        end
        f:write("        \n")
        f:write("        return *this;\n")
        f:write("    }\n")
        f:write("    INLINE Scalar& operator " .. op .. "= (Type val) {return (*this) " .. op .. "= Scalar(val);}\n")

        f:write("    INLINE Scalar operator " .. op .. " (const Scalar& V) const\n")
        f:write("    {\n")
        f:write("        Scalar r;\n")
        if not intrinArithmeticAvailable[op] then
            f:write("        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(V.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment)\n")
            f:write("        for (size_t i = 0; i < kThreadCount; i++)\n")
            f:write("        {\n")
            f:write("            r.m[i] = m[i] " .. op .. " V.m[i];\n")
            f:write("        }\n")
        else
            f:write("        r.reg = " .. func .. "(reg, V.reg);\n")
        end
        f:write("        \n")
        f:write("        return r;\n")
        f:write("    }\n")
        f:write("    INLINE Scalar operator " .. op .. " (Type val) const {return (*this) " .. op .. " Scalar(val);}\n")
        f:write("    \n")
    end
    f:write("    INLINE Scalar operator % (const Scalar& V)\n")
    f:write("    {\n")
    f:write("        Scalar div = *this / V, truncated;\n")
    if ISA.IsFloatingPoint == true then
        if x86_ISA_Limit == ISAs.x86_AVX_512 then
            f:write("        truncated.reg = " .. intrinFuncRound .. "(div.reg, _MM_FROUND_TO_NEG_INF);\n")
        else
            f:write("        truncated.reg = " .. intrinFuncRound .. "(div.reg);\n")
        end
        f:write("        Scalar r = div - truncated;\n")
        f:write("        return r * V;\n")
    else
        f:write("        truncated = div * V;\n")
        f:write("        return *this - truncated;\n")
    end

    f:write("    }\n")
    f:write("    INLINE Scalar operator % (Type val) {return (*this) % Scalar(val);}\n")
    f:write("    INLINE Scalar operator-() const\n")
    f:write("    {\n")
    if ISA.IsFloatingPoint == true then
        -- f:write("        return Scalar(-0) ^ *this;\n")
        f:write("        return *this * Scalar(-1);\n")
    else
        f:write("        return Scalar(0) - *this;\n")
    end
    f:write("    }\n")
    f:write("    \n")

    -- Set operators
    f:write("    INLINE Scalar& Zero() {reg = " .. intrinFuncZero .. "(); return *this;}\n")
    f:write("    \n")

    -- Load/Store operators
    f:write("    INLINE static Scalar Load(const Type* ptr)\n")
    f:write("    {\n")
    f:write("        Scalar r;\n")
    if intrinLoadStoreAvailable then
        f:write("        r.reg = " .. intrinFuncLoadUnaligned .. "(ptr);\n")
    else
        f:write("        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment)\n")
        f:write("        for (size_t i = 0; i < kThreadCount; i++)\n")
        f:write("        {\n")
        f:write("            r.m[i] = ptr[i];\n")
        f:write("        }\n")
    end
    f:write("        return r;\n")
    f:write("    }\n")
    f:write("    INLINE static Scalar Load(const Type* ptr, const MaskType& mask)\n")
    f:write("    {\n")
    f:write("        Scalar r;\n")
    if intrinMaskedLoadStoreAvailable then
        f:write(         snipetsPrepareMask)
        if x86_ISA_Limit == ISAs.x86_AVX_512 then
            f:write("        r.reg = " .. intrinFuncMaskedLoadUnaligned .. "(r.reg, intrin_mask, ptr);\n")
        elseif intrinLoadStoreRequireInt32Cast == true then
            f:write("        r.reg = " .. intrinFuncMaskedLoadUnaligned .. "((const int*)(ptr), intrin_mask);\n")
        else
            f:write("        r.reg = " .. intrinFuncMaskedLoadUnaligned .. "(ptr, intrin_mask);\n")
        end
    else
        f:write("        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment)\n")
        f:write("        for (size_t i = 0; i < kThreadCount; i++)\n")
        f:write("        {\n")
        f:write("            r.m[i] = mask[i] ? ptr[i] : Type(0);\n")
        f:write("        }\n")
    end
    f:write("        return r;\n")
    f:write("    }\n")
    f:write("    INLINE static Scalar LoadAligned(const Type* ptr)\n")
    f:write("    {\n")
    f:write("        Scalar r;\n")
    if intrinLoadStoreAvailable then
        f:write("        r.reg = " .. intrinFuncLoadAligned .. "(ptr);\n")
    else
        f:write("        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(ptr, kAlignment)\n")
        f:write("        for (size_t i = 0; i < kThreadCount; i++)\n")
        f:write("        {\n")
        f:write("            r.m[i] = ptr[i];\n")
        f:write("        }\n")
    end
    f:write("        return r;\n")
    f:write("    }\n")
    f:write("    INLINE static Scalar LoadAligned(const Type* ptr, const MaskType& mask)\n")
    f:write("    {\n")
    f:write("        Scalar r;\n")
    if intrinMaskedLoadStoreAvailable then
        f:write(         snipetsPrepareMask)
        if x86_ISA_Limit == ISAs.x86_AVX_512 then
            f:write("        r.reg = " .. intrinFuncMaskedLoadAligned .. "(r.reg, intrin_mask, ptr);\n")
        elseif intrinLoadStoreRequireInt32Cast == true then
            f:write("        r.reg = " .. intrinFuncMaskedLoadAligned .. "((const int*)(ptr), intrin_mask);\n")
        else
            f:write("        r.reg = " .. intrinFuncMaskedLoadAligned .. "(ptr, intrin_mask);\n")
        end
    else
        f:write("        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(ptr, kAlignment)\n")
        f:write("        for (size_t i = 0; i < kThreadCount; i++)\n")
        f:write("        {\n")
        f:write("            r.m[i] = mask[i] ? ptr[i] : Type(0);\n")
        f:write("        }\n")
    end
    f:write("        return r;\n")
    f:write("    }\n")
    f:write("    \n")
    f:write("    INLINE void Store(Type* ptr)\n")
    f:write("    {\n")
    if intrinLoadStoreAvailable then
        f:write("        " .. intrinFuncStoreUnaligned .. "(ptr, reg);\n")
    else
        f:write("        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment)\n")
        f:write("        for (size_t i = 0; i < kThreadCount; i++)\n")
        f:write("        {\n")
        f:write("            ptr[i] = m[i];\n")
        f:write("        }\n")
    end
    f:write("    }\n")
    f:write("    INLINE void Store(Type* ptr, const MaskType& mask)\n")
    f:write("    {\n")
    if intrinMaskedLoadStoreAvailable then
        f:write(         snipetsPrepareMask)
        if x86_ISA_Limit == ISAs.x86_AVX_512 then
            f:write("        " .. intrinFuncMaskedStoreUnaligned .. "(ptr, intrin_mask, reg);\n")
        elseif intrinLoadStoreRequireInt32Cast == true then
            f:write("        " .. intrinFuncMaskedStoreUnaligned .. "((int*)(ptr), intrin_mask, reg);\n")
        else
            f:write("        " .. intrinFuncMaskedStoreUnaligned .. "(ptr, intrin_mask, reg);\n")
        end
    else
        f:write("        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment)\n")
        f:write("        for (size_t i = 0; i < kThreadCount; i++)\n")
        f:write("        {\n")
        f:write("            ptr[i] = mask[i] ? m[i] : ptr[i];\n")
        f:write("        }\n")
    end
    f:write("    }\n")
    f:write("    INLINE void StoreAligned(Type* ptr)\n")
    f:write("    {\n")
    if intrinLoadStoreAvailable then
        f:write("        " .. intrinFuncStoreAligned .. "(ptr, reg);\n")
    else
        f:write("        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(ptr, kAlignment)\n")
        f:write("        for (size_t i = 0; i < kThreadCount; i++)\n")
        f:write("        {\n")
        f:write("            ptr[i] = m[i];\n")
        f:write("        }\n")
    end
    f:write("    }\n")
    f:write("    INLINE void StoreAligned(Type* ptr, const MaskType& mask)\n")
    f:write("    {\n")
    if intrinMaskedLoadStoreAvailable then
        f:write(         snipetsPrepareMask)
        if x86_ISA_Limit == ISAs.x86_AVX_512 then
            f:write("        " .. intrinFuncMaskedStoreAligned .. "(ptr, intrin_mask, reg);\n")
        elseif intrinLoadStoreRequireInt32Cast == true then
            f:write("        " .. intrinFuncMaskedStoreAligned .. "((int*)(ptr), intrin_mask, reg);\n")
        else
            f:write("        " .. intrinFuncMaskedStoreAligned .. "(ptr, intrin_mask, reg);\n")
        end
    else
        f:write("        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(ptr, kAlignment)\n")
        f:write("        for (size_t i = 0; i < kThreadCount; i++)\n")
        f:write("        {\n")
        f:write("            ptr[i] = mask[i] ? m[i] : ptr[i];\n")
        f:write("        }\n")
    end
    f:write("    }\n")
    f:write("    INLINE void Set(const Type& val)\n")
    f:write("    {\n")
    f:write("        ".. intrinFuncSet1 .."(val);\n")
    f:write("    }\n")
    f:write("    \n")

    -- Bitwise binary operators
    f:write("    INLINE Scalar operator&(const Scalar& other) const\n")
    f:write("    {\n")
    f:write("        return " .. intrinFuncBitwiseAnd .. "(reg, other.reg);\n")
    f:write("    }\n")
    f:write("    INLINE Scalar& operator&=(const Scalar& other) {*this = *this & other; return *this;}\n")
    f:write("    INLINE Scalar operator|(const Scalar& other) const\n")
    f:write("    {\n")
    f:write("        return " .. intrinFuncBitwiseOr .. "(reg, other.reg);\n")
    f:write("    }\n")
    f:write("    INLINE Scalar& operator|=(const Scalar& other) {*this = *this | other; return *this;}\n")
    f:write("    INLINE Scalar operator^(const Scalar& other) const\n")
    f:write("    {\n")
    f:write("        return " .. intrinFuncBitwiseXOr .. "(reg, other.reg);\n")
    f:write("    }\n")
    f:write("    INLINE Scalar& operator^=(const Scalar& other) {*this = *this ^ other; return *this;}\n")
    f:write("    INLINE Scalar operator~() const\n")
    f:write("    {\n")
    f:write("        return " .. intrinFuncBitwiseXOr .. "(reg, " .. intrinFuncSet1 .. "(Type(-1)));\n")
    f:write("    }\n")
    if not ISA.IsFloatingPoint == true then
        f:write("    INLINE Scalar operator<<(int s) const\n")
        f:write("    {\n")
        if intrinBitShiftingAvailable then
            f:write("        return " .. intrinFuncShiftLeft .. "(reg, s);\n")
        else
            f:write("        Scalar r;\n")
            f:write("        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(ptr, kAlignment)\n")
            f:write("        for (size_t i = 0; i < kThreadCount; i++)\n")
            f:write("        {\n")
            f:write("            r.m[i] = m[i] << s;\n")
            f:write("        }\n")
            f:write("        return r;\n")
        end
        f:write("    }\n")
        f:write("    INLINE Scalar operator>>(int s) const\n")
        f:write("    {\n")
        if intrinBitShiftingAvailable then
            f:write("        return " .. intrinFuncShiftRight .. "(reg, s);\n")
        else
            f:write("        Scalar r;\n")
            f:write("        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(ptr, kAlignment)\n")
            f:write("        for (size_t i = 0; i < kThreadCount; i++)\n")
            f:write("        {\n")
            f:write("            r.m[i] = m[i] >> s;\n")
            f:write("        }\n")
            f:write("        return r;\n")
        end
        f:write("    }\n")
    end
    f:write("    \n")

    -- Comparaisons operators
    for _, op_info in ipairs(opsLogicalTests) do
        local op, func, post_operator, op_enum, op_fallback = op_info[1], op_info[2], op_info[3], op_info[4], op_info[5]
        f:write("    INLINE MaskType operator" .. op .. "(const Scalar& other) const\n")
        f:write("    {\n")
        if op_fallback == "" then
            if x86_ISA_Limit == ISAs.x86_AVX_512 then
                f:write("        MaskType res = ");
            else
                f:write("        " .. ISA.Register .. " res = ");
            end
            if ISA.IsFloatingPoint == false then
                f:write("" ..  func .. "(reg, other.reg);\n")
                f:write("        return " .. post_operator .. "MaskType(".. snipetsPackMask.. ");\n")
            else
                f:write("" ..  func .. "(reg, other.reg, " .. op_enum .. ");\n")
                f:write("        return MaskType(".. snipetsPackMask.. ");\n")
            end
        else
            f:write("        return " .. op_fallback .. ";\n")
        end
        f:write("    }\n")
    end
    f:write("};\n")
    f2:write("template<>\n")
    f2:write("INLINE " .. snipetsScalarType .." Select".. snipetTemplateSpecialisation .. "(\n")
    f2:write("    const " .. snipetsScalarType .. "& A,\n")
    f2:write("    const " .. snipetsScalarType .. "& B,\n")
    f2:write("    const " .. snipetsScalarType .. "::MaskType& /*Is A*/ mask\n")
    f2:write("    )\n")
    f2:write("{\n")
    if x86_ISA_Limit == ISAs.x86_AVX_512 then 
        f2:write(     snipetsPrepareMask)
        f2:write("    return " .. intrinFuncBlend .. "(intrin_mask, B.reg, A.reg);\n")
    else
        if intrinIs32bitInteger == true then
            f2:write(     snipetsPrepareMaskBypass)
        elseif ISA.IsFloatingPoint == true then 
            f2:write(     snipetsPrepareMaskBypass)
        else
            f2:write(     snipetsPrepareMask)
        end
        f2:write("    return " .. intrinFuncBlend .. "(B.reg, A.reg, intrin_mask);\n")
    end 
    f2:write("    \n")
    f2:write("}\n")
    f2:write("\n")
    f2:write("template<>\n")
    f2:write("INLINE " .. snipetsScalarType .." Gather".. snipetTemplateSpecialisation .. "(\n")
    f2:write("    const " .. ISA.Type .. "* ptr,\n")
    f2:write("    const " .. snipetsScalarType .. "::IndexerType& indices\n")
    f2:write("    )\n")
    f2:write("{\n")
    if intrinMaskedLoadStoreAvailable == false then
        f2:write("    " .. snipetsScalarType .. " r;\n")
        f2:write("    MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment)\n")
        f2:write("    for (size_t i = 0; i < " .. snipetsScalarType .. "::kThreadCount; i++)\n")
        f2:write("    {\n")
        f2:write("        r.m[i] = ptr[indices[i]];\n")
        f2:write("    }\n")
        f2:write("    return r;\n")
    else
        if x86_ISA == ISAs.x86_SSE and intrinIs64bit == true then
            f2:write("    Scalar<int32_t , " .. tostring(ISA.ElementCount * 2) .. "> intrin_indices;\n")
            f2:write("    intrin_indices.Load(indices.m);\n")
        else
            f2:write("    const " .. snipetsScalarType .. "::IndexerType& intrin_indices = indices;\n")
        end

        if x86_ISA == ISAs.x86_AVX_512 then
            f2:write("    return " .. intrinFuncI32Gather .. "(intrin_indices.reg, ptr, sizeof(" .. ISA.Type .. "));\n")
        elseif intrinLoadStoreRequireInt32Cast == true then
            f2:write("    return " .. intrinFuncI32Gather .. "((int*)(ptr), intrin_indices.reg, sizeof(" .. ISA.Type .. "));\n")
        else
            f2:write("    return " .. intrinFuncI32Gather .. "(ptr, intrin_indices.reg, sizeof(" .. ISA.Type .. "));\n")
        end
    end
    f2:write("}\n")
    f2:write("\n")
    f2:write("template<>\n")
    f2:write("INLINE void Scatter".. snipetTemplateSpecialisation .. "(\n")
    f2:write("    const " .. snipetsScalarType .. "& values,\n")
    f2:write("    " .. ISA.Type .. "* ptr,\n")
    f2:write("    const " .. snipetsScalarType .. "::IndexerType& indices\n")
    f2:write("    )\n")
    f2:write("{\n")
    if intrinIsScatterAvailable == true then
        if x86_ISA == ISAs.x86_SSE and intrinIs64bit == true then
            f2:write("    Scalar<int32_t , " .. tostring(ISA.ElementCount * 2) .. "> intrin_indices;\n")
            f2:write("    intrin_indices.Load(indices.m);\n")
        else
            f2:write("    const " .. snipetsScalarType .. "::IndexerType& intrin_indices = indices;\n")
        end

        f2:write("    " .. intrinFuncI32Scatter .. "(ptr, intrin_indices.reg, values.reg, sizeof(" .. ISA.Type .. "));\n")
    else
        f2:write("    MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(V.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(indices.m, kAlignment)\n")
        f2:write("    for (size_t i = 0; i < " .. snipetsScalarType .. "::kThreadCount; i++)\n")
        f2:write("    {\n")
        f2:write("        ptr[indices[i]] = values.m[i];\n")
        f2:write("    }\n")
    end
    f2:write("}\n")
    f2:write("\n")
    f2:write("template<>\n")
    f2:write("INLINE " .. snipetsScalarType .." Permute".. snipetTemplateSpecialisation .. "(\n")
    f2:write("    const " .. snipetsScalarType .. "& V,\n")
    f2:write("    const " .. snipetsScalarType .. "::IndexerType& indices\n")
    f2:write("    )\n")
    f2:write("{\n")
    if intrinIsPermuteAvailagle == true then
        if x86_ISA_Limit == ISAs.x86_AVX_512 then 
            if intrinIs64bit == true then
                if x86_ISA == ISAs.x86_AVX_512 then
                    f2:write("    Scalar<int32_t , " .. tostring(ISA.ElementCount * 2) .. "> intrin_indices = _mm512_cvtepi32_epi64(indices.reg);\n")
                elseif x86_ISA == ISAs.x86_AVX then
                    f2:write("    Scalar<int32_t , " .. tostring(ISA.ElementCount * 2) .. "> intrin_indices = _mm256_cvtepi32_epi64(indices.reg);\n")
                end
            else
                f2:write("    const " .. snipetsScalarType .. "::IndexerType& intrin_indices = indices;\n")
            end
            f2:write("    return " .. intrinFuncPermute .. "(intrin_indices.reg, V.reg);\n")
        elseif x86_ISA_Limit == ISAs.x86_AVX then
            if intrinIs32bit == true then
                f2:write("    return " .. intrinFuncPermute .. "(V.reg, indices.reg);\n")
            elseif intrinIs64bit == true then
                f2:write(    snipetPack4TwoBitsIndices);
                f2:write("    return " .. intrinFuncPermute .. "(V.reg, intrin_indices);\n")
            end
        end
    else
        f2:write("    " .. snipetsScalarType .." r;\n")
        f2:write("    MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(V.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(indices.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment)\n")
        f2:write("    for (size_t i = 0; i < " .. snipetsScalarType .. "::kThreadCount; i++)\n")
        f2:write("    {\n")
        f2:write("        r.m[i] = V.m[indices[i]];\n")
        f2:write("    }\n")
        f2:write("    return r;\n")
    end
    f2:write("}\n")
    f2:write("\n")
    f2:write("template<>\n")
    f2:write("INLINE " .. snipetsScalarType .." Shift".. snipetTemplateSpecialisation .. "(\n")
    f2:write("    const " .. snipetsScalarType .. "& V,\n")
    f2:write("    int Amount\n")
    f2:write("    )\n")
    f2:write("{\n")
    f2:write("    " .. snipetsScalarType .. "::IndexerType indices;\n")
    -- TODO introduce a hardware accelerated version when possible
    f2:write("    for (int i = 0; i < " .. snipetsScalarType .. "::kThreadCount; ++i)\n")
    f2:write("    {\n")
    f2:write("        indices[i] = std::clamp(i + Amount, 0, static_cast<int>(" .. snipetsScalarType .. "::kThreadCount));\n")
    f2:write("    }\n")
    f2:write("    \n")
    f2:write("    return Permute" .. snipetTemplateSpecialisation .. "(V, indices);\n")
    f2:write("}\n")
    f2:write("template<>\n")
    f2:write("INLINE " .. snipetsScalarType .." Rotate".. snipetTemplateSpecialisation .. "(\n")
    f2:write("    const " .. snipetsScalarType .. "& V,\n")
    f2:write("    int Amount\n")
    f2:write("    )\n")
    f2:write("{\n")
    f2:write("    " .. snipetsScalarType .. "::IndexerType indices;\n")
    f2:write("    const int base_offset = " .. snipetsScalarType .. "::kThreadCount + (Amount % " .. snipetsScalarType .. "::kThreadCount);\n")
    -- TODO introduce a hardware accelerated version when possible
    f2:write("    for (int i = 0; i < " .. snipetsScalarType .. "::kThreadCount; ++i)\n")
    f2:write("    {\n")
    f2:write("        indices[i] = (i + base_offset) % " .. snipetsScalarType .. "::kThreadCount;\n")
    f2:write("    }\n")
    f2:write("    \n")
    f2:write("    return Permute" .. snipetTemplateSpecialisation .. "(V, indices);\n")
    f2:write("}\n")
    f2:write("\n")
    -- f2:write("template<>\n")
    -- f2:write("INLINE " .. snipetsScalarType .." Shuffle".. snipetTemplateSpecialisation .. "(\n")
    -- f2:write("    const " .. snipetsScalarType .. "& A,\n")
    -- f2:write("    const " .. snipetsScalarType .. "& B,\n")
    -- f2:write("    const " .. snipetsScalarType .. "::IndexerType& indices\n")
    -- f2:write("    )\n")
    -- f2:write("{\n")
    -- f2:write("    \n")
    -- f2:write("}\n")
    -- f2:write("\n")
    -- f2:write("template<int... Indices>\n")
    -- f2:write("INLINE " .. snipetsScalarType .." Shuffle".. snipetTemplateSpecialisation .. "(\n")
    -- f2:write("    const " .. snipetsScalarType .. "& A,\n")
    -- f2:write("    const " .. snipetsScalarType .. "& B\n")
    -- f2:write("    )\n")
    -- f2:write("{\n")
    -- f2:write("    static_assert(sizeof...(Indices) == ThreadCount, \"Permute requires exactly N indices\");\n")
    -- f2:write("    int indices[] = { Indices... };\n")
    -- f2:write("    \n")
    -- f2:write("}\n")
    f2:write("template<>\n")
    f2:write("INLINE " .. snipetsScalarType .." Pack".. snipetTemplateSpecialisation .. "(\n")
    f2:write("    const " .. snipetsScalarType .. "& V,\n")
    f2:write("    const " .. snipetsScalarType .. "::MaskType& mask\n")
    f2:write("    )\n")
    f2:write("{\n")
    if intrinIsCompressAvailable == true then
        f2:write("    return " .. intrinFuncCompress .. "(mask.bits, V.reg);\n")
    else
        f2:write("    " .. snipetsScalarType .. " r(0);\n")
        f2:write("    size_t idx = 0;\n")
        -- cannot really auto simdify this sequencial operation
        f2:write("    MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(V.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment)\n")
        f2:write("    for (size_t i = 0; i < " .. snipetsScalarType .. "::kThreadCount; i++)\n")
        f2:write("    {\n")
        f2:write("        if (mask[i])\n")
        f2:write("            r.m[idx++] = V.m[i];\n")
        f2:write("    }\n")
        f2:write("    return r;\n")
    end
    f2:write("}\n")
    -- f2:write("template<>\n")
    -- f2:write("INLINE " .. snipetsScalarType .." Pack".. snipetTemplateSpecialisation .. "(\n")
    -- f2:write("    const " .. snipetsScalarType .. "& V,\n")
    -- f2:write("    const " .. snipetsScalarType .. "::IndexerType& groups\n")
    -- f2:write("    )\n")
    -- f2:write("{\n")
    -- if intrinIsCompressAvailable == true then
    --     f2:write("    return " .. intrinFuncCompress .. "(mask.bits, V.reg);\n")
    -- else
    --     f2:write("    " .. snipetsScalarType .. " r(0);\n")
    --     f2:write("    size_t idx = 0;\n")
    --     -- cannot really auto simdify this sequencial operation
    --     f2:write("    MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(V.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment)\n")
    --     f2:write("    for (size_t i = 0; i < " .. snipetsScalarType .. "::kThreadCount; i++)\n")
    --     f2:write("    {\n")
    --     f2:write("        if (mask[i])\n")
    --     f2:write("            r.m[idx++] = V.m[i];\n")
    --     f2:write("    }\n")
    --     f2:write("    return r;\n")
    -- end
    -- f2:write("}\n")
    f2:write("template<>\n")
    f2:write("INLINE " .. snipetsScalarType .." UnPack".. snipetTemplateSpecialisation .. "(\n")
    f2:write("    const " .. snipetsScalarType .. "& V,\n")
    f2:write("    const " .. snipetsScalarType .. "::MaskType& mask\n")
    f2:write("    )\n")
    f2:write("{\n")
    if intrinIsCompressAvailable == true then
        f2:write("    return " .. intrinFuncExpand .. "(mask.bits, V.reg);\n")
    else
        f2:write("    " .. snipetsScalarType .. " r(0);\n")
        f2:write("    size_t idx = 0;\n")
        -- cannot really auto simdify this sequencial operation
        f2:write("    MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(V.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment)\n")
        f2:write("    for (size_t i = 0; i < " .. snipetsScalarType .. "::kThreadCount; i++)\n")
        f2:write("    {\n")
        f2:write("        if (mask[i])\n")
        f2:write("            r.m[i] = V.m[idx++];\n")
        f2:write("    }\n")
        f2:write("    return r;\n")
    end
    f2:write("}\n")
    f2:write("template<>\n")
    f2:write("INLINE " .. snipetsScalarType .." Split".. snipetTemplateSpecialisation .. "(\n")
    f2:write("    const " .. snipetsScalarType .. "& V,\n")
    f2:write("    const " .. snipetsScalarType .. "::MaskType& mask\n")
    f2:write("    )\n")
    f2:write("{\n")
    if intrinIsCompressAvailable == true then
        f2:write("    const int bin1count = " .. intrinFuncPopcountMask .. "(mask.bits);\n")
        f2:write("    const int bin0count = " .. snipetsScalarType .. "::kThreadCount - bin1count; \n")
        f2:write("    \n")
        f2:write("    " .. snipetsScalarType .. " bin1 = Pack" .. snipetTemplateSpecialisation .. "(V, mask);\n")
        f2:write("    bin1 = Shift" .. snipetTemplateSpecialisation .. "(bin1, -bin0count);\n")
        f2:write("    \n")
        f2:write("    return " .. intrinFuncCompressPassTrough .. "(bin1.reg, (!mask).bits, V.reg);\n")
    else
        f2:write("    " .. snipetsScalarType .. " r(0);\n")
        f2:write("    \n")
        f2:write("    size_t count1 = 0;\n")
        f2:write("    for (size_t i = 0; i < " .. snipetsScalarType .. "::kThreadCount; ++i) if (mask[i]) count1++;\n")
        f2:write("    \n")
        f2:write("    size_t idx1 = count1, idx0 = 0;\n")
        f2:write("    MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(V.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment)\n")
        f2:write("    for (size_t i = 0; i < " .. snipetsScalarType .. "::kThreadCount; i++)\n")
        f2:write("    {\n")
        f2:write("        if (mask[i])\n")
        f2:write("            r.m[idx1++] = V.m[i];\n")
        f2:write("        else\n")
        f2:write("            r.m[idx0++] = V.m[i];\n")
        f2:write("    }\n")
        f2:write("    return r;\n")
    end
    f2:write("}\n")
    f2:write("template<>\n")
    f2:write("INLINE " .. snipetsScalarType .." UnSplit".. snipetTemplateSpecialisation .. "(\n")
    f2:write("    const " .. snipetsScalarType .. "& V,\n")
    f2:write("    const " .. snipetsScalarType .. "::MaskType& mask\n")
    f2:write("    )\n")
    f2:write("{\n")
    if intrinIsCompressAvailable == true then
        f2:write("    const size_t bin1count = " .. intrinFuncPopcountMask .. "(mask.bits);\n")
        f2:write("    const size_t bin0count = " .. snipetsScalarType .. "::kThreadCount - bin1count; \n")
        f2:write("    \n")
        f2:write("    " .. snipetsScalarType .. " expanded1 = UnPack".. snipetTemplateSpecialisation .. "(Shift" .. snipetTemplateSpecialisation .."(V, ((int)bin0count)), mask);\n")
        f2:write("    \n")
        f2:write("    return " .. intrinFuncExpandPassTrough .. "(expanded1.reg, (!mask).bits, V.reg);\n")
    else
        f2:write("    " .. snipetsScalarType .. " r(0);\n")
        f2:write("    \n")
        f2:write("    size_t count1 = 0;\n")
        f2:write("    for (size_t i = 0; i < " .. snipetsScalarType .. "::kThreadCount; ++i) if (mask[i]) count1++;\n")
        f2:write("    \n")
        f2:write("    size_t idx1 = count1, idx0 = 0;\n")
        f2:write("    MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(V.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment)\n")
        f2:write("    for (size_t i = 0; i < " .. snipetsScalarType .. "::kThreadCount; i++)\n")
        f2:write("    {\n")
        f2:write("        if (mask[i])\n")
        f2:write("            r.m[i] = V.m[idx1++];\n")
        f2:write("        else\n")
        f2:write("            r.m[i] = V.m[idx0++];\n")
        f2:write("    }\n")
        f2:write("    return r;\n")
    end
    f2:write("}\n")
    -- TODO introduce a hardware accelerated version when possible
    f2:write("template<>\n")
    f2:write("INLINE " .. snipetsScalarType .." Bin".. snipetTemplateSpecialisation .. "(\n")
    f2:write("    const " .. snipetsScalarType .. "& V,\n")
    f2:write("    const " .. snipetsScalarType .. "::IndexerType& bins\n")
    f2:write("    )\n")
    f2:write("{\n")
    f2:write("    " .. snipetsScalarType .. " r(0);\n")
    f2:write("    int max_bin = 0;\n")
    f2:write("    for (size_t i = 0; i < " .. snipetsScalarType .. "::kThreadCount; ++i) if (bins[i] > max_bin) max_bin = bins[i];\n")
    f2:write("    \n")
    f2:write("    size_t out_idx = 0;\n")
    f2:write("    for (int b = 0; b <= max_bin; ++b)\n")
    f2:write("    for (size_t i = 0; i < " .. snipetsScalarType .. "::kThreadCount; ++i)\n")
    f2:write("    {\n")
    f2:write("        if (bins[i] == b) r[out_idx++] = V[i];\n")
    f2:write("    }\n")
    f2:write("    return r;\n")
    f2:write("}\n")
    -- TODO introduce a hardware accelerated version when possible
    f2:write("template<>\n")
    f2:write("INLINE " .. snipetsScalarType .." UnBin".. snipetTemplateSpecialisation .. "(\n")
    f2:write("    const " .. snipetsScalarType .. "& V,\n")
    f2:write("    const " .. snipetsScalarType .. "::IndexerType& bins\n")
    f2:write("    )\n")
    f2:write("{\n")
    f2:write("    " .. snipetsScalarType .. " r(0);\n")
    f2:write("    int max_bin = 0;\n")
    f2:write("    for (size_t i = 0; i < " .. snipetsScalarType .. "::kThreadCount; ++i) if (bins[i] > max_bin) max_bin = bins[i];\n")
    f2:write("    \n")
    f2:write("    size_t out_idx = 0;\n")
    f2:write("    for (int b = 0; b <= max_bin; ++b)\n")
    f2:write("    for (size_t i = 0; i < " .. snipetsScalarType .. "::kThreadCount; ++i)\n")
    f2:write("    {\n")
    f2:write("        if (bins[i] == b) r[i] = V[out_idx++];\n")
    f2:write("    }\n")
    f2:write("    return r;\n")
    f2:write("}\n")
end

local function GetHighestSIMD_x86_ISA()
    if gbUseSIMD_X86_AVX512 == true then
        print("x86 Max is x86_AVX_512")
        return ISAs.x86_AVX_512
    elseif gbUseSIMD_X86_AVX == true then
        print("x86 Max is x86_AVX")
        return ISAs.x86_AVX
    elseif gbUseSIMD_X86_SSE == true then
        print("x86 Max is x86_SSE")
        return ISAs.x86_SSE 
    else
        error("Unsupported ISA for x86")
    end

    return nil
end

local function UpdateMathSIMTHeadersX86(ISA, HeaderName)
    local out_file = path.join(gb_IntermediatesDir, "generated", "MathSimt", HeaderName .. ".h")
    local out_functions_file = path.join(gb_IntermediatesDir, "generated", "MathSimt", HeaderName .. "_Functions.h")
    
    if not os.isdir(path.join(gb_IntermediatesDir, "generated", "MathSimt")) then
        os.mkdir(path.join(gb_IntermediatesDir, "generated", "MathSimt"))
    end

    local ISA_Limit = GetHighestSIMD_x86_ISA()
    local f = io.open(out_file, "w")
    local f2 = io.open(out_functions_file, "w")

    f:write("#pragma once\n\n")
    f:write("#include <immintrin.h>\n")
    f:write("#include \"_Types.h\"\n")
    f:write("#include \"_TypesMSVCInterop.h\"\n")
    f:write("\n")

    f:write("namespace Math::Simt\n")
    f:write("{\n")

    f2:write("#pragma once\n\n")
    f2:write("#include <immintrin.h>\n")
    f2:write("#include \"_Types.h\"\n")
    f2:write("#include \"_TypesMSVCInterop.h\"\n")
    f2:write("\n")
    f2:write("#ifdef USE_SSE\n")
    f2:write("#include \"MathSimt/_Types_SSE_Functions.h\"\n")
    f2:write("#endif // USE_SSE\n")
    f2:write("#ifdef USE_AVX\n")
    f2:write("#include \"MathSimt/_Types_AVX_Functions.h\"\n")
    f2:write("#endif // USE_AVX\n")
    f2:write("#ifdef USE_AVX512\n")
    f2:write("#include \"MathSimt/_Types_AVX512_Functions.h\"\n")
    f2:write("#endif // USE_AVX512\n")

    f2:write("namespace Math::Simt\n")
    f2:write("{\n")

    WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.Int32, 1)
    WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.UInt32, 1)
    WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.Float, 1)
    WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.Double, 1)
    WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.Int8, 1)
    WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.UInt8, 1)

    -- ISA specific fallbacks
    if ISA == ISAs.x86_SSE then     
        f:write("#ifndef USE_AVX\n")
        f2:write("#ifndef USE_AVX\n")
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.Int32, 2)
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.UInt32, 2)
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.Float, 2)
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.Double, 2)
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.Int8, 2)
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.UInt8, 2)
        f:write("#endif // !USE_AVX\n")
        f2:write("#endif // !USE_AVX\n")

        f:write("#ifndef USE_AVX512\n")
        f2:write("#ifndef USE_AVX512\n")
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.Int32, 4)
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.UInt32, 4)
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.Float, 4)
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.Double, 4)
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.Int8, 4)
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.UInt8, 4)
        f:write("#endif // !USE_AVX512\n")
        f2:write("#endif // !USE_AVX512\n")
    elseif ISA == ISAs.x86_AVX then 

        f:write("#ifndef USE_AVX512\n")
        f2:write("#ifndef USE_AVX512\n")
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.Int32, 2)
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.UInt32, 2)
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.Float, 2)
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.Double, 2)
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.Int8, 2)
        WriteMathSIMTx86Specialization(f, f2, ISA, ISA_Limit, PrimitiveTypes.UInt8, 2)
        f:write("#endif // !USE_AVX512\n")
        f2:write("#endif // !USE_AVX512\n")
    elseif ISA == ISAs.x86_AVX_512 then
    else
        error("Unsupported ISA for x86")
    end

    f:write("}\n")
    f2:write("}\n")

    f:close()
    f2:close()
end

local function UpdateConfig()
    local f = io.open("premake-config.lua", "w")
    
    f:write("gbUseSamples = " .. tostring(gbUseSamples) .. "\n")
    f:write("gbUseSampleScenes = " .. tostring(gbUseSampleScenes) .. "\n")
    f:write("gbUseShaderc = " .. tostring(gbUseShaderc) .. "\n")
    f:write("gbUseBreakpoints = " .. tostring(gbUseBreakpoints) .. "\n")
    f:write("gbUseUnitTests = " .. tostring(gbUseUnitTests) .. "\n")
    f:write("gbUseSIMD_X86_SSE = " .. tostring(gbUseSIMD_X86_SSE) .. "\n")
    f:write("gbUseSIMD_X86_AVX = " .. tostring(gbUseSIMD_X86_AVX) .. "\n")
    f:write("gbUseSIMD_X86_AVX512 = " .. tostring(gbUseSIMD_X86_AVX512) .. "\n")
    f:write("gbWindowAPI = \"" .. gbWindowAPI .. "\"\n")
    
    f:close();
end

newaction {
    trigger = "update-sample-scenes",
    description = "update the sample scene repositories",
    execute = function ()        
        if gbUseSampleScenes == true then
            UpdateSampleScenes()
        end
    
        UpdateConfig()
    end
}

newaction {
    trigger = "update-shaderc",
    description = "update shaderc compiler",
    execute = function ()        
        if gbUseShaderc == true then
            UpdateShaderCompiler()
        end
    
        UpdateConfig()
    end
}

newaction {
    trigger = "update-simd",
    description = "update generated simd headers",
    execute = function ()
        if gbUseSIMD_X86_SSE == true then
            UpdateMathSIMTHeadersX86(ISAs.x86_SSE, "_Types_SSE")
        end

        if gbUseSIMD_X86_AVX == true then
            UpdateMathSIMTHeadersX86(ISAs.x86_AVX, "_Types_AVX")
        end

        if gbUseSIMD_X86_AVX512 == true then
            UpdateMathSIMTHeadersX86(ISAs.x86_AVX_512, "_Types_AVX512")
        end
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
        gbUseUnitTests = _OPTIONS["unit-tests"] ~= nil;
        gbUseSIMD_X86_SSE = _OPTIONS["simd-x86-sse"] ~= nil;
        gbUseSIMD_X86_AVX = _OPTIONS["simd-x86-avx"] ~= nil;
        gbUseSIMD_X86_AVX512 = _OPTIONS["simd-x86-avx512"] ~= nil;
        gbWindowAPI = _OPTIONS["window"]
        
        if gbUseSampleScenes == true then
            UpdateSampleScenes()
        end
        if gbUseShaderc == true then
            UpdateShaderCompiler()
        end
    
        if gbUseUnitTests == true then
            GenerateCatch2Config()
        end

        if gbUseSIMD_X86_SSE == true then
            UpdateMathSIMTHeadersX86(ISAs.x86_SSE, "Types_SSE.h")
        end

        if gbUseSIMD_X86_AVX == true then
            UpdateMathSIMTHeadersX86(ISAs.x86_AVX, "Types_AVX.h")
        end

        if gbUseSIMD_X86_AVX512 == true then
            UpdateMathSIMTHeadersX86(ISAs.x86_AVX_512, "Types_AVX512.h")
        end
        
        UpdateConfig()
        
        print("Setup complete")
    end
}