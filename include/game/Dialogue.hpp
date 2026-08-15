#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace game
{
enum class DialogueAction
{
    None,
    StartChickenQuest,
    SpawnShard,
    ReturnToElder,
    CompleteQuest
};

struct DialogueLine
{
    std::string speaker;
    std::string text;
    std::string portrait;
    std::string action;
    std::string sound;
};

class Dialogue
{
public:
    void Start(std::vector<DialogueLine> lines, DialogueAction completion = DialogueAction::None);
    DialogueAction Update(bool advancePressed);
    [[nodiscard]] bool IsActive() const;
    [[nodiscard]] const DialogueLine &Current() const;
    [[nodiscard]] std::uint64_t Revision() const;

private:
    std::vector<DialogueLine> lines_;
    int index_ = 0;
    bool active_ = false;
    DialogueAction completion_ = DialogueAction::None;
    std::uint64_t revision_ = 0;
};
} // namespace game
