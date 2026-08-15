#include "game/LevelManager.hpp"
#include "game/BambooVillageLevel.hpp"
#include "game/BedroomIntroLevel.hpp"
#include "game/HomeExteriorLevel.hpp"
#include "game/NeighborNightLevel.hpp"
#include "game/VillageGateLevel.hpp"

#include "game/Config.hpp"
#include "game/Ui.hpp"

#include <algorithm>
#include <raylib.h>

namespace game
{
bool LevelManager::LoadInitial(LevelId fallback)
{
    LevelId level = fallback;
    if (progress_.Load())
    {
        if (progress_.CurrentLevel() == "home_exterior") level = LevelId::HomeExterior;
        else if (progress_.CurrentLevel() == "bamboo_village") level = LevelId::BambooVillage;
        else if (progress_.CurrentLevel() == "village_gate") level = LevelId::VillageGate;
        else if (progress_.CurrentLevel() == "neighbor_night") level = LevelId::NeighborNight;
    }
    return Load(level);
}

bool LevelManager::Load(LevelId id)
{
    switch (id)
    {
        case LevelId::BedroomIntro:
            current_ = std::make_unique<BedroomIntroLevel>(progress_);
            break;
        case LevelId::HomeExterior:
            current_ = std::make_unique<HomeExteriorLevel>(progress_);
            break;
        case LevelId::BambooVillage:
            current_ = std::make_unique<BambooVillageLevel>(progress_);
            break;
        case LevelId::VillageGate:
            current_ = std::make_unique<VillageGateLevel>(progress_);
            break;
        case LevelId::NeighborNight:
            current_ = std::make_unique<NeighborNightLevel>(progress_);
            break;
    }
    const bool loaded = current_ && current_->Load();
    if (!loaded) return false;

    const char *levelId = id == LevelId::BedroomIntro ? "bedroom_intro" :
                          id == LevelId::HomeExterior ? "home_exterior" :
                          id == LevelId::BambooVillage ? "bamboo_village" :
                          id == LevelId::VillageGate ? "village_gate" : "neighbor_night";
    if (progress_.CurrentLevel() != levelId)
    {
        progress_.SetLocation(levelId, "level_start");
        progress_.Save();
    }
    return true;
}

void LevelManager::Update(float deltaTime, const GameInput &input)
{
    if (!current_) return;

    constexpr float fadeDuration = 0.55F;
    if (transitionPhase_ == TransitionPhase::FadeOut)
    {
        transitionAlpha_ = std::min(1.0F, transitionAlpha_ + deltaTime / fadeDuration);
        if (transitionAlpha_ >= 1.0F)
        {
            if (chapterEndPending_)
                transitionPhase_ = TransitionPhase::ChapterEnd;
            else if (Load(pendingLevel_))
                transitionPhase_ = TransitionPhase::FadeIn;
        }
        return;
    }
    if (transitionPhase_ == TransitionPhase::FadeIn)
    {
        transitionAlpha_ = std::max(0.0F, transitionAlpha_ - deltaTime / fadeDuration);
        if (transitionAlpha_ <= 0.0F) transitionPhase_ = TransitionPhase::None;
        return;
    }
    if (transitionPhase_ == TransitionPhase::ChapterEnd) return;

    current_->Update(deltaTime, input);
    const LevelTransition transition = current_->RequestedTransition();
    if (transition == LevelTransition::HomeExterior) BeginTransition(LevelId::HomeExterior);
    else if (transition == LevelTransition::BambooVillage)
        BeginTransition(LevelId::BambooVillage);
    else if (transition == LevelTransition::VillageGate)
        BeginTransition(LevelId::VillageGate);
    else if (transition == LevelTransition::NeighborNight)
        BeginTransition(LevelId::NeighborNight);
    else if (transition == LevelTransition::ChapterEnd)
        BeginChapterEnd();
}

void LevelManager::Draw(const Ui &ui) const
{
    if (current_) current_->Draw(ui);
    if (transitionPhase_ != TransitionPhase::None)
        DrawRectangle(0, 0, ScreenWidth, ScreenHeight, Fade(BLACK, transitionAlpha_));
    if (transitionPhase_ == TransitionPhase::ChapterEnd)
    {
        DrawRectangle(0, 0, ScreenWidth, ScreenHeight, BLACK);
        const Font font = ui.GetFont();
        const char *title = "ĐẦU LÀNG";
        const Vector2 titleSize = MeasureTextEx(font, title, 38, 1);
        DrawTextEx(font, title, {ScreenWidth / 2.0F - titleSize.x / 2.0F, 304}, 38, 1,
                   Color{75, 218, 207, 255});
        const char *next = "CÒN TIẾP...";
        const Vector2 nextSize = MeasureTextEx(font, next, 21, 1);
        DrawTextEx(font, next, {ScreenWidth / 2.0F - nextSize.x / 2.0F, 362}, 21, 1,
                   Fade(WHITE, 0.66F));
    }
}

GameProgress &LevelManager::Progress()
{
    return progress_;
}

CombatSystem *LevelManager::ActiveCombat()
{
    return current_ ? current_->ActiveCombat() : nullptr;
}

SystemMenuCommand LevelManager::ConsumeSystemMenuCommand()
{
    return current_ ? current_->ConsumeSystemMenuCommand() : SystemMenuCommand::None;
}

bool LevelManager::IsTransitioning() const
{
    return transitionPhase_ != TransitionPhase::None;
}

void LevelManager::BeginTransition(LevelId destination)
{
    pendingLevel_ = destination;
    chapterEndPending_ = false;
    transitionAlpha_ = 0.0F;
    transitionPhase_ = TransitionPhase::FadeOut;
}

void LevelManager::BeginChapterEnd()
{
    chapterEndPending_ = true;
    transitionAlpha_ = 0.0F;
    transitionPhase_ = TransitionPhase::FadeOut;
}
} // namespace game
