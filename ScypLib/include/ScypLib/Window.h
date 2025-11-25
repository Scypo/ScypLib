#pragma once
#include <stdexcept>
#include <string>

namespace sl
{
    class Window
    {
        friend class EventDispatcher;
        friend class Graphics;
    public:
        Window(const char* title, int width, int height, int resizable = 1, int decorated = 1, int visible = 1);
        ~Window();
        void ToggleMaximize(bool isMaximized);
        int GetWidth() const { return width; }
        int GetHeight() const { return height; }
        void Close() { isRunning = false; }
        bool IsRunning() const { return isRunning; }
        void Resize(int width, int height);
        void SetPosition(int x, int y);
        void Show();
        void Hide();
    private:
        void* GetWindowBackend()const;
    private:
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;

        bool isRunning = false;
        struct InternalWindow;
        InternalWindow* internalWindow = nullptr;
    };
}