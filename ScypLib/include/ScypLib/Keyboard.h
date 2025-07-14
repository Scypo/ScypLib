#pragma once
#include <GLFW/glfw3.h>
#include <queue>
#include <bitset>

namespace sl
{
    class Keyboard
    {
        friend class EventDispatcher;
    public:
        Keyboard() = default;
        Keyboard(const Keyboard&) = delete;
        Keyboard& operator=(const Keyboard&) = delete;

        bool KeyIsPressed(int key) const;
        bool KeyIsPressed(char character) const;
        void Flush();
        bool IsEmpty() const;
        void ProcessKeyState(int key, int action);
    private:
        int CharToKey(char character) const;
    private:
        static constexpr unsigned int nKeys = GLFW_KEY_LAST + 1;
        std::bitset<nKeys> keystates{};
        bool empty = true;
    };
}
