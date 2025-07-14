#include<cassert>

#include"ScypLib/EventDispatcher.h"

namespace sl
{
    EventDispatcher::EventDispatcher(GLFWwindow* window)
    {
        SetupCallbacks(window);
    }

    void EventDispatcher::SetupCallbacks(GLFWwindow* window)
    {
        glfwSetWindowUserPointer(window, this);

        glfwSetKeyCallback(window, KeyCallback);
        glfwSetCursorPosCallback(window, MouseMoveCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
        glfwSetScrollCallback(window, MouseScrollCallback);
        glfwSetCursorEnterCallback(window, MouseEnterCallback);
        glfwSetWindowCloseCallback(window, WindowCloseCallback);
        glfwSetFramebufferSizeCallback(window, SetFrameBufferSizeCallback);
        glfwSetWindowMaximizeCallback(window, SetWindowMaximizeCallback);
        glfwSetWindowPosCallback(window, SetWindowPosCallback);
    }

    void EventDispatcher::PollEvents() const
    {
        mouse->scrollOffset = { 0,0 };
        mouse->empty = true;
        kbd->empty = true;
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
    {}

    void EventDispatcher::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
        if (dispatcher->kbd)
        {
            dispatcher->kbd->ProcessKeyState(key, action);
            dispatcher->kbd->empty = false;
        }
    }

    void EventDispatcher::MouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
    {
        EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
        if (dispatcher->mouse)
        {
            dispatcher->mouse->OnMouseMove(static_cast<float>(xpos), static_cast<float>(ypos));
            dispatcher->mouse->empty = false;
        }
    }

    void EventDispatcher::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));

        if (dispatcher->mouse)
        {
            dispatcher->mouse->empty = false;

            if (button == GLFW_MOUSE_BUTTON_LEFT)
            {
                if (action == GLFW_PRESS)
                {
                    dispatcher->mouse->OnLeftPressed();
                }
                else if (action == GLFW_RELEASE)
                {
                    dispatcher->mouse->OnLeftReleased();
                }
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT)
            {
                if (action == GLFW_PRESS)
                {
                    dispatcher->mouse->OnRightPressed();
                }
                else if (action == GLFW_RELEASE)
                {
                    dispatcher->mouse->OnRightReleased();
                }
            }
            else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
            {
                if (action == GLFW_PRESS)
                {
                    dispatcher->mouse->OnScrollPressed();
                }
                else if (action == GLFW_RELEASE)
                {
                    dispatcher->mouse->OnScrollReleased();
                }
            }
        }
    }

    void EventDispatcher::MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
        if (dispatcher->mouse)
        {
            dispatcher->mouse->OnWheelScroll(float(xoffset), float(yoffset));
            dispatcher->mouse->empty = false;
        }
    }

    void EventDispatcher::MouseEnterCallback(GLFWwindow* window, int entered)
    {
        EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
        if (dispatcher->mouse)
        {
            dispatcher->mouse->SetIsInWindow(entered != 0);
            dispatcher->mouse->empty = false;
        }
    }

    void EventDispatcher::WindowCloseCallback(GLFWwindow* window)
    {
        EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
        if (dispatcher->wnd) dispatcher->wnd->Close();
    }

    void EventDispatcher::SetFrameBufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
        if (dispatcher->wnd) dispatcher->wnd->Resize(width, height);
    }

    void EventDispatcher::SetWindowMaximizeCallback(GLFWwindow* window, int isMaximized)
    {
        EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
        if (dispatcher->wnd) dispatcher->wnd->ToggleMaximize(isMaximized == GLFW_TRUE);
    }

    void EventDispatcher::SetWindowPosCallback(GLFWwindow* window, int x, int y)
    {
        EventDispatcher* dispatcher = static_cast<EventDispatcher*>(glfwGetWindowUserPointer(window));
        if (dispatcher->wnd) dispatcher->wnd->SetPosition(x, y);
    }
}