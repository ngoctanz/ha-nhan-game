#pragma once

#include "game/CharacterFacing.hpp"

#include <raylib.h>

namespace game
{
enum class SystemDogPose
{
    Idle,
    Quest,
    Laugh
};

class SystemDog
{
public:
    SystemDog() = default;
    ~SystemDog();
    SystemDog(const SystemDog &) = delete;
    SystemDog &operator=(const SystemDog &) = delete;

    bool Load();
    void PlaceForConversation(Vector2 position, float speakerX);
    void FaceToward(float speakerX);
    void TriggerHit();
    void UpdateReaction(float deltaTime);
    void Update(float deltaTime, Vector2 playerFeet, bool playerFacingRight);
    void DrawWorld(SystemDogPose pose, float time) const;
    [[nodiscard]] const Texture2D *Pose(SystemDogPose pose) const;
    [[nodiscard]] Vector2 Position() const;

private:
    Texture2D idle_ = {};
    Texture2D quest_ = {};
    Texture2D laugh_ = {};
    Vector2 position_ = {};
    float lastPlayerX_ = 0.0F;
    float pendingTravel_ = 0.0F;
    int formationSide_ = -1;
    int pendingSide_ = -1;
    bool positioned_ = false;
    float hitReactionTimer_ = 0.0F;
    CharacterFacing facing_;
};
} // namespace game
