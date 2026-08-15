#pragma once

#include <raylib.h>

namespace game
{
struct GameInput
{
    float moveAxis = 0.0F;
    bool sprintHeld = false;
    bool sneakHeld = false;
    bool interactHeld = false;
    bool interactPressed = false;
    bool advanceDialoguePressed = false;
    bool pointerPressed = false;
    bool pointerConsumedByControls = false;
    bool hostMenuPressed = false;
    bool storageMenuPressed = false;
    bool menuBackPressed = false;
    bool menuUpPressed = false;
    bool menuDownPressed = false;
    Vector2 pointerScreen = {};
};

namespace MobileLayout
{
inline constexpr Vector2 MoveLeft = {68.0F, 632.0F};
inline constexpr Vector2 MoveRight = {196.0F, 632.0F};
inline constexpr Vector2 Sneak = {1196.0F, 632.0F};
inline constexpr float MoveRadius = 54.0F;
inline constexpr float ActionRadius = 52.0F;
} // namespace MobileLayout

[[nodiscard]] bool MobileControlsEnabled();
[[nodiscard]] bool PointInCircle(Vector2 point, Vector2 center, float radius);

class InputSystem
{
public:
    [[nodiscard]] GameInput Poll();

private:
    bool sneakToggle_ = false;
    bool sneakWasHeld_ = false;  // tracks previous frame's button state for edge detection
};
} // namespace game
