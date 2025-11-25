#include"ScypLib/Keyboard.h"
#include<array>
#include<GLFW/glfw3.h>

namespace sl
{
    int Keyboard::CharToKey(char character) const
    {
        if (character >= 'a' && character <= 'z')
        {
            return int(Key::A) + (character - 'a');
        }
        else if (character >= 'A' && character <= 'Z')
        {
            return int(Key::A) + (character - 'A');
        }
        else if (character >= '0' && character <= '9')
        {
            return int(Key::D0) + (character - '0');
        }

        switch (character)
        {
        case ' ': return int(Key::Space);
        case '\n':
        case '\r': return int(Key::Enter);
        case '\t': return int(Key::Tab);
        case '\b': return int(Key::Backspace);

        case '!': return int(Key::D1);
        case '@': return int(Key::D2);
        case '#': return int(Key::D3);
        case '$': return int(Key::D4);
        case '%': return int(Key::D5);
        case '^': return int(Key::D6);
        case '&': return int(Key::D7);
        case '*': return int(Key::D8);
        case '(': return int(Key::D9);
        case ')': return int(Key::D0);

        case '-': return int(Key::Minus);
        case '=': return int(Key::Equal);
        case '[': return int(Key::LeftBracket);
        case ']': return int(Key::RightBracket);
        case '\\': return int(Key::Backslash);
        case ';': return int(Key::Semicolon);
        case '\'': return int(Key::Apostrophe);
        case ',': return int(Key::Comma);
        case '.': return int(Key::Period);
        case '/': return int(Key::Slash);
        case '`': return int(Key::GraveAccent);

        default: return int(Key::Unknown);
        }
    }

    bool Keyboard::KeyIsPressed(Key key) const
    {
        return keystates[int(key)];
    }

    bool Keyboard::KeyIsPressed(char character) const
    {
        int key = CharToKey(character);
        return (key != int(Key::Unknown)) ? keystates[key] : false;
    }

    void Keyboard::Flush()
    {
        empty = true;
        keystates.reset();
    }

    bool Keyboard::IsEmpty() const
    {
        return empty;
    }

    void Keyboard::ProcessKeyState(int key, int action)
    {
        if (key < 0 || key >= GLFW_KEY_LAST) return;

        int index = int(GLFWToKey(key));
        if (action == GLFW_PRESS)
        {
            if (!keystates[index])
            {
                keystates[index] = true;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            if (keystates[index])
            {
                keystates[index] = false;
            }
        }
    }
    Key Keyboard::GLFWToKey(int glfwKey)const
    {
        static constexpr std::array<Key, GLFW_KEY_LAST + 1> glfwToKey = [] 
            {
            std::array<Key, GLFW_KEY_LAST + 1> a{};
            for (int i = 0; i <= GLFW_KEY_LAST; ++i) a[i] = Key::Unknown;

            a[GLFW_KEY_SPACE] = Key::Space;
            a[GLFW_KEY_APOSTROPHE] = Key::Apostrophe;
            a[GLFW_KEY_COMMA] = Key::Comma;
            a[GLFW_KEY_MINUS] = Key::Minus;
            a[GLFW_KEY_PERIOD] = Key::Period;
            a[GLFW_KEY_SLASH] = Key::Slash;
            for (int i = 0; i <= 9; ++i) a[GLFW_KEY_0 + i] = Key(int(Key::D0) + i);
            a[GLFW_KEY_SEMICOLON] = Key::Semicolon;
            a[GLFW_KEY_EQUAL] = Key::Equal;
            for (int i = 0; i < 26; ++i) a[GLFW_KEY_A + i] = Key(int(Key::A) + i);
            a[GLFW_KEY_LEFT_BRACKET] = Key::LeftBracket;
            a[GLFW_KEY_BACKSLASH] = Key::Backslash;
            a[GLFW_KEY_RIGHT_BRACKET] = Key::RightBracket;
            a[GLFW_KEY_GRAVE_ACCENT] = Key::GraveAccent;
            a[GLFW_KEY_WORLD_1] = Key::World1;
            a[GLFW_KEY_WORLD_2] = Key::World2;

            a[GLFW_KEY_ESCAPE] = Key::Escape;
            a[GLFW_KEY_ENTER] = Key::Enter;
            a[GLFW_KEY_TAB] = Key::Tab;
            a[GLFW_KEY_BACKSPACE] = Key::Backspace;
            a[GLFW_KEY_INSERT] = Key::Insert;
            a[GLFW_KEY_DELETE] = Key::Delete;
            a[GLFW_KEY_RIGHT] = Key::Right;
            a[GLFW_KEY_LEFT] = Key::Left;
            a[GLFW_KEY_DOWN] = Key::Down;
            a[GLFW_KEY_UP] = Key::Up;
            a[GLFW_KEY_PAGE_UP] = Key::PageUp;
            a[GLFW_KEY_PAGE_DOWN] = Key::PageDown;
            a[GLFW_KEY_HOME] = Key::Home;
            a[GLFW_KEY_END] = Key::End;

            a[GLFW_KEY_CAPS_LOCK] = Key::CapsLock;
            a[GLFW_KEY_SCROLL_LOCK] = Key::ScrollLock;
            a[GLFW_KEY_NUM_LOCK] = Key::NumLock;

            a[GLFW_KEY_PRINT_SCREEN] = Key::PrintScreen;
            a[GLFW_KEY_PAUSE] = Key::Pause;

            for (int i = 0; i <= 25; ++i) a[GLFW_KEY_F1 + i] = Key(int(Key::F1) + i);

            for (int i = 0; i <= 9; ++i) a[GLFW_KEY_KP_0 + i] = Key(int(Key::Kp0) + i);
            a[GLFW_KEY_KP_DECIMAL] = Key::KpDecimal;
            a[GLFW_KEY_KP_DIVIDE] = Key::KpDivide;
            a[GLFW_KEY_KP_MULTIPLY] = Key::KpMultiply;
            a[GLFW_KEY_KP_SUBTRACT] = Key::KpSubtract;
            a[GLFW_KEY_KP_ADD] = Key::KpAdd;
            a[GLFW_KEY_KP_ENTER] = Key::KpEnter;
            a[GLFW_KEY_KP_EQUAL] = Key::KpEqual;

            a[GLFW_KEY_LEFT_SHIFT] = Key::LeftShift;
            a[GLFW_KEY_LEFT_CONTROL] = Key::LeftControl;
            a[GLFW_KEY_LEFT_ALT] = Key::LeftAlt;
            a[GLFW_KEY_LEFT_SUPER] = Key::LeftSuper;
            a[GLFW_KEY_RIGHT_SHIFT] = Key::RightShift;
            a[GLFW_KEY_RIGHT_CONTROL] = Key::RightControl;
            a[GLFW_KEY_RIGHT_ALT] = Key::RightAlt;
            a[GLFW_KEY_RIGHT_SUPER] = Key::RightSuper;

            a[GLFW_KEY_MENU] = Key::Menu;

            return a;
            }();

            return glfwToKey[glfwKey];
    }
}