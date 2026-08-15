#pragma once

#include "game/CharacterFacing.hpp"

#include <raylib.h>

namespace game
{
enum class ChickenState
{
    Idle,
    Alert,
    Startled,
    Running,
    Settling,
    Hidden
};

enum class ChickenEvent
{
    None,
    EscapeFinished,
    Vanished
};

class Chicken
{
public:
    Chicken() = default;
    ~Chicken();
    Chicken(const Chicken &) = delete;
    Chicken &operator=(const Chicken &) = delete;

    bool Load();
    void SetPosition(Vector2 feet);
    void SetAlert(bool alert);
    void StartEscape(Vector2 destination, bool vanishAfterRun);
    ChickenEvent Update(float deltaTime);
    void Draw() const;

    [[nodiscard]] Vector2 Position() const;
    [[nodiscard]] bool CanInteract() const;
    [[nodiscard]] bool IsHidden() const;

private:
    [[nodiscard]] const Texture2D *CurrentTexture() const;

    Texture2D idle_ = {};
    Texture2D suspicious_ = {};
    Texture2D run_ = {};
    Texture2D startled_ = {};
    Vector2 position_ = {};
    Vector2 runOrigin_ = {};
    Vector2 runTarget_ = {};
    ChickenState state_ = ChickenState::Idle;
    float stateTimer_ = 0.0F;
    bool alertAfterAction_ = false;
    bool vanishAfterRun_ = false;
    CharacterFacing facing_;
};
} // namespace game
