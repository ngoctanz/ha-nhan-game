#pragma once

#include <raylib.h>

namespace game
{
// Places the authored foot line on the requested world-space point.  The
// normalized foot value is measured once from the source asset, so transparent
// padding below a character can never make it hover above its shadow.
inline Rectangle GroundedDestination(const Texture2D &texture, Vector2 feet,
                                     float drawHeight, float normalizedFootY)
{
    const float drawWidth = drawHeight * texture.width / static_cast<float>(texture.height);
    return {
        feet.x - drawWidth / 2.0F,
        feet.y - drawHeight * normalizedFootY,
        drawWidth,
        drawHeight};
}

inline void DrawGroundShadow(Vector2 feet, float radiusX, float radiusY, Color color)
{
    // A tiny overlap puts the shoes in front of the shadow instead of leaving
    // a bright strip that reads as empty air.
    DrawEllipse(static_cast<int>(feet.x), static_cast<int>(feet.y + 2.0F),
                radiusX, radiusY, color);
}
} // namespace game
