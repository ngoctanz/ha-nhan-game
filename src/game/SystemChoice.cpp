#include "game/SystemChoice.hpp"
#include "game/GameInput.hpp"

#include <raylib.h>

#include <utility>

namespace game
{
void SystemChoice::Start(SystemChoiceRequest request, ChoiceOption defaultOption)
{
    request_ = std::move(request);
    selected_ = defaultOption == ChoiceOption::None ? ChoiceOption::Yes : defaultOption;
    previousAxis_ = 0.0F;
    active_ = true;
}

ChoiceOption SystemChoice::Update(const GameInput &input)
{
    if (!active_) return ChoiceOption::None;

    if (input.moveAxis < -0.5F && previousAxis_ >= -0.5F) selected_ = ChoiceOption::Yes;
    if (input.moveAxis > 0.5F && previousAxis_ <= 0.5F) selected_ = ChoiceOption::No;
    previousAxis_ = input.moveAxis;

    if (input.pointerPressed && !input.pointerConsumedByControls)
    {
        if (CheckCollisionPointRec(input.pointerScreen, YesBounds())) selected_ = ChoiceOption::Yes;
        else if (CheckCollisionPointRec(input.pointerScreen, NoBounds())) selected_ = ChoiceOption::No;
        else return ChoiceOption::None;
        active_ = false;
        return selected_;
    }

    if (input.advanceDialoguePressed)
    {
        active_ = false;
        return selected_;
    }
    return ChoiceOption::None;
}

bool SystemChoice::IsActive() const { return active_; }
ChoiceOption SystemChoice::Selected() const { return selected_; }
const SystemChoiceRequest &SystemChoice::Request() const { return request_; }
Rectangle SystemChoice::YesBounds() { return {418.0F, 480.0F, 205.0F, 64.0F}; }
Rectangle SystemChoice::NoBounds() { return {657.0F, 480.0F, 205.0F, 64.0F}; }
} // namespace game
