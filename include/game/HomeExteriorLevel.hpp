#pragma once

#include "game/Dialogue.hpp"
#include "game/FaceLibrary.hpp"
#include "game/Level.hpp"
#include "game/NeighborWoman.hpp"
#include "game/Player.hpp"
#include "game/SystemDog.hpp"

#include <raylib.h>

#include <cstdint>

namespace game
{
class HomeExteriorLevel final : public Level
{
public:
    explicit HomeExteriorLevel(GameProgress &progress) : Level(progress) {}
    ~HomeExteriorLevel() override;
    bool Load() override;
    void Update(float deltaTime, const GameInput &input) override;
    void Draw(const Ui &ui) const override;
    [[nodiscard]] LevelTransition RequestedTransition() const override;

private:
    void BeginMeetingDialogue();
    void BeginReflectionDialogue();
    void SyncDialoguePresentation();
    [[nodiscard]] const Texture2D *CurrentPortrait() const;
    [[nodiscard]] const Texture2D *CurrentPortraitFace() const;

    Texture2D background_ = {};
    Player player_;
    NeighborWoman neighbor_;
    SystemDog systemDog_;
    FaceLibrary faces_;
    FaceAnimator playerFace_;
    FaceAnimator neighborFace_;
    Dialogue dialogue_;
    std::uint64_t observedDialogueRevision_ = 0;
    bool neighborVisible_ = true;
    LevelTransition transition_ = LevelTransition::None;
};
} // namespace game
