#pragma once

#include "game/CharacterFacing.hpp"

#include <raylib.h>

namespace game
{
enum class NeighborWomanEvent
{
    None,
    MoveFinished
};

class NeighborWoman
{
public:
    ~NeighborWoman();
    bool Load();
    void Place(Vector2 feet, float speakerX);
    void FaceToward(float speakerX);
    void StartWalkTo(float targetX);
    NeighborWomanEvent Update(float deltaTime);
    void Draw(const Texture2D *face) const;
    [[nodiscard]] const Texture2D *PortraitTexture() const;
    [[nodiscard]] Vector2 Position() const;

private:
    Texture2D body_ = {};
    Vector2 position_ = {};
    CharacterFacing facing_{FacingDirection::Right};
    float walkTargetX_ = 0.0F;
    float walkTime_ = 0.0F;
    bool walking_ = false;
};
} // namespace game
