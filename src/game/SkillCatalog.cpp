#include "game/SkillCatalog.hpp"

#include <array>

namespace game
{
namespace
{
constexpr std::array<SkillDefinition, 1> Skills = {{
    {"van_vat_phi_trich", "Vạn Vật Phi Trịch",
     "Mượn lực ném vạn vật thành ám khí. Chỉ thi triển khi có vật phẩm hỗ trợ và đang đối đầu kẻ địch."}
}};
}

const SkillDefinition *SkillCatalog::Find(std::string_view id)
{
    for (const SkillDefinition &skill : Skills)
        if (skill.id == id) return &skill;
    return nullptr;
}
} // namespace game
