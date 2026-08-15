#pragma once

#include "game/Dialogue.hpp"
#include "game/Chicken.hpp"
#include "game/Elder.hpp"
#include "game/FaceLibrary.hpp"
#include "game/GameInput.hpp"
#include "game/Level.hpp"
#include "game/Player.hpp"
#include "game/Quest.hpp"
#include "game/SoundEffects.hpp"
#include "game/SystemDog.hpp"

#include <raylib.h>

#include <cstdint>
#include <string>

namespace game
{
enum class QuestState
{
    MeetElder,
    CatchChicken,
    CollectShard,
    ReturnToElder,
    Complete,
    LeaveVillage,
    LeavingVillage
};

class BambooVillageLevel final : public Level
{
public:
    explicit BambooVillageLevel(GameProgress &progress) : Level(progress), quest_(progress) {}
    ~BambooVillageLevel() override;
    bool Load() override;
    void Update(float deltaTime, const GameInput &input) override;
    void Draw(const Ui &ui) const override;
    [[nodiscard]] LevelTransition RequestedTransition() const override { return transition_; }

private:
    void ApplyDialogueAction(DialogueAction action);
    void HandleInteraction(const GameInput &input);
    void SyncDialoguePresentation();
    void BeginChickenResultDialogue();
    void BeginChickenLoreDialogue();
    [[nodiscard]] const std::string &Objective() const;
    [[nodiscard]] std::string InteractionPrompt() const;
    [[nodiscard]] const Texture2D *CurrentPortrait() const;
    [[nodiscard]] const Texture2D *CurrentPlayerFace() const;
    [[nodiscard]] const Texture2D *CurrentPortraitFace() const;

    Texture2D background_ = {};
    Player player_;
    Chicken chicken_;
    Elder elder_;
    FaceLibrary faces_;
    FaceAnimator playerFace_;
    FaceAnimator elderFace_;
    SystemDog systemDog_;
    SoundEffects soundEffects_;
    Dialogue dialogue_;
    QuestState questState_ = QuestState::MeetElder;
    QuestTracker quest_;
    Camera2D camera_ = {};
    Vector2 elderPosition_ = {610.0F, 622.0F};
    Vector2 shardPosition_ = {2020.0F, 537.0F};
    bool shardVisible_ = false;
    int catchAttempts_ = 0;
    float worldTime_ = 0.0F;
    float toastTimer_ = 4.5F;
    float questIssuedTimer_ = 0.0F;
    float levelTitleTimer_ = 4.0F;
    float controlsHintTimer_ = 5.0F;
    std::uint64_t observedDialogueRevision_ = 0;
    GameInput input_ = {};
    LevelTransition transition_ = LevelTransition::None;
    const std::string exitObjective_ = "Đi tới đầu làng tìm dấu vết Gà Linh Khí";
    std::string toast_ = "Ting! Ký chủ mới tới. Việc đầu tiên: đi hỏi xin cơm.";
};
} // namespace game
