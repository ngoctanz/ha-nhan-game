#pragma once

namespace game
{
class GameProgress;
class CombatSystem;
class Ui;
struct GameInput;

enum class SystemMenuCommand
{
    None,
    PreviewStorage
};

enum class LevelTransition
{
    None,
    HomeExterior,
    BambooVillage,
    VillageGate,
    NeighborNight,
    ChapterEnd
};

class Level
{
public:
    explicit Level(GameProgress &progress) : progress_(progress) {}
    virtual ~Level() = default;
    virtual bool Load() = 0;
    virtual void Update(float deltaTime, const GameInput &input) = 0;
    virtual void Draw(const Ui &ui) const = 0;
    [[nodiscard]] virtual LevelTransition RequestedTransition() const
    {
        return LevelTransition::None;
    }
    [[nodiscard]] virtual CombatSystem *ActiveCombat() { return nullptr; }
    [[nodiscard]] virtual SystemMenuCommand ConsumeSystemMenuCommand()
    {
        return SystemMenuCommand::None;
    }

protected:
    [[nodiscard]] GameProgress &Progress() { return progress_; }
    [[nodiscard]] const GameProgress &Progress() const { return progress_; }

private:
    GameProgress &progress_;
};
} // namespace game
