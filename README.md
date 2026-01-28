# ScypLib

**ScypLib** is a fast and lightweight 2D graphics framework written in modern C++20 using OpenGL. It provides a powerful and flexible rendering backend with batching, texture/shader/font management, custom shader support, and basic audio integration — all packed as a simple static library.

---

## Features

-  Efficient OpenGL based renderer
-  Batched 2D and simple 3D quad rendering 
-  Texture and sprite drawing with transform, color tinting, and UV mapping
-  Custom shader pipeline via uniform and shader storage buffers
-  Font rendering with stb_truetype
-  Simple audio playback using miniaudio
-  Window and input handling via GLFW
-  Entity component system
---
##  Using ScypLib in Your Project

### 1. Add Include Directories
Go to your project settings and add the following paths under **C/C++ → General → Additional Include Directories**:

- `<Path to ScypLib>/include`
- `<Path to GLFW>/include`

---

### 2. Add Library Directories
Go to your project settings and add the following paths under **Linker → General → Additional Library Directories**:

- `<Path to ScypLib>/lib/x64/Release`
- `<Path to GLFW>/GLFW/lib-vc2022`
   
### 3. Link Libraries:
   Link against these libraries:
   - ScypLib.lib
   - glfw3.lib
   - opengl32.lib

### 4. Set language standard to c++20.

### 5. Include Headers and Build:
   Include ScypLib and GLFW headers in your source files and build.
---
##  Dependencies

ScypLib depends on the following open-source libraries:

| Library       | Usage                        | License                        |
|---------------|------------------------------|--------------------------------|
| [GLFW](https://www.glfw.org/)       | Window creation & input         | zlib/libpng license            |
| [glad](https://glad.dav1d.de/)| OpenGL function loading         | MIT                            |
| [stb_image](https://github.com/nothings/stb)     | Image loading (PNG, JPG, etc)   | Public Domain / MIT            |
| [stb_truetype](https://github.com/nothings/stb)  | Font rasterization              | Public Domain / MIT            |
| [miniaudio](https://miniaud.io/)   | Audio playback                   | Public Domain / MIT            |

---
## Shaders must follow this pattern.

### Vertex Shader (`example.vert`)

```glsl
#version 330 core

layout(location = 0) in vec3 aPosition;

layout(location = 1) in vec4 iUV;
layout(location = 2) in vec4 iColor;
layout(location = 3) in mat4 iModel;		

layout(std140) uniform CameraBuffer 
{
    mat4 view;
    mat4 projection;
};

out vec2 vTexCoord;	
out vec4 vColorTint;

void main()
{
    gl_Position = projection * view * iModel * vec4(aPosition, 1.0);
	int corner = gl_VertexID & 3;
	vec2 uv;
   
	uv.x = mix(iUV.x, iUV.y, aPosition.x);
	uv.y = mix(iUV.w, iUV.z, aPosition.y);
	vTexCoord = uv;
    
    vColorTint = iColor;
}
```
### Fragment Shader (`example.frag`)

```glsl
#version 330 core

in vec2 vTexCoord;
in vec4 vColorTint;

out vec4 FragColor;
uniform sampler2D uTexture;

void main()
{
    vec4 texColor = texture(uTexture, vTexCoord);
    vec4 finalColor = texColor * vColorTint;

    if (finalColor.a < 0.1) discard;

    FragColor = finalColor;
}
```
