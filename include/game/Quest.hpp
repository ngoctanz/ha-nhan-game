#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace game
{
class GameProgress;

enum class QuestCategory
{
    Main,
    Side,
    System,
    Daily
};

enum class QuestDifficulty
{
    Easy,
    Normal,
    Hard,
    Elite,
    Legendary
};

struct QuestReward
{
    int levelGain = 0;
    int experience = 0;
    int currency = 0;
    std::vector<std::pair<std::string, int>> items;
};

struct QuestDefinition
{
    std::string id;
    std::string title;
    QuestCategory category = QuestCategory::Side;
    QuestDifficulty difficulty = QuestDifficulty::Normal;
    int requiredLevel = 0;
    std::string prerequisiteQuest;
    QuestReward reward;
};

class QuestTracker
{
public:
    explicit QuestTracker(GameProgress &progress);

    void Register(QuestDefinition definition);
    bool Start(const std::string &id, std::string objective, int target = 0);
    void SetObjective(std::string objective, int target = 0);
    void SetProgress(int progress);
    bool Complete();

    [[nodiscard]] bool IsActive() const;
    [[nodiscard]] bool IsComplete(const std::string &id) const;
    [[nodiscard]] bool IsAvailable(const std::string &id) const;
    [[nodiscard]] const QuestDefinition *NextAvailable(QuestCategory category) const;
    [[nodiscard]] const std::string &Id() const;
    [[nodiscard]] const std::string &DisplayText() const;

private:
    void RefreshDisplayText();
    [[nodiscard]] const QuestDefinition *Find(const std::string &id) const;

    GameProgress &progress_;
    std::vector<QuestDefinition> definitions_;
    std::string activeId_;
    std::string objective_;
    int progressValue_ = 0;
    int target_ = 0;
    std::string displayText_;
};
} // namespace game
