# GraphicsBox
Graphics Box is a set of tools regarding graphics programming for prototyping graphical features. GraphicsBox is build 
on C++ 20 and uses OpenGL as the Graphics API. 

# Setup

1. Get submodules using: ```git submodule update --init --recursive```
2. Install or get an executable of premake 5. For more information, visit https://premake.github.io/download/. (Note: 
Premake 5 is still under development, on most recent version of Premake, generating the solution can generate 
deprecated flag usage warnings depending on your version of Premake.)
3. run: ```<premake-executable> setup [option(s)]```
   - ```--samples``` to generate the samples projects
   - ```--sample-scenes``` to generate the samples projects
   - ```--breakpoints``` to generate the samples projects
   - ```--shaderc``` when compiling shader from source code, will use shaderc to shader source code compile to Spir-V. 
   This will download and compile the shaderc shader compiler. This requires a toolset to be specified using ```--dependencies-toolset=...```.
   - ```--dependencies-toolset=<toolset>``` toolset used to compile third party library during setup phase.
   - ```--unit-tests``` to generate the unit tests projects
   - ```simd-x86-sse``` to enable SSE SIMD instruction generation for x86/x64
   - ```simd-x86-avx``` to enable SSE SIMD instruction generation for x86/x64
   - ```simd-x86-avx512``` to enable SSE SIMD instruction generation for x86/x64

## Windows (using Visual studio) 

You will need Visual Studio, or any IDE that supports .sln projects (like JetBrains Rider).

4. run: ```<premake-executable> vs<version>```. Graphics Box works well on Visual Studio 2022 solutions for example.
You can now open the generated ```GraphicsBox.sln``` file generated and build.

## Linux

4. Get the GLFW and GLEW dependencies from your package manager. If using APT, run this command: 
  ```sudo apt install libglfw3 libglfw3-dev libglew-dev```

### Using makefiles

5. run ```<premake-executable> gmake```. 
6. To build run: ```Make <project> config=<config>```
   - config - "Debug", "Development" or "Release"

### Using CMake (not tested)

5. run ```<premake-executable> cmake``` to generate the CMake project.