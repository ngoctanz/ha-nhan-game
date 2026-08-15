#pragma once

#include "game/CharacterFacing.hpp"

#include <raylib.h>

#include <string>
#include <string_view>

namespace game
{
class Elder
{
public:
    Elder() = default;
    ~Elder();
    Elder(const Elder &) = delete;
    Elder &operator=(const Elder &) = delete;

    bool Load();
    void FaceToward(float selfX, float targetX);
    void Draw(Vector2 feet, float worldTime, const Texture2D *face) const;
    [[nodiscard]] const Texture2D *Portrait(std::string_view emotion) const;

private:
    Texture2D fullBody_ = {};
    CharacterFacing facing_;
};
} // namespace game
