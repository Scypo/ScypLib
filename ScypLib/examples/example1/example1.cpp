#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <ScypLib/ScypLib.h>

int main()
{
    // Initialize window and systems
    sl::Window wnd("Shader Example", 600, 480);
    sl::Keyboard kbd;
    sl::Mouse mouse;
    sl::Graphics gfx(&wnd);
    sl::Audio audio;
    sl::EventDispatcher ed;

    // Setup input callbacks
    ed.SetWindow(&wnd);
    ed.SetKeyboard(&kbd);
    ed.SetMouse(&mouse);
    ed.SetupCallbacks(wnd.GetGLFWWindow());

    // Load resources
    sl::Texture* texture = gfx.LoadTexture("knight.png");
    sl::Sound* sound = audio.LoadSound("hit.wav");
    sl::Shader* shader = gfx.LoadShader("plasma.vert", "plasma.frag", true);

    // Setup sprite
    sl::Sprite sprite(texture);
    sprite.SetOrigin({ 20.0f, 20.0f });
    sprite.SetPos({ 0, 0 });
    sprite.SetSize({ 80, 80 });

    sl::Vec2f cam = { 0, 0 };
    float zoom = 1.0f;
    float speed = 1.0f;

    sl::FrameTimer ft;

    while (wnd.IsRunning())
    {
        float dt = ft.Mark();

        // Input handling
        if (kbd.KeyIsPressed('W')) cam.y += speed;
        if (kbd.KeyIsPressed('S')) cam.y -= speed;
        if (kbd.KeyIsPressed('A')) cam.x += speed;
        if (kbd.KeyIsPressed('D')) cam.x -= speed;

        float scroll = mouse.GetScrollOffsetY();
        if (scroll > 0) zoom += 0.05f;
        if (scroll < 0) zoom = std::max(0.05f, zoom - 0.05f);

        if (mouse.ScrollIsPressed())
            audio.PlaySound(sound);

        // Update shader uniform
        shader->SetUniform1f("uTime", glfwGetTime() * 5);

        // Rendering
        gfx.BeginFrame();
        gfx.BeginView(cam, zoom);

        gfx.SetDrawLayer(0);

        // Draw sprite with shader effect
        gfx.DrawTexture(sprite.GetPos(), sprite.GetSize(), texture, shader);

        gfx.EndView();
        gfx.EndFrame();

        ed.PollEvents();
    }

    // Cleanup
    gfx.UnloadTexture(texture); 
    gfx.UnloadShader(shader);
    audio.UnloadSound(sound);

    return 0;
}
