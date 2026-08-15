#include "game/GameInput.hpp"

#include <algorithm>
#include <cmath>

namespace game
{
bool MobileControlsEnabled()
{
#if defined(PLATFORM_ANDROID) || defined(HA_NHAN_MOBILE_UI_PREVIEW)
    return true;
#else
    return false;
#endif
}

bool PointInCircle(Vector2 point, Vector2 center, float radius)
{
    const float x = point.x - center.x;
    const float y = point.y - center.y;
    return x * x + y * y <= radius * radius;
}

GameInput InputSystem::Poll()
{
    GameInput input;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) input.moveAxis -= 1.0F;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) input.moveAxis += 1.0F;
    input.sprintHeld = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    input.sneakHeld = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
    input.goofyHeld = IsKeyDown(KEY_C);

    input.pointerPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    input.pointerScreen = GetMousePosition();

    if (MobileControlsEnabled())
    {
        int contactCount = GetTouchPointCount();
#if defined(HA_NHAN_MOBILE_UI_PREVIEW) && !defined(PLATFORM_ANDROID)
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) contactCount = std::max(contactCount, 1);
#endif
        for (int index = 0; index < contactCount; ++index)
        {
            Vector2 point = GetTouchPosition(index);
#if defined(HA_NHAN_MOBILE_UI_PREVIEW) && !defined(PLATFORM_ANDROID)
            if (GetTouchPointCount() == 0) point = GetMousePosition();
#endif
            if (PointInCircle(point, MobileLayout::MoveLeft, MobileLayout::MoveRadius))
                input.moveAxis = -1.0F;
            else if (PointInCircle(point, MobileLayout::MoveRight, MobileLayout::MoveRadius))
                input.moveAxis = 1.0F;

            input.goofyHeld = input.goofyHeld ||
                PointInCircle(point, MobileLayout::Goofy, MobileLayout::ActionRadius);
            input.sneakHeld = input.sneakHeld ||
                PointInCircle(point, MobileLayout::Sneak, MobileLayout::ActionRadius);
        }

        input.pointerConsumedByControls =
            PointInCircle(input.pointerScreen, MobileLayout::MoveLeft, MobileLayout::MoveRadius) ||
            PointInCircle(input.pointerScreen, MobileLayout::MoveRight, MobileLayout::MoveRadius) ||
            PointInCircle(input.pointerScreen, MobileLayout::Goofy, MobileLayout::ActionRadius) ||
            PointInCircle(input.pointerScreen, MobileLayout::Sneak, MobileLayout::ActionRadius);
    }

    const bool keyboardInteract = IsKeyPressed(KEY_E) || IsKeyPressed(KEY_SPACE);
    input.interactHeld = IsKeyDown(KEY_E) || IsKeyDown(KEY_SPACE);
    input.interactPressed = keyboardInteract;
    input.advanceDialoguePressed = input.interactPressed || IsKeyPressed(KEY_ENTER) ||
                                   (input.pointerPressed && !input.pointerConsumedByControls);
    return input;
}
} // namespace game
