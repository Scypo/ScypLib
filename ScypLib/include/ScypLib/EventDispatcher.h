#pragma once
#include"Window.h"
#include"Mouse.h"
#include"Keyboard.h"

namespace sl
{
	class EventDispatcher
	{
	public:
		EventDispatcher(GLFWwindow* window);
		EventDispatcher(Keyboard* kbd, Mouse* mouse, Window* wnd);
		EventDispatcher() = default;
		~EventDispatcher() = default;
		void SetupCallbacks(GLFWwindow* window);
		void PollEvents() const;

		void SetMouse(Mouse* mouse);
		void SetKeyboard(Keyboard* kbd);
		void SetWindow(Window* wnd);

		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
		static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
		static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static void MouseEnterCallback(GLFWwindow* window, int entered);
		static void WindowCloseCallback(GLFWwindow* window);
		static void SetFrameBufferSizeCallback(GLFWwindow* window, int width, int height);
		static void SetWindowMaximizeCallback(GLFWwindow* window, int isMaximized);
		static void SetWindowPosCallback(GLFWwindow* window, int x, int y);
	public:
		Keyboard* kbd = nullptr;
		Mouse* mouse = nullptr;
		Window* wnd = nullptr;
	};
}