#pragma once

#include <raylib.h>

namespace game
{
struct FaceAnchor
{
    float centerX = 0.5F;
    float centerY = 0.35F;
    float width = 0.36F;
    float height = 0.23F;
    Color maskColor = {250, 250, 247, 255};
    float rotationDegrees = 0.0F;
    bool maskOriginal = true;
    // Signed offset in the unflipped artwork's X direction. Mirroring applies
    // it to the opposite side automatically, so the face always moves forward.
    float forwardOffsetX = 0.0F;
};

void DrawFaceOverlay(const Texture2D *face, Rectangle bodyDestination,
                     FaceAnchor anchor, bool flipped);
void DrawFaceOverlay(const Texture2D *face, Rectangle bodyDestination,
                     FaceAnchor anchor, bool bodyFlipped, bool faceFlipped);
} // namespace game
