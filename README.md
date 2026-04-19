# GraphicsBox
Graphics Box is a set of tools regarding graphics programming for prototyping graphical features. GraphicsBox is build 
on C++ 20 and uses OpenGL as the Graphics API. 

# Setup

- Get submodules using: ```git submodule update --init --recursive```
- Install or get an executable of premake 5. For more information, visit https://premake.github.io/download/. (Note: 
Premake 5 is still under development, on most recent version of Premake, generating the solution can generate 
deprecated flag usage warnings depending on your version of Premake.)

## Windows (using Visual studio) 

You will need Visual Studio, or any IDE that supports .sln projects (like JetBrains Rider).

- run: ```<premake-executable> vs<version>```. Graphics Box works well on Visual Studio 2022 solutions for example.
- (optional) to generate the samples run: ```<premake-executable> vs<version> <...> --samples```
- (optional) to download samples assets and test scenes: ```<premake-executable> vs<version> <...> --sample-scenes```
- (optional) to enable assertions throwing breakpoints: ```<premake-executable> vs<version> <...> --breakpoints```

You can now open the generated ```GraphicsBox.sln``` file generated and build.

## Linux

- Get the GLFW and GLEW dependencies from your package manager. If using APT, run this command: 
  ```sudo apt install libglfw3 libglfw3-dev libglew-dev```

### Using makefiles

- run ```<premake-executable> gmake```. Graphics Box works well on Visual Studio 2022 solutions for example.
- (optional) to generate the samples run: ```<premake-executable> gmake <...> --samples```
- (optional) to download samples assets and test scenes: ```<premake-executable> gmake <...> --sample-scenes```
- (optional) to enable assertions throwing breakpoints: ```<premake-executable> gmake <...> --breakpoints```
- To build run: ```Make <project> config=<config>```
  - config - "Debug", "Development" or "Release"