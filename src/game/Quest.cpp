#include "game/Quest.hpp"

#include <utility>

namespace game
{
void QuestTracker::Start(std::string id, std::string title, std::string objective, int target)
{
    id_ = std::move(id);
    title_ = std::move(title);
    objective_ = std::move(objective);
    target_ = target;
    progress_ = 0;
    complete_ = false;
    RefreshDisplayText();
}

void QuestTracker::SetObjective(std::string objective, int target)
{
    objective_ = std::move(objective);
    target_ = target;
    progress_ = 0;
    RefreshDisplayText();
}

void QuestTracker::SetProgress(int progress)
{
    progress_ = progress;
    RefreshDisplayText();
}

void QuestTracker::Complete(std::string finalObjective)
{
    objective_ = std::move(finalObjective);
    complete_ = true;
    RefreshDisplayText();
}

const std::string &QuestTracker::Id() const
{
    return id_;
}

bool QuestTracker::IsComplete() const
{
    return complete_;
}

const std::string &QuestTracker::DisplayText() const
{
    return displayText_;
}

void QuestTracker::RefreshDisplayText()
{
    if (target_ > 0 && !complete_)
        displayText_ = objective_ + " (" + std::to_string(progress_) + "/" +
                       std::to_string(target_) + ")";
    else
        displayText_ = objective_;
}
} // namespace game
