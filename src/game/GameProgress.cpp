#include "game/GameProgress.hpp"

#include <raylib.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace game
{
namespace
{
constexpr int SaveVersion = 1;

bool ValidId(const std::string &value)
{
    if (value.empty() || value.size() > 96) return false;
    return std::all_of(value.begin(), value.end(), [](unsigned char character) {
        return std::isalnum(character) || character == '_' || character == '-' || character == '.';
    });
}
} // namespace

void GameProgress::ResetDefaults()
{
    currentLevel_ = "bedroom_intro";
    checkpoint_ = "intro_start";
    playerLevel_ = 0;
    experience_ = 50;
    currency_ = 0;
    inventory_.clear();
    unlockedSkills_.clear();
    flags_.clear();
    choices_.clear();
}

std::string GameProgress::SavePath() const
{
#if defined(PLATFORM_ANDROID)
    // raylib maps relative writable files to the app's private internal-data
    // directory on Android, so no storage permission is required.
    return "ha_nhan_progress.sav";
#else
    return (std::filesystem::path(GetWorkingDirectory()) / "save" /
            "ha_nhan_progress.sav").string();
#endif
}

bool GameProgress::Load()
{
    ResetDefaults();
    const std::string path = SavePath();
    if (LoadFrom(path)) return true;
    ResetDefaults();
    if (LoadFrom(path + ".bak")) return true;
    ResetDefaults();
    return false;
}

bool GameProgress::LoadFrom(const std::string &path)
{
    char *savedText = LoadFileText(path.c_str());
    if (savedText == nullptr) return false;
    std::istringstream input(savedText);
    UnloadFileText(savedText);

    std::string magic;
    int version = 0;
    if (!(input >> magic >> version) || magic != "HA_NHAN_SAVE" || version != SaveVersion)
        return false;

    std::string record;
    while (input >> record)
    {
        if (record == "LOCATION")
        {
            std::string level;
            std::string checkpoint;
            if (!(input >> std::quoted(level) >> std::quoted(checkpoint)) ||
                !ValidId(level) || !ValidId(checkpoint)) return false;
            currentLevel_ = std::move(level);
            checkpoint_ = std::move(checkpoint);
        }
        else if (record == "STATS")
        {
            if (!(input >> playerLevel_ >> experience_ >> currency_)) return false;
            playerLevel_ = std::clamp(playerLevel_, 0, 999);
            experience_ = std::clamp(experience_, 0, 99);
            currency_ = std::max(0, currency_);
        }
        else if (record == "ITEM")
        {
            std::string id;
            int quantity = 0;
            if (!(input >> std::quoted(id) >> quantity) || !ValidId(id) || quantity < 1)
                return false;
            inventory_[std::move(id)] = quantity;
        }
        else if (record == "SKILL")
        {
            std::string id;
            if (!(input >> std::quoted(id)) || !ValidId(id)) return false;
            unlockedSkills_.insert(std::move(id));
        }
        else if (record == "FLAG")
        {
            std::string id;
            int value = 0;
            if (!(input >> std::quoted(id) >> value) || !ValidId(id) || (value != 0 && value != 1))
                return false;
            flags_[std::move(id)] = value != 0;
        }
        else if (record == "CHOICE")
        {
            std::string id;
            std::string option;
            if (!(input >> std::quoted(id) >> std::quoted(option)) ||
                !ValidId(id) || !ValidId(option)) return false;
            choices_[std::move(id)] = std::move(option);
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool GameProgress::Save() const
{
    const std::string path = SavePath();
#if !defined(PLATFORM_ANDROID)
    std::error_code directoryError;
    std::filesystem::create_directories(std::filesystem::path(path).parent_path(), directoryError);
    if (directoryError) return false;
#endif

    std::ostringstream output;
    output << "HA_NHAN_SAVE " << SaveVersion << '\n';
    output << "LOCATION " << std::quoted(currentLevel_) << ' ' << std::quoted(checkpoint_) << '\n';
    output << "STATS " << playerLevel_ << ' ' << experience_ << ' ' << currency_ << '\n';
    for (const auto &[id, quantity] : inventory_)
        output << "ITEM " << std::quoted(id) << ' ' << quantity << '\n';
    for (const std::string &id : unlockedSkills_)
        output << "SKILL " << std::quoted(id) << '\n';
    for (const auto &[id, value] : flags_)
        output << "FLAG " << std::quoted(id) << ' ' << (value ? 1 : 0) << '\n';
    for (const auto &[id, option] : choices_)
        output << "CHOICE " << std::quoted(id) << ' ' << std::quoted(option) << '\n';
    std::string serialized = output.str();

    // Keep the last readable snapshot before replacing the primary file. On
    // Android both names are mapped by raylib into private app storage.
    char *previous = LoadFileText(path.c_str());
    if (previous != nullptr)
    {
        const bool backupSaved = SaveFileText((path + ".bak").c_str(), previous);
        UnloadFileText(previous);
        if (!backupSaved) return false;
    }
    return SaveFileText(path.c_str(), serialized.data());
}

void GameProgress::SetLocation(std::string levelId, std::string checkpoint)
{
    if (ValidId(levelId)) currentLevel_ = std::move(levelId);
    if (ValidId(checkpoint)) checkpoint_ = std::move(checkpoint);
}

void GameProgress::SetPlayerLevel(int level) { playerLevel_ = std::clamp(level, 0, 999); }

void GameProgress::AddExperience(int amount)
{
    if (amount <= 0) return;
    experience_ += amount;
    while (experience_ >= 100 && playerLevel_ < 999)
    {
        experience_ -= 100;
        ++playerLevel_;
    }
    if (playerLevel_ == 999) experience_ = std::min(experience_, 99);
}

void GameProgress::AddCurrency(int amount) { currency_ = std::max(0, currency_ + amount); }

void GameProgress::GrantItem(std::string itemId, int quantity)
{
    if (!ValidId(itemId) || quantity <= 0) return;
    inventory_[std::move(itemId)] += quantity;
}

bool GameProgress::RemoveItem(const std::string &itemId, int quantity)
{
    const auto found = inventory_.find(itemId);
    if (found == inventory_.end() || quantity <= 0 || found->second < quantity) return false;
    found->second -= quantity;
    if (found->second == 0) inventory_.erase(found);
    return true;
}

void GameProgress::UnlockSkill(std::string skillId)
{
    if (ValidId(skillId)) unlockedSkills_.insert(std::move(skillId));
}

void GameProgress::SetFlag(std::string flagId, bool value)
{
    if (ValidId(flagId)) flags_[std::move(flagId)] = value;
}

void GameProgress::RecordChoice(std::string choiceId, std::string optionId)
{
    if (ValidId(choiceId) && ValidId(optionId)) choices_[std::move(choiceId)] = std::move(optionId);
}

const std::string &GameProgress::CurrentLevel() const { return currentLevel_; }
const std::string &GameProgress::Checkpoint() const { return checkpoint_; }
int GameProgress::PlayerLevel() const { return playerLevel_; }
int GameProgress::Experience() const { return experience_; }
int GameProgress::Currency() const { return currency_; }
int GameProgress::ItemCount(const std::string &itemId) const
{
    const auto found = inventory_.find(itemId);
    return found == inventory_.end() ? 0 : found->second;
}
bool GameProgress::HasSkill(const std::string &skillId) const { return unlockedSkills_.contains(skillId); }
bool GameProgress::Flag(const std::string &flagId) const
{
    const auto found = flags_.find(flagId);
    return found != flags_.end() && found->second;
}
std::string GameProgress::Choice(const std::string &choiceId) const
{
    const auto found = choices_.find(choiceId);
    return found == choices_.end() ? std::string{} : found->second;
}
const std::map<std::string, int> &GameProgress::Inventory() const { return inventory_; }
const std::set<std::string> &GameProgress::UnlockedSkills() const { return unlockedSkills_; }
} // namespace game
