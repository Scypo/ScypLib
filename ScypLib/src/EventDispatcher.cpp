#include<cassert>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include"ScypLib/EventDispatcher.h"

namespace sl
{
    struct InternalEventDispatcher
    {
    public:
        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
            dispatcher->KeyCallback(key, scancode, action, mods);
        }
        static void MouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
        {
            EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
            dispatcher->MouseMoveCallback(xpos, ypos);
        }
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
        {
            EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
            dispatcher->MouseButtonCallback(button, action, mods);
        }
        static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
        {
            EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
            dispatcher->MouseScrollCallback(xoffset, yoffset);
        }

        static void MouseEnterCallback(GLFWwindow* window, int entered)
        {
            EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
            dispatcher->MouseEnterCallback(entered);
        }
        static void WindowCloseCallback(GLFWwindow* window)
        {
            EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
            if (dispatcher->wnd) dispatcher->wnd->Close();
        }
        static void SetFrameBufferSizeCallback(GLFWwindow* window, int width, int height)
        {
            EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
            if (dispatcher->wnd) dispatcher->wnd->Resize(width, height);
        }
        static void SetWindowMaximizeCallback(GLFWwindow* window, int isMaximized)
        {
            EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
            if (dispatcher->wnd) dispatcher->wnd->ToggleMaximize(isMaximized == GLFW_TRUE);
        }
        static void SetWindowPosCallback(GLFWwindow* window, int x, int y)
        {
            EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
            if (dispatcher->wnd) dispatcher->wnd->SetPosition(x, y);
        }
        static void SetupCallbacks(GLFWwindow* window, EventDispatcher* ed)
        {
            glfwSetWindowUserPointer(window, ed);

            if (ed->kbd) glfwSetKeyCallback(window, KeyCallback);
            if (ed->mouse)
            {
                glfwSetCursorPosCallback(window, MouseMoveCallback);
                glfwSetMouseButtonCallback(window, MouseButtonCallback);
                glfwSetScrollCallback(window, MouseScrollCallback);
                glfwSetCursorEnterCallback(window, MouseEnterCallback);
            }

            if (ed->wnd)
            {
                glfwSetWindowCloseCallback(window, WindowCloseCallback);
                glfwSetFramebufferSizeCallback(window, SetFrameBufferSizeCallback);
                glfwSetWindowMaximizeCallback(window, SetWindowMaximizeCallback);
                glfwSetWindowPosCallback(window, SetWindowPosCallback);
            }
        }
    };

    void EventDispatcher::SetupCallbacks()
    {
        InternalEventDispatcher::SetupCallbacks(reinterpret_cast<GLFWwindow*>(wnd->GetWindowBackend()), this);
    }

    void EventDispatcher::PollEvents() const
    {
        if (mouse)
        {
            mouse->scrollOffset = { 0,0 };
            mouse->empty = true;
        }
        if(kbd) kbd->empty = true;
        glfwPollEvents();
    }

    void EventDispatcher::SetMouse(Mouse* mouse)
    {
        this->mouse = mouse;
    }

    void EventDispatcher::SetKeyboard(Keyboard* kbd)
    {
        this->kbd = kbd;
    }

    void EventDispatcher::SetWindow(Window* wnd)
    {
        this->wnd = wnd;
    }

    EventDispatcher::EventDispatcher(Keyboard* kbd, Mouse* mouse, Window* wnd)
        : kbd(kbd), mouse(mouse), wnd(wnd)
    {
        SetupCallbacks();
    }
    void EventDispatcher::KeyCallback(int key, int scancode, int action, int mods)
    {
        if (kbd)
        {
            kbd->ProcessKeyState(key, action);
            kbd->empty = false;
        }
    }
    void EventDispatcher::MouseMoveCallback(double xpos, double ypos)
    {
        if (mouse)
        {
            glfwGetCursorPos(reinterpret_cast<GLFWwindow*>(wnd->GetWindowBackend()), &xpos, &ypos);
            mouse->OnMouseMove(float(xpos), float(ypos));
            mouse->empty = false;
        }
    }
    void EventDispatcher::MouseButtonCallback(int button, int action, int mods)
    {
        if (mouse)
        {
            mouse->empty = false;

            if (button == GLFW_MOUSE_BUTTON_LEFT)
            {
                if (action == GLFW_PRESS)
                {
                    mouse->OnLeftPressed();
                }
                else if (action == GLFW_RELEASE)
                {
                    mouse->OnLeftReleased();
                }
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT)
            {
                if (action == GLFW_PRESS)
                {
                    mouse->OnRightPressed();
                }
                else if (action == GLFW_RELEASE)
                {
                    mouse->OnRightReleased();
                }
            }
            else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
            {
                if (action == GLFW_PRESS)
                {
                    mouse->OnScrollPressed();
                }
                else if (action == GLFW_RELEASE)
                {
                    mouse->OnScrollReleased();
                }
            }
        }
    }
    void EventDispatcher::MouseScrollCallback(double xoffset, double yoffset)
    {
        if (mouse)
        {
            mouse->OnWheelScroll(float(xoffset), float(yoffset));
            mouse->empty = false;
        }
    }
    void EventDispatcher::MouseEnterCallback(int entered)
    {
        if (mouse)
        {
            mouse->SetIsInWindow(entered != 0);
            mouse->empty = false;
        }
    }
}