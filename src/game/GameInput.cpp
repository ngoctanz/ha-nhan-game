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
    const bool sneakKeyPressed = IsKeyPressed(KEY_LEFT_CONTROL) || IsKeyPressed(KEY_RIGHT_CONTROL);
    if (sneakKeyPressed) sneakToggle_ = !sneakToggle_;
    input.sneakHeld = sneakToggle_;  // applies to both desktop and mobile

    input.pointerPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    input.pointerScreen = GetMousePosition();
    input.hostMenuPressed = IsKeyPressed(KEY_P);
    input.storageMenuPressed = IsKeyPressed(KEY_I);
    input.menuBackPressed = IsKeyPressed(KEY_ESCAPE);
    input.menuUpPressed = IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP);
    input.menuDownPressed = IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN);

    if (MobileControlsEnabled())
    {
        int contactCount = GetTouchPointCount();
#if defined(HA_NHAN_MOBILE_UI_PREVIEW) && !defined(PLATFORM_ANDROID)
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) contactCount = std::max(contactCount, 1);
#endif
        bool sneakButtonDown = false;
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

            sneakButtonDown = sneakButtonDown ||
                PointInCircle(point, MobileLayout::Sneak, MobileLayout::ActionRadius);
        }
        // Sneak button is a toggle: rising edge (not-held → held) flips state.
        if (sneakButtonDown && !sneakWasHeld_) sneakToggle_ = !sneakToggle_;
        sneakWasHeld_ = sneakButtonDown;
        input.sneakHeld = sneakToggle_;

        input.pointerConsumedByControls =
            PointInCircle(input.pointerScreen, MobileLayout::MoveLeft, MobileLayout::MoveRadius) ||
            PointInCircle(input.pointerScreen, MobileLayout::MoveRight, MobileLayout::MoveRadius) ||
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
