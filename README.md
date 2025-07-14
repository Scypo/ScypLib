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

---
## 🔧 Using ScypLib in Your Project

1. Add Include Directory:
   Add the path to ScypLib headers:
   <Path to ScypLib>/include

2. Add Library Directory:
   Add the path to ScypLib libraries:
   <Path to ScypLib>/lib/x64/Release

3. Link Libraries:
   Link against these libraries:
   - ScypLib.lib
   - glfw3.lib
   - glew32s.lib
   - opengl32.lib

4. Define Preprocessor Macro:
   Add the definition:
   GLEW_STATIC

5. Set language standard to c++20.

6. Include Headers and Build:
   Include ScypLib headers in your source files and build.
---
## 🧰 Dependencies

ScypLib depends on the following open-source libraries:

| Library       | Usage                        | License                        |
|---------------|------------------------------|--------------------------------|
| [GLFW](https://www.glfw.org/)       | Window creation & input         | zlib/libpng license            |
| [GLEW](http://glew.sourceforge.net/)| OpenGL function loading         | MIT                            |
| [stb_image](https://github.com/nothings/stb)     | Image loading (PNG, JPG, etc)   | Public Domain / MIT            |
| [stb_truetype](https://github.com/nothings/stb)  | Font rasterization              | Public Domain / MIT            |
| [miniaudio](https://miniaud.io/)   | Audio playback                   | Public Domain / MIT            |
| [glm](https://github.com/g-truc/glm)| Math library (vec/mat/quats)     | MIT                            |

> ScypLib includes these libraries.

---
## 🕹️ Example Program
```c++

#include"ScypLib/ScypLib.h"

int main()
{
    sl::Window wnd("title", 600, 480);
    sl::Keyboard kbd;
    sl::Mouse mouse;
    sl::Graphics gfx(&wnd);
    sl::Audio audio;
    sl::EventDispatcher ed;
    ed.SetWindow(&wnd);
    ed.SetKeyboard(&kbd);
    ed.SetMouse(&mouse);
    ed.SetupCallbacks(wnd.GetGLFWWindow());

    sl::Texture* texture = gfx.LoadTexture("Assets/Images/texture.png");
    sl::Font* fnt = gfx.LoadFont("Assets/Fonts/font.ttf", ' ', '~');
    sl::Sound* sound = audio.LoadSound("Assets/Sounds/sound.wav");
	gfx.SetDefaultFont(fnt);

    sl::Vec2f cam = { 0,0 };
    float zoom = 1.0f;

    while (wnd.IsRunning())
    {        
        gfx.SetCanvasSize(Vec2f(float(wnd.GetWidth()), float(wnd.GetHeight())));
        if (kbd.KeyIsPressed('W'))
        {
            audio.PlaySound(sound);
        }
        if (mouse.GetScrollOffsetY() < 0)
        {
            zoom -= 0.05f;
            if (zoom < 0) zoom = 0.0f;
        }

        gfx.BeginFrame();
        gfx.BeginView();
        gfx.SetDrawLayer(0);

        gfx.DrawRect(sl::RectF({ 200.0f, 200.0f }, 50.5, 50.5), sl::Colors::Blue);
        gfx.DrawLine(20, 0, 100, 70, 1.0f, sl::Colors::Red);
        gfx.DrawTexture(400, 300, texture);
        gfx.EndView();
        gfx.EndFrame();
        ed.PollEvents();
    }
    gfx.UnloadTexture(texture);
    audio.UnloadSound(sound);
    gfx.UnloadFont(fnt);

    return 0;
}
```

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
