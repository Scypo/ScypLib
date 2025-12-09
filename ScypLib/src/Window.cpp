#include "ScypLib/Window.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace sl
{
    struct Window::InternalWindow
    {
        GLFWwindow* glfwWindow = nullptr;
    };

    Window::Window(const char* title, int width, int height,
        int resizable, int decorated, int visible)
    {
        if (!glfwInit()) throw std::runtime_error("Failed to initialize GLFW");

        glfwWindowHint(GLFW_RESIZABLE, resizable);
        glfwWindowHint(GLFW_DECORATED, decorated);
        glfwWindowHint(GLFW_VISIBLE, visible);

        internalWindow = std::make_unique<InternalWindow>();
        internalWindow->glfwWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

        if (!internalWindow->glfwWindow)
        {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwMakeContextCurrent(internalWindow->glfwWindow);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) throw std::runtime_error("Failed to initialize GLAD");
        isRunning = true;
    }

    Window::~Window()
    {
        if (internalWindow)
        {
            if (internalWindow->glfwWindow) glfwDestroyWindow(internalWindow->glfwWindow);
            glfwTerminate();
        }
    }

    void Window::ToggleMaximize(bool isMaximized)
    {
        if (isMaximized)
            glfwMaximizeWindow(internalWindow->glfwWindow);
        else
            glfwRestoreWindow(internalWindow->glfwWindow);
    }

    Vec2i Window::GetSize() const
    {
        Vec2i size{};
        glfwGetWindowSize(internalWindow->glfwWindow, &size.x, &size.y);
        return size;
    }

    Vec2i Window::GetPos() const
    {
        Vec2i pos{};
        glfwGetWindowPos(internalWindow->glfwWindow, &pos.x, &pos.y);
        return pos;
    }

    int Window::GetWidth() const
    {
        return GetSize().x;
    }

    int Window::GetHeight() const
    {
        return GetSize().y;
    }

    void Window::Resize(int width, int height)
    {
        glfwSetWindowSize(internalWindow->glfwWindow, width, height);
    }

    void Window::SetPosition(int x, int y)
    {
        glfwSetWindowPos(internalWindow->glfwWindow, x, y);
    }

    void Window::Show()
    {
        glfwShowWindow(internalWindow->glfwWindow);
    }

    void Window::Hide()
    {
        glfwHideWindow(internalWindow->glfwWindow);
    }
    void Window::ToggleFullscreen()
    {
        static int x, y, w, h;
        GLFWwindow* window = internalWindow->glfwWindow;
        if (!isFullscreen)
        {
            glfwGetWindowPos(window, &x, &y);
            glfwGetWindowSize(window, &w, &h);

            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);

            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        }
        else
        {
            glfwSetWindowMonitor(window, nullptr, x, y, w, h, 0);
        }
        isFullscreen = !isFullscreen;
    }
    void Window::SwitchCursoreMode(CursorMode mode)
    {
        GLFWwindow* window = internalWindow->glfwWindow;
        switch (mode)
        {
        case sl::CursorMode::Normal:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            break;
        case sl::CursorMode::Hidden:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            break;
        case sl::CursorMode::Disabled:
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            break;
        default:
            break;
        }
    }
    void* Window::GetWindowBackend() const
    {
        return internalWindow->glfwWindow;
    }
}
