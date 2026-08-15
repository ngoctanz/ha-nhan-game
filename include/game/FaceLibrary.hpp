#pragma once

#include <raylib.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace game
{
enum class FaceGender : std::uint8_t
{
    Male,
    Female
};

struct FaceId
{
    static constexpr std::uint16_t InvalidValue = 0xffff;
    std::uint16_t value = InvalidValue;

    [[nodiscard]] bool IsValid() const { return value != InvalidValue; }
    friend bool operator==(FaceId, FaceId) = default;
};

class FaceLibrary
{
public:
    FaceLibrary() = default;
    ~FaceLibrary();
    FaceLibrary(const FaceLibrary &) = delete;
    FaceLibrary &operator=(const FaceLibrary &) = delete;

    // Loads every six-frame expression exactly once. Runtime code only keeps
    // FaceId values, so drawing never performs path work or string allocation.
    bool Load();
    [[nodiscard]] FaceId Find(FaceGender gender, std::string_view expression) const;
    [[nodiscard]] const Texture2D *Frame(FaceId face, float elapsedSeconds) const;
    [[nodiscard]] std::size_t ExpressionCount() const;
    [[nodiscard]] std::size_t TextureCount() const;

private:
    struct Clip
    {
        std::vector<Texture2D> frames;
        float framesPerSecond = 8.0F;
    };

    std::vector<Clip> clips_;
    std::unordered_map<std::string, FaceId> ids_;
    FaceId maleFallback_;
    FaceId femaleFallback_;
};

// Lightweight per-character playback state. Call SetExpression only when the
// desired emotion changes, Update once per frame, and pass Current() to the
// existing body/portrait renderer.
class FaceAnimator
{
public:
    bool SetExpression(const FaceLibrary &library, FaceGender gender,
                       std::string_view expression);
    void Update(float deltaTime);
    [[nodiscard]] const Texture2D *Current(const FaceLibrary &library) const;
    [[nodiscard]] FaceId CurrentId() const;

private:
    FaceId face_;
    FaceGender gender_ = FaceGender::Male;
    std::string expression_;
    float elapsedSeconds_ = 0.0F;
};
} // namespace game
