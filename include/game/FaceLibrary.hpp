#pragma once

#include <raylib.h>

#include <string>
#include <unordered_map>

namespace game
{
class FaceLibrary
{
public:
    FaceLibrary() = default;
    ~FaceLibrary();
    FaceLibrary(const FaceLibrary &) = delete;
    FaceLibrary &operator=(const FaceLibrary &) = delete;

    bool Load();
    [[nodiscard]] const Texture2D *Get(const std::string &expression) const;

private:
    std::unordered_map<std::string, Texture2D> faces_;
};
} // namespace game
