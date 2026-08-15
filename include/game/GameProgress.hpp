#pragma once

#include <map>
#include <set>
#include <string>

namespace game
{
class GameProgress
{
public:
    bool Load();
    bool Save() const;

    void SetLocation(std::string levelId, std::string checkpoint);
    void SetPlayerLevel(int level);
    void AddExperience(int amount);
    void AddCurrency(int amount);
    void GrantItem(std::string itemId, int quantity = 1);
    bool RemoveItem(const std::string &itemId, int quantity = 1);
    void UnlockSkill(std::string skillId);
    void SetFlag(std::string flagId, bool value = true);
    void RecordChoice(std::string choiceId, std::string optionId);

    [[nodiscard]] const std::string &CurrentLevel() const;
    [[nodiscard]] const std::string &Checkpoint() const;
    [[nodiscard]] int PlayerLevel() const;
    [[nodiscard]] int Experience() const;
    [[nodiscard]] int Currency() const;
    [[nodiscard]] int ItemCount(const std::string &itemId) const;
    [[nodiscard]] bool HasSkill(const std::string &skillId) const;
    [[nodiscard]] bool Flag(const std::string &flagId) const;
    [[nodiscard]] std::string Choice(const std::string &choiceId) const;
    [[nodiscard]] const std::map<std::string, int> &Inventory() const;
    [[nodiscard]] const std::set<std::string> &UnlockedSkills() const;

private:
    void ResetDefaults();
    bool LoadFrom(const std::string &path);
    [[nodiscard]] std::string SavePath() const;

    std::string currentLevel_ = "bedroom_intro";
    std::string checkpoint_ = "intro_start";
    int playerLevel_ = 0;
    int experience_ = 50;
    int currency_ = 0;
    std::map<std::string, int> inventory_;
    std::set<std::string> unlockedSkills_;
    std::map<std::string, bool> flags_;
    std::map<std::string, std::string> choices_;
};
} // namespace game
