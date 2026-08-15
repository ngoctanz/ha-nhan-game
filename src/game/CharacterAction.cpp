#include "game/CharacterAction.hpp"
#include "game/TextureAsset.hpp"

#include <algorithm>

namespace game
{
CharacterAction ParseCharacterAction(std::string_view name)
{
    if (name == "hands_folded") return CharacterAction::HandsFolded;
    if (name == "point") return CharacterAction::Point;
    if (name == "present_low") return CharacterAction::PresentLow;
    if (name == "present_open") return CharacterAction::PresentOpen;
    if (name == "wave") return CharacterAction::Wave;
    // Backward-compatible aliases for older dialogue data.
    if (name == "rant") return CharacterAction::Point;
    if (name == "explain") return CharacterAction::PresentOpen;
    return CharacterAction::None;
}

CharacterActionPlayer::~CharacterActionPlayer()
{
    for (Texture2D &pose : poses_) UnloadTextureAsset(pose);
}

bool CharacterActionPlayer::Load(const std::string &assetRoot, Rectangle crop,
                                 int resizedWidth, int resizedHeight)
{
    const std::array<const char *, PoseCount> files = {
        "hands_folded.png", "point.png", "present_low.png",
        "present_open.png", "wave.png"};
    bool loaded = true;
    for (int i = 0; i < PoseCount; ++i)
    {
        poses_[i] = LoadTextureAsset(assetRoot + files[i], crop,
                                     resizedWidth, resizedHeight);
        loaded = loaded && poses_[i].id != 0;
    }
    return loaded;
}

void CharacterActionPlayer::Trigger(CharacterAction action)
{
    action_ = action;
    switch (action)
    {
        case CharacterAction::HandsFolded: activePose_ = HandsFolded; break;
        case CharacterAction::Point: activePose_ = Point; break;
        case CharacterAction::PresentLow: activePose_ = PresentLow; break;
        case CharacterAction::PresentOpen: activePose_ = PresentOpen; break;
        case CharacterAction::Wave: activePose_ = Wave; break;
        case CharacterAction::None:
            Clear();
            return;
    }
    active_ = true;
}

void CharacterActionPlayer::Clear()
{
    action_ = CharacterAction::None;
    active_ = false;
}

bool CharacterActionPlayer::IsActive() const
{
    return active_;
}

const Texture2D *CharacterActionPlayer::CurrentFrame() const
{
    if (!active_) return nullptr;
    const Texture2D &pose = poses_[activePose_];
    return pose.id == 0 ? nullptr : &pose;
}

const char *CharacterActionPlayer::SuggestedFace() const
{
    switch (action_)
    {
        case CharacterAction::HandsFolded: return "verified_indecisive_thought";
        case CharacterAction::Point: return "verified_steely_determination";
        case CharacterAction::PresentLow: return "verified_awkward_conflict";
        case CharacterAction::PresentOpen: return "verified_baffled_closed_eyes";
        case CharacterAction::Wave: return "verified_excited_surprise";
        case CharacterAction::None: return "verified_roundface_teasing_smirk";
    }
    return "verified_roundface_teasing_smirk";
}

int CharacterActionPlayer::LoadedPoseCount() const
{
    return static_cast<int>(std::count_if(
        poses_.begin(), poses_.end(), [](const Texture2D &pose) { return pose.id != 0; }));
}
} // namespace game
