#pragma once

#include <raylib.h>

#include <string>

namespace game
{
Texture2D LoadTextureAsset(const std::string &relativePath,
                           Rectangle crop = {},
                           int resizedWidth = 0,
                           int resizedHeight = 0);
void UnloadTextureAsset(Texture2D &texture);
} // namespace game
