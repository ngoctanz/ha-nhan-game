#pragma once

#include <raylib.h>

#include <string>

namespace game
{
struct GameInput;

enum class ChoiceOption
{
    None,
    Yes,
    No
};

struct SystemChoiceRequest
{
    std::string title;
    std::string message;
    std::string reward;
    std::string yesLabel = "CÓ";
    std::string noLabel = "KHÔNG";
};

class SystemChoice
{
public:
    void Start(SystemChoiceRequest request, ChoiceOption defaultOption = ChoiceOption::Yes);
    ChoiceOption Update(const GameInput &input);
    [[nodiscard]] bool IsActive() const;
    [[nodiscard]] ChoiceOption Selected() const;
    [[nodiscard]] const SystemChoiceRequest &Request() const;

    static Rectangle YesBounds();
    static Rectangle NoBounds();

private:
    SystemChoiceRequest request_;
    ChoiceOption selected_ = ChoiceOption::Yes;
    float previousAxis_ = 0.0F;
    bool active_ = false;
};
} // namespace game
