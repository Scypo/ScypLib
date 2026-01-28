#include"ScypLib/Mouse.h"
#include"ScypLib/Vec2.h"

namespace sl
{
    void Mouse::SetIsInWindow(bool isInWind) { isInWindow = isInWind; }

    Vec2f Mouse::GetPos() const { return pos; }
    Vec2f Mouse::GetScrollOffset() const { return scrollOffset; }
    float Mouse::GetPosX() const { return pos.x; }
    float Mouse::GetScrollOffsetX() const { return scrollOffset.x; }
    float Mouse::GetScrollOffsetY() const { return scrollOffset.y; }
    Vec2f Mouse::GetMoveOffset() const { return moveOffset; }
    float Mouse::GetMoveOffsetX() const { return moveOffset.x; }
    float Mouse::GetMoveOffsetY() const { return moveOffset.y; }
    float Mouse::GetPosY() const { return pos.y; }
    bool Mouse::LeftIsPressed() const { return leftIsPressed; }
    bool Mouse::RightIsPressed() const { return rightIsPressed; }
    bool Mouse::ScrollIsPressed() const
    {
        return scrollIsPressed;
    }
    bool Mouse::IsInWindow() const { return isInWindow; }

    void Mouse::Flush()
    {
        leftIsPressed = false;
        rightIsPressed = false;
        scrollIsPressed = false;
        empty = true;
        scrollOffset = { 0,0 };
        moveOffset = { 0,0 };
    }

    bool Mouse::IsEmpty() const
    {
        return empty;
    }

    void Mouse::OnMouseMove(float x, float y)
    {
        moveOffset = { x - pos.x, y - pos.y };
        pos = { x, y };
    }

    void Mouse::OnLeftPressed()
    {
        leftIsPressed = true;
    }

    void Mouse::OnLeftReleased()
    {
        leftIsPressed = false;
    }

    void Mouse::OnRightPressed()
    {
        rightIsPressed = true;
    }

    void Mouse::OnRightReleased()
    {
        rightIsPressed = false;
    }

    void Mouse::OnWheelScroll(float offsetX, float offsetY)
    {
        scrollOffset = { offsetX, offsetY };
    }

    void Mouse::OnScrollPressed()
    {
        scrollIsPressed = true;
    }

    void Mouse::OnScrollReleased()
    {
        scrollIsPressed = false;
    }
}