#include "game/Dialogue.hpp"

#include <utility>

namespace game
{
void Dialogue::Start(std::vector<DialogueLine> lines, DialogueAction completion)
{
    lines_ = std::move(lines);
    index_ = 0;
    active_ = !lines_.empty();
    completion_ = completion;
    ++revision_;
}

DialogueAction Dialogue::Update(bool advancePressed)
{
    if (!active_) return DialogueAction::None;
    if (!advancePressed) return DialogueAction::None;

    ++index_;
    ++revision_;
    if (index_ < static_cast<int>(lines_.size())) return DialogueAction::None;

    active_ = false;
    const DialogueAction result = completion_;
    completion_ = DialogueAction::None;
    return result;
}

bool Dialogue::IsActive() const
{
    return active_;
}

const DialogueLine &Dialogue::Current() const
{
    return lines_[index_];
}

std::uint64_t Dialogue::Revision() const
{
    return revision_;
}
} // namespace game
