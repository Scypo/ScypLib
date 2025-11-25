#pragma once
#include <queue>
#include <bitset>

namespace sl
{
    enum class Key 
    {
        Unknown = 0,
    
        Space, Apostrophe, Comma, Minus, Period, Slash,
        D0, D1, D2, D3, D4, D5, D6, D7, D8, D9,
        Semicolon, Equal,
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        LeftBracket, Backslash, RightBracket, GraveAccent, World1, World2,
    
        Escape, Enter, Tab, Backspace, Insert, Delete,
        Right, Left, Down, Up, PageUp, PageDown, Home, End,
    
        CapsLock, ScrollLock, NumLock,
    
        PrintScreen, Pause,
    
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24, F25,
    
        Kp0, Kp1, Kp2, Kp3, Kp4, Kp5, Kp6, Kp7, Kp8, Kp9,
        KpDecimal, KpDivide, KpMultiply, KpSubtract, KpAdd, KpEnter, KpEqual,
    
        LeftShift, LeftControl, LeftAlt, LeftSuper,
        RightShift, RightControl, RightAlt, RightSuper,
    
        Menu,
        Last
    };
    class Keyboard
    {
        friend class EventDispatcher;
    public:
        Keyboard() = default;
        Keyboard(const Keyboard&) = delete;
        Keyboard& operator=(const Keyboard&) = delete;

        bool KeyIsPressed(Key key) const;
        bool KeyIsPressed(char character) const;
        void Flush();
        bool IsEmpty() const;
    private:
        void ProcessKeyState(int key, int action);
        int CharToKey(char character) const;
        Key GLFWToKey(int glfwKey)const;
    private:
        static constexpr unsigned int nKeys = int(Key::Last) + 1;
        std::bitset<nKeys> keystates{};
        bool empty = true;
    };
}
