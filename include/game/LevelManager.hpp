#pragma once

#include "game/Level.hpp"
#include "game/GameProgress.hpp"

#include <memory>

namespace game
{
class Ui;
struct GameInput;

enum class LevelId
{
    BedroomIntro,
    HomeExterior,
    BambooVillage,
    VillageGate,
    NeighborNight
};

class LevelManager
{
public:
    bool LoadInitial(LevelId fallback);
    bool Load(LevelId id);
    void Update(float deltaTime, const GameInput &input);
    void Draw(const Ui &ui) const;
    [[nodiscard]] GameProgress &Progress();
    [[nodiscard]] CombatSystem *ActiveCombat();
    [[nodiscard]] SystemMenuCommand ConsumeSystemMenuCommand();
    [[nodiscard]] bool IsTransitioning() const;

private:
    enum class TransitionPhase
    {
        None,
        FadeOut,
        FadeIn,
        ChapterEnd
    };

    void BeginTransition(LevelId destination);
    void BeginChapterEnd();

    GameProgress progress_;
    std::unique_ptr<Level> current_;
    LevelId pendingLevel_ = LevelId::BedroomIntro;
    TransitionPhase transitionPhase_ = TransitionPhase::None;
    float transitionAlpha_ = 0.0F;
    bool chapterEndPending_ = false;
};
} // namespace game
