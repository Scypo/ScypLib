#pragma once
#include <string>
#include<memory>
#include"Vec2.h"

namespace sl
{
    enum class CursorMode
    {
        Normal,
        Hidden,
        Disabled
    };
    class Window
    {
        friend class EventDispatcher;
        friend class Graphics;
    public:
        Window(const char* title, int width, int height, int resizable = 1, int decorated = 1, int visible = 1);
        ~Window();
        void ToggleMaximize(bool isMaximized);
        Vec2i GetSize() const;
        Vec2i GetPos() const;
        int GetWidth() const;
        int GetHeight() const;
        void Close() { isRunning = false; }
        bool IsRunning() const { return isRunning; }
        void Resize(int width, int height);
        void SetPosition(int x, int y);
        void Show();
        void Hide();
        void ToggleFullscreen();
        void SwitchCursoreMode(CursorMode mode);
    private:
        void* GetWindowBackend()const;
    private:
        bool isFullscreen = false;
        bool isRunning = false;
        struct InternalWindow;
        std::unique_ptr<InternalWindow> internalWindow = nullptr;
    };
}