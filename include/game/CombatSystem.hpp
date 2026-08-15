#pragma once

#include "game/ItemCatalog.hpp"

#include <raylib.h>
#include <string>
#include <string_view>

namespace game
{
class GameProgress;

enum class CombatUseResult
{
    Ready,
    NoEncounter,
    TargetNotHostile,
    TargetDefeated,
    MissingItem,
    UnsupportedItem,
    MissingSkill,
    CommandPending
};

struct EnemyEncounterDefinition
{
    std::string id;
    std::string name;
    int maxHealth = 1;
    bool hostile = true;
};

struct CombatCommand
{
    ItemFunction function = ItemFunction::None;
    std::string itemId;
    int power = 0;
};

class CombatSystem
{
public:
    void BeginEncounter(EnemyEncounterDefinition definition);
    void EndEncounter();
    [[nodiscard]] CombatUseResult CanUseItem(const GameProgress &progress,
                                              std::string_view itemId) const;
    CombatUseResult QueueItemUse(const GameProgress &progress, std::string_view itemId);
    [[nodiscard]] CombatCommand ConsumeCommand();
    bool ResolveHit(GameProgress &progress, const CombatCommand &command);
    void DrawHud(Font font) const;

    [[nodiscard]] bool IsActive() const;
    [[nodiscard]] bool HasHostileTarget() const;
    [[nodiscard]] int Health() const;
    [[nodiscard]] int MaxHealth() const;
    [[nodiscard]] const std::string &EnemyName() const;
    [[nodiscard]] static const char *UseResultText(CombatUseResult result);

private:
    EnemyEncounterDefinition enemy_;
    CombatCommand pendingCommand_;
    int health_ = 0;
    bool active_ = false;
};
} // namespace game
