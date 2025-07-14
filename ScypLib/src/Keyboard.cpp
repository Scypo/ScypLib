#include"ScypLib/Keyboard.h"

namespace sl
{
    int Keyboard::CharToKey(char character) const
    {
        if (character >= 'a' && character <= 'z')
        {
            return GLFW_KEY_A + (character - 'a');
        }
        else if (character >= 'A' && character <= 'Z')
        {
            return GLFW_KEY_A + (character - 'A');
        }
        else if (character >= '0' && character <= '9')
        {
            return GLFW_KEY_0 + (character - '0');
        }

        switch (character)
        {
        case ' ': return GLFW_KEY_SPACE;
        case '\n': return GLFW_KEY_ENTER;
        case '\t': return GLFW_KEY_TAB;
        case '\r': return GLFW_KEY_ENTER;
        case '\b': return GLFW_KEY_BACKSPACE;

        case '!': return GLFW_KEY_1;
        case '@': return GLFW_KEY_2;
        case '#': return GLFW_KEY_3;
        case '$': return GLFW_KEY_4;
        case '%': return GLFW_KEY_5;
        case '^': return GLFW_KEY_6;
        case '&': return GLFW_KEY_7;
        case '*': return GLFW_KEY_8;
        case '(': return GLFW_KEY_9;
        case ')': return GLFW_KEY_0;

        case '-': return GLFW_KEY_MINUS;
        case '=': return GLFW_KEY_EQUAL;
        case '[': return GLFW_KEY_LEFT_BRACKET;
        case ']': return GLFW_KEY_RIGHT_BRACKET;
        case '\\': return GLFW_KEY_BACKSLASH;
        case ';': return GLFW_KEY_SEMICOLON;
        case '\'': return GLFW_KEY_APOSTROPHE;
        case ',': return GLFW_KEY_COMMA;
        case '.': return GLFW_KEY_PERIOD;
        case '/': return GLFW_KEY_SLASH;
        case '`': return GLFW_KEY_GRAVE_ACCENT;

        default: return GLFW_KEY_UNKNOWN;
        }
    }

    bool Keyboard::KeyIsPressed(int key) const
    {
        return keystates[key];
    }

    bool Keyboard::KeyIsPressed(char character) const
    {
        int key = CharToKey(character);
        return (key != GLFW_KEY_UNKNOWN) ? keystates[key] : false;
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

        if (action == GLFW_PRESS)
        {
            if (!keystates[key])
            {
                keystates[key] = true;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            if (keystates[key])
            {
                keystates[key] = false;
            }
        }
    }
}