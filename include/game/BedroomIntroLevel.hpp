#pragma once

#include "game/Dialogue.hpp"
#include "game/FaceLibrary.hpp"
#include "game/Level.hpp"
#include "game/Player.hpp"
#include "game/SoundEffects.hpp"
#include "game/SystemDog.hpp"
#include "game/SystemChoice.hpp"

#include <raylib.h>

#include <cstdint>

namespace game
{
class BedroomIntroLevel final : public Level
{
public:
    explicit BedroomIntroLevel(GameProgress &progress) : Level(progress) {}
    ~BedroomIntroLevel() override;
    bool Load() override;
    void Update(float deltaTime, const GameInput &input) override;
    void Draw(const Ui &ui) const override;
    [[nodiscard]] LevelTransition RequestedTransition() const override;

private:
    void BeginAfterThrowDialogue();
    void BeginRewardChoice();
    void ResolveRewardChoice(ChoiceOption attemptedChoice);
    void BeginPenaltyDialogue(ChoiceOption attemptedChoice);
    void SyncDialoguePresentation();
    [[nodiscard]] const Texture2D *CurrentPortrait() const;
    [[nodiscard]] const Texture2D *CurrentPortraitFace() const;

    Texture2D background_ = {};
    Player player_;
    FaceLibrary faces_;
    FaceAnimator playerFace_;
    SystemDog systemDog_;
    SoundEffects soundEffects_;
    Dialogue dialogue_;
    SystemChoice systemChoice_;
    SystemDogPose systemPose_ = SystemDogPose::Idle;
    std::uint64_t observedDialogueRevision_ = 0;
    bool systemVisible_ = false;
    bool finished_ = false;
    LevelTransition transition_ = LevelTransition::None;
};
} // namespace game
