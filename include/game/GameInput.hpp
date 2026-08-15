#pragma once

#include <raylib.h>

namespace game
{
struct GameInput
{
    float moveAxis = 0.0F;
    bool sprintHeld = false;
    bool sneakHeld = false;
    bool goofyHeld = false;
    bool interactHeld = false;
    bool interactPressed = false;
    bool advanceDialoguePressed = false;
    bool pointerPressed = false;
    bool pointerConsumedByControls = false;
    Vector2 pointerScreen = {};
};

namespace MobileLayout
{
inline constexpr Vector2 MoveLeft = {82.0F, 638.0F};
inline constexpr Vector2 MoveRight = {184.0F, 638.0F};
inline constexpr Vector2 Sneak = {1074.0F, 638.0F};
inline constexpr Vector2 Goofy = {1182.0F, 638.0F};
inline constexpr float MoveRadius = 47.0F;
inline constexpr float ActionRadius = 45.0F;
} // namespace MobileLayout

[[nodiscard]] bool MobileControlsEnabled();
[[nodiscard]] bool PointInCircle(Vector2 point, Vector2 center, float radius);

class InputSystem
{
public:
    [[nodiscard]] GameInput Poll();
};
} // namespace game
