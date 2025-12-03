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

        glfwGetWindowPos(internalWindow->glfwWindow, &x, &y);

        this->width = width;
        this->height = height;
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

    void Window::Resize(int width, int height)
    {
        this->width = width;
        this->height = height;
        glfwSetWindowSize(internalWindow->glfwWindow, width, height);
    }

    void Window::SetPosition(int x, int y)
    {
        this->x = x;
        this->y = y;
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
    void* Window::GetWindowBackend() const
    {
        return internalWindow->glfwWindow;
    }
}
