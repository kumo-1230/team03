#include "system_cursor.h"
#include "System/Graphics.h"

void SystemCursor::Show()
{
    if (!is_visible_)
    {
        ::ShowCursor(TRUE);
        is_visible_ = true;
    }
}

void SystemCursor::Hide()
{
    if (is_visible_)
    {
        ::ShowCursor(FALSE);
        is_visible_ = false;
    }
}

bool SystemCursor::IsVisible()
{
    return is_visible_;
}

DirectX::XMFLOAT2 SystemCursor::GetPosition()
{
    POINT cursor;
    ::GetCursorPos(&cursor);
    ::ScreenToClient(Graphics::Instance().GetWindowHandle(), &cursor);
    return { static_cast<float>(cursor.x), static_cast<float>(cursor.y) };
}

void SystemCursor::SetPosition(const DirectX::XMFLOAT2& pos)
{
    POINT screenPos{ static_cast<LONG>(pos.x), static_cast<LONG>(pos.y) };
    ::ClientToScreen(Graphics::Instance().GetWindowHandle(), &screenPos);
    ::SetCursorPos(screenPos.x, screenPos.y);
}

void SystemCursor::CenterCursor()
{
    const float centerX = Graphics::Instance().GetScreenWidth() / 2.0f;
    const float centerY = Graphics::Instance().GetScreenHeight() / 2.0f;
    SetPosition({ centerX, centerY });
}