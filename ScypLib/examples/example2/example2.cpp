#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <ScypLib/ScypLib.h>
#include <ScypLib/EntityComponentSystem.h>

#include<random>

struct Ent
{
    sl::Vec2f pos{};
    sl::Vec2f dir{};
    float speed=5.0f;
    sl::Color c = { 1.f,1.f,1.f,1.f };
};
sl::Graphics* globalGfx = nullptr;
class DrawSystem : public sl::System
{
    void Run(float dt, sl::Scene& scene) override
    {
        globalGfx->BeginFrame();
        globalGfx->BeginView();
        scene.ForEach<Ent>([&](sl::EntityId id, Ent& e)
            {
                sl::RectF rect(e.pos, 1, 1);
                globalGfx->DrawRect(rect, e.c);
            });
        globalGfx->EndView();
        globalGfx->EndFrame();
    }
};

class UpdateSystem : public sl::System
{
    void Run(float dt, sl::Scene& scene) override
    {
        static std::random_device rd;
        static std::minstd_rand gen(rd());
        scene.ForEach<Ent>([&](sl::EntityId id, Ent& e)
            {
                std::uniform_real_distribution<float> pos_dist(-1.0f, 1.0f);
                std::uniform_real_distribution<float> dir_dist(-0.1f, 0.1f);
                std::uniform_real_distribution<float> speed_dist(-0.05f, 0.05f);
                std::uniform_real_distribution<float> color_dist(-0.05f, 0.05f);

                e.speed = std::max(0.0f, e.speed + speed_dist(gen));

                e.dir.x += dir_dist(gen);
                e.dir.y += dir_dist(gen);
                float len = std::sqrt(e.dir.x * e.dir.x + e.dir.y * e.dir.y);
                if (len > 0.0001f) {
                    e.dir.x /= len;
                    e.dir.y /= len;
                }

                e.pos += e.dir * e.speed * dt;
                
                e.c.r = std::clamp(e.c.r + color_dist(gen), 0.0f, 1.0f);
                e.c.g = std::clamp(e.c.g + color_dist(gen), 0.0f, 1.0f);
                e.c.b = std::clamp(e.c.b + color_dist(gen), 0.0f, 1.0f);
                e.c.a = std::clamp(e.c.a + color_dist(gen), 0.0f, 1.0f);

            });
    }
};

int main()
{
    sl::Window wnd("ECS example", 600, 480);
    sl::Graphics gfx(&wnd);
    globalGfx = &gfx;
    sl::EventDispatcher ed(nullptr, nullptr, &wnd);
    sl::EntityComponentSystem ecs;

    sl::Vec2f cam = { 0, 0 };
    float speed = 1.0f;

    sl::FrameTimer ft;

    ecs.CreateScene("main");
    sl::Scene* scene = ecs.GetScene("main");

    for (int y = 0; y < gfx.GetCanvasHeight(); y += 4)
    {
        for (int x = 0; x < gfx.GetCanvasWidth(); x += 4)
        {
            sl::EntityId entity = scene->CreateEntity();
            scene->AddComponent<Ent>(entity, Ent{});
            Ent& ent = scene->GetComponent<Ent>(entity);
            ent.pos = sl::Vec2f(x, y);
            ent.speed = 1;
            ent.c = sl::Colors::Cyan;
        }
    }

    scene->RegisterSystem<UpdateSystem>();
    scene->RegisterSystem<DrawSystem>();
    gfx.SetVSyncInterval(1);

    while (wnd.IsRunning())
    {
        float dt = ft.Mark();

        ecs.Run(dt);
        
        ed.PollEvents();
    }
    return 0;
}
