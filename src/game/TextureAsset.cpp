#include "game/TextureAsset.hpp"
#include "game/AssetLocator.hpp"

#include <algorithm>

namespace game
{
Texture2D LoadTextureAsset(const std::string &relativePath, Rectangle crop,
                           int resizedWidth, int resizedHeight)
{
    const std::string path = ResolveAssetPath(relativePath);
    const bool needsProcessing = crop.width > 0.0F || crop.height > 0.0F ||
                                 resizedWidth > 0 || resizedHeight > 0;
    if (!needsProcessing)
    {
        Texture2D texture = LoadTexture(path.c_str());
        if (texture.id != 0) SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
        return texture;
    }

    Image image = LoadImage(path.c_str());
    if (image.data == nullptr) return {};
    if (crop.width > 0.0F && crop.height > 0.0F)
    {
        Rectangle safe = crop;
        safe.width = std::min(safe.width, static_cast<float>(image.width) - safe.x);
        safe.height = std::min(safe.height, static_cast<float>(image.height) - safe.y);
        ImageCrop(&image, safe);
    }
    if (resizedWidth > 0 && resizedHeight > 0)
        ImageResize(&image, resizedWidth, resizedHeight);

    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    if (texture.id != 0) SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
    return texture;
}

void UnloadTextureAsset(Texture2D &texture)
{
    if (texture.id == 0) return;
    UnloadTexture(texture);
    texture = {};
}
} // namespace game
