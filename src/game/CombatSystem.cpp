#include "game/CombatSystem.hpp"

#include "game/GameProgress.hpp"
#include "game/Ui.hpp"

#include <algorithm>
#include <utility>

namespace game
{
void CombatSystem::BeginEncounter(EnemyEncounterDefinition definition)
{
    enemy_ = std::move(definition);
    enemy_.maxHealth = std::max(1, enemy_.maxHealth);
    health_ = enemy_.maxHealth;
    pendingCommand_ = {};
    active_ = true;
}

void CombatSystem::EndEncounter()
{
    active_ = false;
    pendingCommand_ = {};
}

CombatUseResult CombatSystem::CanUseItem(const GameProgress &progress,
                                          std::string_view itemId) const
{
    if (!active_) return CombatUseResult::NoEncounter;
    if (!enemy_.hostile) return CombatUseResult::TargetNotHostile;
    if (health_ <= 0) return CombatUseResult::TargetDefeated;
    if (pendingCommand_.function != ItemFunction::None) return CombatUseResult::CommandPending;
    if (progress.ItemCount(std::string(itemId)) <= 0) return CombatUseResult::MissingItem;
    const ItemDefinition *item = ItemCatalog::Find(itemId);
    if (item == nullptr || item->function == ItemFunction::None)
        return CombatUseResult::UnsupportedItem;
    if (!item->requiredSkill.empty() && !progress.HasSkill(std::string(item->requiredSkill)))
        return CombatUseResult::MissingSkill;
    return CombatUseResult::Ready;
}

CombatUseResult CombatSystem::QueueItemUse(const GameProgress &progress,
                                            std::string_view itemId)
{
    const CombatUseResult result = CanUseItem(progress, itemId);
    if (result != CombatUseResult::Ready) return result;
    const ItemDefinition *item = ItemCatalog::Find(itemId);
    pendingCommand_ = {item->function, std::string(itemId), item->power};
    return CombatUseResult::Ready;
}

CombatCommand CombatSystem::ConsumeCommand()
{
    CombatCommand command = std::move(pendingCommand_);
    pendingCommand_ = {};
    return command;
}

bool CombatSystem::ResolveHit(GameProgress &progress, const CombatCommand &command)
{
    if (!active_ || !enemy_.hostile || health_ <= 0 ||
        command.function == ItemFunction::None) return false;
    const ItemDefinition *item = ItemCatalog::Find(command.itemId);
    if (item == nullptr || item->function != command.function) return false;
    if (item->consumable && !progress.RemoveItem(command.itemId, 1)) return false;
    health_ = std::max(0, health_ - std::max(0, command.power));
    return health_ == 0;
}

void CombatSystem::DrawHud(Font font) const
{
    if (!active_) return;
    const Rectangle bar = {360, 20, 460, 28};
    DrawRectangleRounded({bar.x + 3, bar.y + 4, bar.width, bar.height},
                         0.5F, 10, Fade(BLACK, 0.30F));
    DrawRectangleRounded(bar, 0.5F, 10, Color{45, 25, 31, 238});
    const float ratio = static_cast<float>(health_) / static_cast<float>(enemy_.maxHealth);
    if (ratio > 0.0F)
        DrawRectangleRounded({bar.x, bar.y, bar.width * ratio, bar.height},
                             0.5F, 10, Color{205, 52, 65, 255});
    DrawRectangleRoundedLinesEx(bar, 0.5F, 10, 1.5F, Fade(WHITE, 0.52F));
    DrawTextLine(font, enemy_.name, {bar.x + 16, bar.y + 5}, 16, RAYWHITE);
    DrawTextLine(font, std::to_string(health_) + " / " + std::to_string(enemy_.maxHealth),
                 {bar.x + bar.width - 56, bar.y + 5}, 16, Color{255, 230, 230, 255});
}

bool CombatSystem::IsActive() const { return active_; }
bool CombatSystem::HasHostileTarget() const
{
    return active_ && enemy_.hostile && health_ > 0;
}
int CombatSystem::Health() const { return health_; }
int CombatSystem::MaxHealth() const { return enemy_.maxHealth; }
const std::string &CombatSystem::EnemyName() const { return enemy_.name; }

const char *CombatSystem::UseResultText(CombatUseResult result)
{
    switch (result)
    {
        case CombatUseResult::Ready: return "Có thể sử dụng";
        case CombatUseResult::NoEncounter: return "Chỉ dùng khi đang chiến đấu";
        case CombatUseResult::TargetNotHostile: return "Mục tiêu không phải kẻ địch";
        case CombatUseResult::TargetDefeated: return "Kẻ địch đã bị hạ";
        case CombatUseResult::MissingItem: return "Không còn vật phẩm";
        case CombatUseResult::UnsupportedItem: return "Vật phẩm này không thể dùng";
        case CombatUseResult::MissingSkill: return "Chưa mở khóa kỹ năng phù hợp";
        case CombatUseResult::CommandPending: return "Đang thi triển kỹ năng";
    }
    return "Không thể sử dụng";
}
} // namespace game
