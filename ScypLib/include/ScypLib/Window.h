#pragma once
#include <stdexcept>
#include <string>

#include<GL/glew.h>
#include<GLFW/glfw3.h>

namespace sl
{
    class Window
    {
        friend class EventDispatcher;
    public:
        Window(const char* title, int width, int height, int resizable = 1, int decorated = 1, int visible = 1)
            : width(width), height(height)
        {
            if (!glfwInit())
            {
                throw std::runtime_error("Failed to initialize GLFW");
            }

            glfwWindowHint(GLFW_RESIZABLE, resizable);
            glfwWindowHint(GLFW_DECORATED, decorated);
            glfwWindowHint(GLFW_VISIBLE, visible);

            glfwWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
            if (!glfwWindow)
            {
                glfwTerminate();
                throw std::runtime_error("Failed to create GLFW window");
            }

            glfwMakeContextCurrent(glfwWindow);

            if (glewInit() != GLEW_OK)
            {
                throw std::runtime_error("Failed to initialize GLEW");
            }

            glfwGetWindowPos(glfwWindow, &x, &y);

            isRunning = true;
            Resize(width, height);
        }

        ~Window()
        {
            if (glfwWindow)
            {
                glfwDestroyWindow(glfwWindow);
                glfwTerminate();
            }
        }

        void ToggleMaximize(bool isMaximized)
        {
            if (isMaximized)
            {
                glfwMaximizeWindow(glfwWindow);
            }
            else
            {
                glfwRestoreWindow(glfwWindow);
            }
        }

        GLFWwindow* GetGLFWWindow() const { return glfwWindow; }
        int GetWidth() const { return width; }
        int GetHeight() const { return height; }
        void Close() { isRunning = false; }
        bool IsRunning() const { return isRunning; }
        void Resize(int width, int height)
        {
            this->width = width;
            this->height = height;
            glfwSetWindowSize(glfwWindow, width, height);
        }
        void SetPosition(int x, int y)
        {
            this->x = x;
            this->y = y;
            glfwSetWindowPos(glfwWindow, x, y);
        }
        void Show()
        {
            glfwShowWindow(glfwWindow);
        }
        void Hide()
        {
            glfwHideWindow(glfwWindow);
        }
    private:
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;

        bool isRunning = false;
        GLFWwindow* glfwWindow = nullptr;
    };
}