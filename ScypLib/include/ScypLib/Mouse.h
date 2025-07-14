#pragma once
#include <queue>
#include <GLFW/glfw3.h>
#include "Vec2.h"

namespace sl
{
    class Mouse
    {
    public:
        friend class EventDispatcher;
    public:
        Mouse() = default;
        Mouse(const Mouse&) = delete;
        Mouse& operator=(const Mouse&) = delete;

        Vec2f GetPos() const;
        Vec2f GetScrollOffset() const;
        float GetScrollOffsetX() const;
        float GetScrollOffsetY() const;
        float GetPosX() const;
        float GetPosY() const;
        bool LeftIsPressed() const;
        bool RightIsPressed() const;
        bool ScrollIsPressed() const;
        bool IsInWindow() const;

        void Flush();
        bool IsEmpty() const;
    private:
        void SetIsInWindow(bool isInWind);
        void OnMouseMove(float x, float y);
        void OnLeftPressed();
        void OnLeftReleased();
        void OnRightPressed();
        void OnRightReleased();
        void OnScrollPressed();
        void OnScrollReleased();
        void OnWheelScroll(float offsetX, float offsetY);
    private:
        Vec2f pos = { 0,0 };
        Vec2f scrollOffset = { 0,0 };
        bool leftIsPressed = false;
        bool rightIsPressed = false;
        bool scrollIsPressed = false;
        bool empty = true;
        bool isInWindow = false;
    };
}