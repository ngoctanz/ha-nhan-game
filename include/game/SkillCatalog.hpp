#pragma once

#include <string_view>

namespace game
{
struct SkillDefinition
{
    std::string_view id;
    std::string_view name;
    std::string_view description;
};

class SkillCatalog
{
public:
    [[nodiscard]] static const SkillDefinition *Find(std::string_view id);
};
} // namespace game
