# 🎮 ScypLib

**ScypLib** is a fast and lightweight 2D graphics framework written in modern C++20 using OpenGL. It provides a powerful and flexible rendering backend with batching, texture/shader/font management, custom shader support, and basic audio integration — all packed as a simple static library.

---

## 🚀 Features

- 🔥 Efficient OpenGL 4.5-based renderer
- 🧱 Batched 2D rendering
- 🎨 Texture and sprite drawing with transform, color tinting, and UV mapping
- 📜 Custom shader pipeline via uniform and shader storage buffers
- 🖼️ Font rendering with stb_truetype
- 🔉 Simple audio playback using miniaudio
- 🗔 Window and input handling via GLFW
- 🤖 Entity component system
---
## 🔧 Using ScypLib in Your Project

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
## 🧰 Dependencies

ScypLib depends on the following open-source libraries:

| Library       | Usage                        | License                        |
|---------------|------------------------------|--------------------------------|
| [GLFW](https://www.glfw.org/)       | Window creation & input         | zlib/libpng license            |
| [glad](https://glad.dav1d.de/)| OpenGL function loading         | MIT                            |
| [stb_image](https://github.com/nothings/stb)     | Image loading (PNG, JPG, etc)   | Public Domain / MIT            |
| [stb_truetype](https://github.com/nothings/stb)  | Font rasterization              | Public Domain / MIT            |
| [miniaudio](https://miniaud.io/)   | Audio playback                   | Public Domain / MIT            |

---

## 🧪 Shader Structure

ScypLib supports **custom GLSL shaders** using SSBO/UBO layouts. To use them, your shader must follow this layout:

### Vertex Shader (`example.vert`)

```glsl
#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in float aInstanceIndex;

struct InstanceData 
{
    mat4 transform;
    vec4 colorTint;
    float textureSlot;
    float padding[3]; // padding to align std430
};

layout(std140, binding = 0) uniform CameraBuffer 
{
    mat4 view;
    mat4 projection;
};

layout(std430, binding = 1) readonly buffer instanceData
{
    InstanceData instances[];
};

out vec2 vTexCoord;
out float vTexSlot;
out vec4 vColorTint;

void main()
{
    InstanceData data = instances[int(aInstanceIndex)];
    gl_Position = projection * view * data.transform * vec4(aPosition, 1.0);

    vTexCoord = aTexCoord;
    vTexSlot = data.textureSlot;
    vColorTint = data.colorTint;
}
```
### Fragment Shader (`example.frag`)

```glsl
#version 450 core

in vec2 vTexCoord;
in float vTexSlot;
in vec4 vColorTint;

out vec4 FragColor;
uniform sampler2D uTextures[32];

void main()
{
    int slot = int(vTexSlot);
    vec4 texColor = texture(uTextures[slot], vTexCoord);
    vec4 finalColor = texColor * vColorTint;

    if (finalColor.a < 0.1) discard;

    FragColor = finalColor;
}
```
