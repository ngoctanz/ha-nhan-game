#pragma once

#include "game/CombatSystem.hpp"
#include "game/Dialogue.hpp"
#include "game/FaceLibrary.hpp"
#include "game/Level.hpp"
#include "game/NeighborWoman.hpp"
#include "game/Player.hpp"
#include "game/RuffianGroup.hpp"
#include "game/SystemChoice.hpp"
#include "game/SystemDog.hpp"

#include <cstdint>
#include <raylib.h>
#include <string>

namespace game
{
class NeighborNightLevel final : public Level
{
public:
    explicit NeighborNightLevel(GameProgress &progress) : Level(progress) {}
    ~NeighborNightLevel() override;
    bool Load() override;
    void Update(float deltaTime, const GameInput &input) override;
    void Draw(const Ui &ui) const override;
    [[nodiscard]] CombatSystem *ActiveCombat() override { return &combat_; }
    [[nodiscard]] SystemMenuCommand ConsumeSystemMenuCommand() override;

private:
    enum class Scene { ReturnVillage, FrontYard, BackYard };
    enum class Phase
    {
        ReturnEntering,
        ReturnDialogue,
        ReturnLeaving,
        FrontDialogue,
        FrontFenceWalk,
        FenceDialogue,
        BackSneak,
        BackDialogue,
        BackApproach,
        WhisperDialogue,
        QuestIssued,
        ProvocationChoice,
        ProvocationDialogue,
        CaughtDialogue,
        StoragePreview,
        StorageExplanation,
        AwaitingThrow,
        Throwing,
        CombatImpact,
        VictoryDialogue,
        NeighborEntering,
        NeighborAftermath,
        EndingFade,
        ChapterEnd,
        BackIdle,
        FadingOut,
        FadingIn
    };

    void RestoreCheckpoint();
    void ConfigureReturnVillage();
    void ConfigureFrontYard();
    void ConfigureBackYard();
    void BeginReturnDialogue();
    void BeginFrontDialogue();
    void BeginFenceDialogue();
    void BeginBackDialogue();
    void BeginWhisperDialogue();
    void BeginProvocationChoice();
    void ResolveProvocationChoice(ChoiceOption option);
    void BeginEnemyEncounterDialogue();
    void BeginStorageExplanation();
    void UnlockThrowSkill();
    void BeginVictoryDialogue();
    void BeginNeighborEntrance();
    void BeginNeighborAftermathDialogue();
    void BeginEndingFade();
    void BeginCombatEncounter();
    void ApplyDialogueAction(DialogueAction action);
    void SyncDialoguePresentation();
    void BeginSceneFade(Scene destination, Phase afterFade);
    void SaveCheckpoint(const char *checkpoint);
    [[nodiscard]] const Texture2D &CurrentBackground() const;
    [[nodiscard]] const Texture2D *CurrentPortrait() const;
    [[nodiscard]] const Texture2D *CurrentPortraitFace() const;
    [[nodiscard]] int CurrentRuffianIndex() const;

    Texture2D villageBackground_ = {};
    Texture2D frontBackground_ = {};
    Texture2D backBackground_ = {};
    Texture2D heavenlyMeteorBurst_ = {};
    Player player_;
    SystemDog systemDog_;
    NeighborWoman neighbor_;
    RuffianGroup ruffians_;
    FaceLibrary faces_;
    FaceAnimator playerFace_;
    FaceAnimator neighborFace_;
    Dialogue dialogue_;
    SystemChoice choice_;
    CombatSystem combat_;
    CombatCommand activeCombatCommand_;
    Camera2D camera_ = {};
    Scene scene_ = Scene::FrontYard;
    Scene fadeDestination_ = Scene::FrontYard;
    Phase phase_ = Phase::FrontDialogue;
    Phase phaseAfterFade_ = Phase::FrontDialogue;
    std::uint64_t observedDialogueRevision_ = 0;
    float worldTime_ = 0.0F;
    float fadeAlpha_ = 0.0F;
    bool injured_ = false;
    bool ruffiansVisible_ = false;
    bool neighborVisible_ = false;
    int talkingRuffian_ = -1;
    float combatImpactTimer_ = 0.0F;
    float skillUnlockTimer_ = 0.0F;
    float rewardNoticeTimer_ = 0.0F;
    float questIssuedTimer_ = 0.0F;
    float endingFadeAlpha_ = 0.0F;
    SystemMenuCommand pendingMenuCommand_ = SystemMenuCommand::None;
    std::string objective_;
    std::string notice_;
};
} // namespace game
