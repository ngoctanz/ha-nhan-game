#include "game/Quest.hpp"
#include "game/GameProgress.hpp"

#include <algorithm>
#include <utility>

namespace game
{
namespace
{
const char *DifficultyName(QuestDifficulty difficulty)
{
    switch (difficulty)
    {
        case QuestDifficulty::Easy: return "Dễ";
        case QuestDifficulty::Normal: return "Thường";
        case QuestDifficulty::Hard: return "Khó";
        case QuestDifficulty::Elite: return "Tinh Anh";
        case QuestDifficulty::Legendary: return "Truyền Thuyết";
    }
    return "?";
}

std::string CompletionFlag(const std::string &id)
{
    return "quest_completed." + id;
}
} // namespace

QuestTracker::QuestTracker(GameProgress &progress) : progress_(progress) {}

void QuestTracker::Register(QuestDefinition definition)
{
    if (definition.id.empty() || Find(definition.id) != nullptr) return;
    definitions_.push_back(std::move(definition));
}

bool QuestTracker::Start(const std::string &id, std::string objective, int target)
{
    if (!activeId_.empty() || !IsAvailable(id)) return false;
    activeId_ = id;
    objective_ = std::move(objective);
    target_ = std::max(0, target);
    progressValue_ = 0;
    RefreshDisplayText();
    return true;
}

void QuestTracker::SetObjective(std::string objective, int target)
{
    if (activeId_.empty()) return;
    objective_ = std::move(objective);
    target_ = std::max(0, target);
    progressValue_ = 0;
    RefreshDisplayText();
}

void QuestTracker::SetProgress(int progress)
{
    if (activeId_.empty()) return;
    progressValue_ = target_ > 0 ? std::clamp(progress, 0, target_) : std::max(0, progress);
    RefreshDisplayText();
}

bool QuestTracker::Complete()
{
    const QuestDefinition *active = Find(activeId_);
    if (active == nullptr) return false;
    const QuestDefinition completed = *active;
    progress_.SetFlag(CompletionFlag(completed.id));
    progress_.SetPlayerLevel(progress_.PlayerLevel() + completed.reward.levelGain);
    progress_.AddExperience(completed.reward.experience);
    progress_.AddCurrency(completed.reward.currency);
    for (const auto &[itemId, quantity] : completed.reward.items)
        progress_.GrantItem(itemId, quantity);
    progress_.Save();

    activeId_.clear();
    objective_.clear();
    progressValue_ = 0;
    target_ = 0;
    displayText_.clear();
    return true;
}

bool QuestTracker::IsActive() const { return !activeId_.empty(); }
bool QuestTracker::IsComplete(const std::string &id) const
{
    return progress_.Flag(CompletionFlag(id));
}

bool QuestTracker::IsAvailable(const std::string &id) const
{
    const QuestDefinition *definition = Find(id);
    if (definition == nullptr || IsComplete(id) || progress_.PlayerLevel() < definition->requiredLevel)
        return false;
    return definition->prerequisiteQuest.empty() || IsComplete(definition->prerequisiteQuest);
}

const QuestDefinition *QuestTracker::NextAvailable(QuestCategory category) const
{
    const auto found = std::find_if(definitions_.begin(), definitions_.end(),
        [&](const QuestDefinition &definition) {
            return definition.category == category && IsAvailable(definition.id);
        });
    return found == definitions_.end() ? nullptr : &*found;
}

const std::string &QuestTracker::Id() const
{
    return activeId_;
}

const std::string &QuestTracker::DisplayText() const { return displayText_; }

const QuestDefinition *QuestTracker::Find(const std::string &id) const
{
    const auto found = std::find_if(definitions_.begin(), definitions_.end(),
        [&](const QuestDefinition &definition) { return definition.id == id; });
    return found == definitions_.end() ? nullptr : &*found;
}

void QuestTracker::RefreshDisplayText()
{
    const QuestDefinition *active = Find(activeId_);
    if (active == nullptr)
    {
        displayText_.clear();
        return;
    }
    displayText_ = "[" + std::string(DifficultyName(active->difficulty)) + "] " + objective_;
    if (target_ > 0)
        displayText_ += " (" + std::to_string(progressValue_) + "/" + std::to_string(target_) + ")";
}
} // namespace game
