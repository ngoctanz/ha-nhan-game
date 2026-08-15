#include "game/AssetLocator.hpp"

#include <raylib.h>

#include <array>

namespace game
{
std::string ResolveAssetPath(const std::string &relativePath)
{
    const std::array<std::string, 5> prefixes = {"", "../", "../../", "../../../", "../../../../"};
    for (const std::string &prefix : prefixes)
    {
        const std::string candidate = prefix + relativePath;
        if (FileExists(candidate.c_str())) return candidate;
    }
    return relativePath;
}
} // namespace game
