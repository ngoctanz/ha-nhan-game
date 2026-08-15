#pragma once

#include <string>

namespace game
{
class QuestTracker
{
public:
    void Start(std::string id, std::string title, std::string objective, int target = 0);
    void SetObjective(std::string objective, int target = 0);
    void SetProgress(int progress);
    void Complete(std::string finalObjective);

    [[nodiscard]] const std::string &Id() const;
    [[nodiscard]] bool IsComplete() const;
    [[nodiscard]] const std::string &DisplayText() const;

private:
    void RefreshDisplayText();

    std::string id_;
    std::string title_;
    std::string objective_;
    int progress_ = 0;
    int target_ = 0;
    bool complete_ = false;
    std::string displayText_;
};
} // namespace game
