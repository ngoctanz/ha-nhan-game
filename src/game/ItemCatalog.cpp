#include "game/ItemCatalog.hpp"

#include <array>

namespace game
{
namespace
{
constexpr std::array<ItemDefinition, 8> Items = {{
    {"banh_ngo_nong", "Bánh Ngô Còn Nóng", "Thức ăn",
     "Bánh ngô tẩu tẩu mang sang. Vẫn còn ấm và có mùi thơm khiến ký chủ tạm quên nghèo.",
     ItemRarity::Common, ItemFunction::Throw, "van_vat_phi_trich", 1, true},
    {"goi_tan_thu_bi_an", "Gói Tân Thủ Bí Ẩn", "Hộp phần thưởng",
     "Không ai biết bên trong có gì. Bản của Hà Nhân đã bị thu hồi vì hành hung Hệ Thống.",
     ItemRarity::Epic},
    {"manh_bang_hieu_chuong_ga", "Mảnh Hệ Thống (?)", "Vật phẩm nhiệm vụ",
     "Mảnh phát sáng do Gà Linh Khí đánh rơi. Hệ Thống nghi là bảng hiệu chuồng gà, nhưng nhiệm vụ vẫn bắt mang về.",
     ItemRarity::Uncommon},
    {"com_nam", "Cơm Nắm", "Thức ăn", "Một nắm cơm chắc bụng. Phần thưởng kinh điển của làng Hà Gia.", ItemRarity::Common},
    {"keo_boc_do", "Kẹo Bọc Đỏ", "Thức ăn", "Kẹo ngọt gói giấy đỏ, thường dùng để dỗ trẻ con và ký chủ đang cáu.", ItemRarity::Common},
    {"banh_nuong", "Bánh Nướng", "Thức ăn", "Bánh nướng vàng thơm. Ăn được, không tăng tu vi nhưng tăng tinh thần.", ItemRarity::Uncommon},
    {"giay_vai_den", "Giày Vải Đen", "Trang bị chân", "Giày vải nhẹ, hợp chạy việc làng và chạy khỏi chủ nợ.", ItemRarity::Uncommon},
    {"dep_rom", "Dép Rơm", "Trang bị chân", "Dép rơm bình dân. Phòng thủ thấp, âm thanh tấu hài cao.", ItemRarity::Common}
}};
} // namespace

const ItemDefinition *ItemCatalog::Find(std::string_view id)
{
    for (const ItemDefinition &item : Items)
        if (item.id == id) return &item;
    return nullptr;
}

const char *ItemCatalog::RarityName(ItemRarity rarity)
{
    switch (rarity)
    {
        case ItemRarity::Common: return "Phổ Thông";
        case ItemRarity::Uncommon: return "Ít Gặp";
        case ItemRarity::Rare: return "Hiếm";
        case ItemRarity::Epic: return "Sử Thi";
        case ItemRarity::Legendary: return "Truyền Thuyết";
    }
    return "Không Rõ";
}
const char *ItemCatalog::FunctionName(ItemFunction function)
{
    switch (function)
    {
        case ItemFunction::None: return "Không thể sử dụng";
        case ItemFunction::Throw: return "Ném";
    }
    return "Không rõ";
}
} // namespace game
