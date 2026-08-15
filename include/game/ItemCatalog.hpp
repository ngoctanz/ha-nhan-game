#pragma once

#include <string_view>

namespace game
{
enum class ItemRarity
{
    Common,
    Uncommon,
    Rare,
    Epic,
    Legendary
};

enum class ItemFunction
{
    None,
    Throw
};

struct ItemDefinition
{
    std::string_view id;
    std::string_view name;
    std::string_view type;
    std::string_view description;
    ItemRarity rarity = ItemRarity::Common;
    ItemFunction function = ItemFunction::None;
    std::string_view requiredSkill;
    int power = 0;
    bool consumable = false;
};

class ItemCatalog
{
public:
    [[nodiscard]] static const ItemDefinition *Find(std::string_view id);
    [[nodiscard]] static const char *RarityName(ItemRarity rarity);
    [[nodiscard]] static const char *FunctionName(ItemFunction function);
};
} // namespace game
