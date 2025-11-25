#pragma once
#include"Window.h"
#include"Mouse.h"
#include"Keyboard.h"

namespace sl
{
	class EventDispatcher
	{
	public:
		EventDispatcher(Keyboard* kbd, Mouse* mouse, Window* wnd);
		EventDispatcher() = default;
		~EventDispatcher() = default;
		void SetupCallbacks();
		void PollEvents() const;

		void SetMouse(Mouse* mouse);
		void SetKeyboard(Keyboard* kbd);
		void SetWindow(Window* wnd);

	private:
		friend struct InternalEventDispatcher;
		void KeyCallback(int key, int scancode, int action, int mods);
		void MouseMoveCallback(double xpos, double ypos);
		void MouseButtonCallback(int button, int action, int mods);
		void MouseScrollCallback(double xoffset, double yoffset);
		void MouseEnterCallback(int entered);
	public:
		Keyboard* kbd = nullptr;
		Mouse* mouse = nullptr;
		Window* wnd = nullptr;
	};
}