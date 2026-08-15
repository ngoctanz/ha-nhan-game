#pragma once

#include "game/Dialogue.hpp"
#include "game/FaceLibrary.hpp"
#include "game/Level.hpp"
#include "game/Player.hpp"
#include "game/RuffianGroup.hpp"
#include "game/SoundEffects.hpp"
#include "game/SystemChoice.hpp"
#include "game/SystemDog.hpp"

#include <cstdint>
#include <raylib.h>
#include <string>

namespace game
{
class VillageGateLevel final : public Level
{
public:
    explicit VillageGateLevel(GameProgress &progress) : Level(progress) {}
    ~VillageGateLevel() override;
    bool Load() override;
    void Update(float deltaTime, const GameInput &input) override;
    void Draw(const Ui &ui) const override;
    [[nodiscard]] LevelTransition RequestedTransition() const override { return transition_; }

private:
    enum class Phase
    {
        Entering,
        Dialogue,
        Hiding,
        Choice,
        FlyingKick,
        Fight,
        RuffiansLeaving,
        SneakPursuit,
        EscapeRun,
        Finished
    };

    void StartFresh();
    void RestoreCheckpoint();
    void ApplyDialogueAction(DialogueAction action);
    void SyncDialoguePresentation();
    void BeginSpottingDialogue();
    void BeginEavesdropDialogue();
    void BeginInterventionChoice();
    void ResolveChoice(ChoiceOption choice);
    void BeginYesConfrontation();
    void BeginNoRestraintDialogue();
    void BeginNoPursuitDialogue();
    void BeginAfterFightDialogue();
    void FinishScene(const char *checkpoint);
    [[nodiscard]] int CurrentRuffianIndex() const;
    [[nodiscard]] const Texture2D *CurrentPortrait() const;
    [[nodiscard]] const Texture2D *CurrentPortraitFace() const;
    [[nodiscard]] const std::string &Objective() const;

    Texture2D background_ = {};
    Texture2D hidingRock_ = {};
    Player player_;
    FaceLibrary faces_;
    FaceAnimator playerFace_;
    SystemDog systemDog_;
    RuffianGroup ruffians_;
    SoundEffects soundEffects_;
    Dialogue dialogue_;
    SystemChoice choice_;
    Camera2D camera_ = {};
    Phase phase_ = Phase::Entering;
    LevelTransition transition_ = LevelTransition::None;
    std::uint64_t observedDialogueRevision_ = 0;
    int talkingRuffian_ = -1;
    float worldTime_ = 0.0F;
    float fightTimer_ = 0.0F;
    float toastTimer_ = 0.0F;
    bool fightResultStaged_ = false;
    bool playerBehindRock_ = false;
    bool ruffiansVisible_ = false;
    std::string objective_;
    std::string toast_;
};
} // namespace game
