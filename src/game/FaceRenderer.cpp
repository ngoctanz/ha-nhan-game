#include "game/FaceRenderer.hpp"

#include <algorithm>

namespace game
{
void DrawFaceOverlay(const Texture2D *face, Rectangle bodyDestination,
                     FaceAnchor anchor, bool flipped)
{
    DrawFaceOverlay(face, bodyDestination, anchor, flipped, flipped);
}

void DrawFaceOverlay(const Texture2D *face, Rectangle bodyDestination,
                     FaceAnchor anchor, bool bodyFlipped, bool faceFlipped)
{
    if (face == nullptr || face->id == 0) return;

    const float authoredX = anchor.centerX + anchor.forwardOffsetX;
    const float normalizedX = bodyFlipped ? 1.0F - authoredX : authoredX;
    const Vector2 center = {
        bodyDestination.x + bodyDestination.width * normalizedX,
        bodyDestination.y + bodyDestination.height * anchor.centerY};
    const float width = bodyDestination.width * anchor.width;
    const float height = bodyDestination.height * anchor.height;

    // The body sprites still contain their original face. This inner-head
    // patch turns them into a clean reusable body before the new expression
    // is rendered on top.
    if (anchor.maskOriginal)
        DrawEllipse(static_cast<int>(center.x), static_cast<int>(center.y),
                    width * 0.72F, height * 0.72F, anchor.maskColor);

    Rectangle source = {0, 0, static_cast<float>(face->width), static_cast<float>(face->height)};
    if (faceFlipped)
    {
        source.width = -source.width;
    }
    const float faceScale = std::min(width / static_cast<float>(face->width),
                                     height / static_cast<float>(face->height));
    const float faceWidth = face->width * faceScale;
    const float faceHeight = face->height * faceScale;
    const float rotation = bodyFlipped ? -anchor.rotationDegrees : anchor.rotationDegrees;
    DrawTexturePro(*face, source,
                   {center.x, center.y, faceWidth, faceHeight},
                   {faceWidth / 2.0F, faceHeight / 2.0F}, rotation, WHITE);
}
} // namespace game
