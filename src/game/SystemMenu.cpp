#include "game/SystemMenu.hpp"
#include "game/CombatSystem.hpp"
#include "game/CharacterFacing.hpp"
#include "game/Config.hpp"
#include "game/FaceRenderer.hpp"
#include "game/GameInput.hpp"
#include "game/GameProgress.hpp"
#include "game/ItemCatalog.hpp"
#include "game/TextureAsset.hpp"
#include "game/Ui.hpp"

#include <algorithm>
#include <iterator>
#include <string>

namespace game
{
namespace
{
constexpr Color Ink = {13, 24, 31, 255};
constexpr Color Paper = {236, 243, 239, 255};
constexpr Color Muted = {165, 185, 184, 255};
constexpr Color Accent = {75, 218, 207, 255};
constexpr Rectangle Window = {100.0F, 70.0F, 1080.0F, 580.0F};

bool HostProfileUnlocked(const GameProgress &progress)
{
    return progress.Flag("ui.host_profile_unlocked") || progress.CurrentLevel() != "bedroom_intro";
}

bool StorageSpaceUnlocked(const GameProgress &progress)
{
    return progress.Flag("ui.storage_space_unlocked") || progress.CurrentLevel() != "bedroom_intro";
}

Color RarityColor(ItemRarity rarity)
{
    switch (rarity)
    {
        case ItemRarity::Common: return Color{196, 204, 201, 255};
        case ItemRarity::Uncommon: return Color{91, 210, 126, 255};
        case ItemRarity::Rare: return Color{88, 162, 245, 255};
        case ItemRarity::Epic: return Color{185, 102, 239, 255};
        case ItemRarity::Legendary: return Color{244, 181, 56, 255};
    }
    return Paper;
}

const char *LocationName(const std::string &level)
{
    if (level == "bedroom_intro") return "Phòng ngủ của Hà Nhân";
    if (level == "home_exterior") return "Sân nhà Hà Nhân";
    if (level == "bamboo_village") return "Làng Hà Gia";
    return "Chưa xác định";
}

void DrawWindow(Font font, const char *title, const char *subtitle)
{
    DrawRectangle(0, 0, ScreenWidth, ScreenHeight, Fade(BLACK, 0.58F));
    DrawRectangleRounded({Window.x + 8, Window.y + 12, Window.width, Window.height},
                         0.035F, 12, Fade(BLACK, 0.32F));
    DrawRectangleRounded(Window, 0.035F, 12, Color{15, 28, 36, 252});
    DrawRectangleRoundedLinesEx(Window, 0.035F, 12, 2.0F, Fade(Accent, 0.65F));
    DrawRectangleRounded({Window.x, Window.y, Window.width, 78.0F},
                         0.035F, 12, Color{27, 64, 66, 255});
    DrawRectangle(static_cast<int>(Window.x), static_cast<int>(Window.y + 58),
                  static_cast<int>(Window.width), 20, Color{27, 64, 66, 255});
    DrawTextLine(font, title, {Window.x + 34, Window.y + 17}, 30, Paper);
    DrawTextLine(font, subtitle, {Window.x + 35, Window.y + 50}, 16, Fade(Muted, 0.86F));
}

void DrawItemGlyph(Vector2 center, const Texture2D *icon, const ItemDefinition *definition)
{
    if (icon && icon->id != 0)
    {
        constexpr float size = 66.0F;
        DrawTexturePro(*icon, {0, 0, static_cast<float>(icon->width), static_cast<float>(icon->height)},
                       {center.x - size / 2, center.y - size / 2, size, size},
                       {0, 0}, 0.0F, WHITE);
        return;
    }
    DrawRectangleRounded({center.x - 24, center.y - 20, 48, 40}, 0.18F, 6,
                         Color{128, 91, 54, 255});
    DrawRectangleLinesEx({center.x - 24, center.y - 20, 48, 40}, 2,
                         Color{214, 170, 90, 255});
    DrawLineEx({center.x, center.y - 20}, {center.x, center.y + 20}, 2,
               Color{214, 170, 90, 255});
}

void DrawStatCard(Font font, Rectangle bounds, const char *label, const std::string &value)
{
    DrawRectangleRounded(bounds, 0.08F, 6, Color{12, 37, 46, 238});
    DrawRectangleRoundedLinesEx(bounds, 0.08F, 6, 1.0F, Fade(Accent, 0.34F));
    DrawRectangle(static_cast<int>(bounds.x), static_cast<int>(bounds.y + 15), 4,
                  static_cast<int>(bounds.height - 30), Accent);
    DrawTextLine(font, label, {bounds.x + 20, bounds.y + 13}, 15, Fade(Accent, 0.82F));
    DrawTextLine(font, value, {bounds.x + 20, bounds.y + 40}, 24, Paper);
    DrawLineEx({bounds.x + bounds.width - 34, bounds.y + 10},
               {bounds.x + bounds.width - 10, bounds.y + 10}, 1, Fade(Accent, 0.42F));
    DrawLineEx({bounds.x + bounds.width - 10, bounds.y + 10},
               {bounds.x + bounds.width - 10, bounds.y + 28}, 1, Fade(Accent, 0.42F));
}
} // namespace

ItemIconLibrary::~ItemIconLibrary()
{
    UnloadTextureAsset(cornCake_);
    UnloadTextureAsset(riceBall_);
    UnloadTextureAsset(candy_);
    UnloadTextureAsset(pastry_);
    UnloadTextureAsset(clothShoes_);
    UnloadTextureAsset(strawSandals_);
}

bool ItemIconLibrary::Load()
{
    cornCake_ = LoadTextureAsset("assets/ui/items/banh_ngo.png");
    riceBall_ = LoadTextureAsset("assets/ui/items/com_nam.png");
    candy_ = LoadTextureAsset("assets/ui/items/keo.png");
    pastry_ = LoadTextureAsset("assets/ui/items/banh_nuong.png");
    clothShoes_ = LoadTextureAsset("assets/ui/items/giay_vai.png");
    strawSandals_ = LoadTextureAsset("assets/ui/items/dep_rom.png");
    return cornCake_.id != 0;
}

const Texture2D *ItemIconLibrary::Find(std::string_view itemId) const
{
    if (itemId == "banh_ngo_nong") return &cornCake_;
    if (itemId == "com_nam") return &riceBall_;
    if (itemId == "keo_boc_do") return &candy_;
    if (itemId == "banh_nuong") return &pastry_;
    if (itemId == "giay_vai_den") return &clothShoes_;
    if (itemId == "dep_rom") return &strawSandals_;
    return nullptr;
}

HostProfilePanel::~HostProfilePanel()
{
    UnloadTextureAsset(avatarBody_);
    UnloadTextureAsset(avatarFace_);
}

bool HostProfilePanel::Load()
{
    avatarBody_ = LoadTextureAsset(
        "assets/characters/player/emotions/angry/actions/idle/idle_neutral.png",
        {950.0F, 115.0F, 535.0F, 840.0F}, 267, 420);
    avatarFace_ = LoadTextureAsset("assets/faces/boy_faces/14_blank_stare/frame_01.png");
    return avatarBody_.id != 0 && avatarFace_.id != 0;
}

void HostProfilePanel::Draw(const GameProgress &progress, Font font) const
{
    DrawWindow(font, "HỒ SƠ KÝ CHỦ", "HỆ THỐNG SINH TỒN CỔ ĐẠI  ·  BẢN GHI CÁ NHÂN");
    const Rectangle identity = {130, 170, 300, 430};
    DrawRectangleRounded(identity, 0.035F, 8, Color{8, 27, 35, 245});
    DrawRectangleRoundedLinesEx(identity, 0.035F, 8, 1.5F, Fade(Accent, 0.55F));
    DrawTextLine(font, "ĐỊNH DANH // HN-0001", {152, 187}, 15, Fade(Accent, 0.86F));
    DrawCircleV({280, 330}, 116, Fade(Accent, 0.035F));
    DrawCircleLinesV({280, 330}, 114, Fade(Accent, 0.16F));
    DrawCircleLinesV({280, 330}, 88, Fade(Accent, 0.10F));
    for (int y = 218; y < 456; y += 24)
        DrawLineEx({151, static_cast<float>(y)}, {409, static_cast<float>(y)},
                   1, Fade(Accent, 0.045F));

    BeginScissorMode(151, 216, 258, 246);
    if (avatarBody_.id != 0)
    {
        const CharacterFacing facing(FacingDirection::Right);
        const Rectangle avatarDestination = {143, 172, 274, 431};
        DrawTexturePro(avatarBody_, SourceForFacing(avatarBody_, facing, false),
                       avatarDestination, {0, 0}, 0.0F, WHITE);
        const FaceAnchor anchor = {0.43F, 0.314F, 0.24F, 0.15F,
                                   Color{250, 250, 247, 255}};
        DrawFaceOverlay(&avatarFace_, avatarDestination, anchor,
                        facing.IsFlipped(false));
    }
    EndScissorMode();
    DrawRectangleLinesEx({151, 216, 258, 246}, 1, Fade(Accent, 0.30F));
    DrawRectangle(151, 458, 72, 4, Accent);
    DrawTextLine(font, "HÀ NHÂN", {152, 486}, 28, Paper);
    DrawTextLine(font, "KÝ CHỦ  ·  TÂN THỦ", {153, 526}, 17, Accent);
    DrawTextLine(font, "Đồng bộ dữ liệu: ỔN ĐỊNH", {153, 562}, 14, Fade(Muted, 0.78F));

    DrawTextLine(font, "CHỈ SỐ SINH TỒN", {470, 177}, 17, Fade(Accent, 0.90F));
    DrawLineEx({470, 205}, {1115, 205}, 1, Fade(Accent, 0.30F));
    DrawStatCard(font, {470, 224, 302, 94}, "CẤP ĐỘ",
                 "LV. " + std::to_string(progress.PlayerLevel()));
    DrawStatCard(font, {794, 224, 321, 94}, "KINH NGHIỆM",
                 std::to_string(progress.Experience()) + " / 100");

    const float experienceRatio = std::clamp(progress.Experience() / 100.0F, 0.0F, 1.0F);
    DrawRectangleRounded({470, 337, 645, 14}, 0.5F, 6, Color{5, 20, 27, 255});
    if (experienceRatio > 0.0F)
        DrawRectangleRounded({470, 337, 645 * experienceRatio, 14}, 0.5F, 6, Accent);
    DrawCircleV({470 + 645 * experienceRatio, 344}, 4, WHITE);

    DrawStatCard(font, {470, 378, 302, 104}, "TRẠNG THÁI",
                 progress.Flag("assaulted_system") ? "Bình thường · Bị ghi sổ" : "Bình thường");
    DrawStatCard(font, {794, 378, 321, 104}, "VỊ TRÍ HIỆN TẠI",
                 LocationName(progress.CurrentLevel()));
    DrawRectangleRounded({470, 510, 645, 70}, 0.06F, 6, Fade(Accent, 0.055F));
    DrawRectangleRoundedLinesEx({470, 510, 645, 70}, 0.06F, 6, 1,
                                Fade(Accent, 0.22F));
    DrawTextLine(font, "HỆ THỐNG ĐÁNH GIÁ", {490, 524}, 14, Fade(Muted, 0.82F));
    DrawTextLine(font, "Còn sống. Tạm thời đạt yêu cầu.", {490, 546}, 20, Paper);
}

void StorageSpacePanel::Update(const GameInput &input, const GameProgress &progress)
{
    const int itemCount = static_cast<int>(progress.Inventory().size());
    if (itemCount == 0) { selectedIndex_ = 0; return; }
    if (input.menuUpPressed) selectedIndex_ = (selectedIndex_ + itemCount - 1) % itemCount;
    if (input.menuDownPressed) selectedIndex_ = (selectedIndex_ + 1) % itemCount;

    int index = 0;
    for (const auto &[id, quantity] : progress.Inventory())
    {
        (void)id;
        (void)quantity;
        const int column = index % 4;
        const int row = index / 4;
        const Rectangle slot = {142.0F + column * 118.0F, 190.0F + row * 118.0F, 98.0F, 98.0F};
        if (CheckCollisionPointRec(input.pointerScreen, slot)) selectedIndex_ = index;
        ++index;
    }
    selectedIndex_ = std::clamp(selectedIndex_, 0, itemCount - 1);
}

void StorageSpacePanel::Draw(const GameProgress &progress,
                             const ItemIconLibrary &icons, Font font,
                             const CombatSystem *combat, std::string_view status) const
{
    DrawWindow(font, "KHÔNG GIAN LƯU TRỮ", "Kho riêng nằm ngoài túi áo  ·  Hệ Thống không chịu trách nhiệm thất lạc");
    const Rectangle gridPanel = {128, 166, 508, 430};
    DrawRectangleRounded(gridPanel, 0.05F, 8, Fade(BLACK, 0.20F));
    DrawRectangleRoundedLinesEx(gridPanel, 0.05F, 8, 1.0F, Fade(Accent, 0.18F));

    const int itemCount = static_cast<int>(progress.Inventory().size());
    for (int index = 0; index < 12; ++index)
    {
        const int column = index % 4;
        const int row = index / 4;
        const Rectangle slot = {142.0F + column * 118.0F, 190.0F + row * 118.0F, 98.0F, 98.0F};
        const bool selected = index == selectedIndex_ && index < itemCount;
        DrawRectangleRounded(slot, 0.12F, 6,
                             selected ? Fade(Accent, 0.20F) : Fade(WHITE, 0.035F));
        DrawRectangleRoundedLinesEx(slot, 0.12F, 6, selected ? 2.0F : 1.0F,
                                    selected ? Accent : Fade(WHITE, 0.11F));
    }

    int index = 0;
    for (const auto &[id, quantity] : progress.Inventory())
    {
        const int column = index % 4;
        const int row = index / 4;
        const Vector2 center = {191.0F + column * 118.0F, 234.0F + row * 118.0F};
        const ItemDefinition *definition = ItemCatalog::Find(id);
        DrawItemGlyph(center, icons.Find(id), definition);
        DrawTextLine(font, "x" + std::to_string(quantity), {211.0F + column * 118.0F,
                     259.0F + row * 118.0F}, 16, Paper);
        ++index;
    }

    const Rectangle detail = {666, 166, 486, 430};
    DrawRectangleRounded(detail, 0.05F, 8, Fade(WHITE, 0.045F));
    DrawRectangleRoundedLinesEx(detail, 0.05F, 8, 1.0F, Fade(WHITE, 0.12F));
    if (itemCount == 0)
    {
        DrawTextLine(font, "TRỐNG TRƠN", {808, 330}, 30, Muted);
        DrawTextLine(font, "Nghèo đến mức không gian cũng rộng hơn.", {720, 380}, 20, Fade(Muted, 0.78F));
        return;
    }

    auto selected = progress.Inventory().begin();
    std::advance(selected, std::min(selectedIndex_, itemCount - 1));
    const ItemDefinition *definition = ItemCatalog::Find(selected->first);
    const std::string name = definition ? std::string(definition->name) : selected->first;
    const ItemRarity rarity = definition ? definition->rarity : ItemRarity::Common;
    DrawItemGlyph({748, 245}, icons.Find(selected->first), definition);
    DrawTextLine(font, name, {710, 302}, 29, Paper);
    DrawTextLine(font, ItemCatalog::RarityName(rarity), {710, 344}, 20, RarityColor(rarity));
    DrawTextLine(font, definition ? std::string(definition->type) : "Chưa phân loại",
                 {710, 378}, 18, Muted);
    DrawTextLine(font, "Số lượng: " + std::to_string(selected->second), {710, 414}, 20, Paper);
    DrawWrappedText(font,
                    definition ? std::string(definition->description) : "Hệ Thống chưa có dữ liệu về vật phẩm này.",
                    {710, 448, 390, 70}, 20, 26, Fade(Paper, 0.90F));

    if (definition != nullptr)
    {
        DrawTextLine(font, std::string("Công năng: ") +
                     ItemCatalog::FunctionName(definition->function),
                     {710, 522}, 17, Muted);
        const CombatUseResult result = combat
            ? combat->CanUseItem(progress, selected->first)
            : CombatUseResult::NoEncounter;
        const bool enabled = result == CombatUseResult::Ready;
        const Rectangle use = UseButtonBounds();
        DrawRectangleRounded(use, 0.16F, 6,
                             enabled ? Fade(Accent, 0.72F) : Fade(Muted, 0.14F));
        DrawRectangleRoundedLinesEx(use, 0.16F, 6, 1.5F,
                                    enabled ? Accent : Fade(Muted, 0.28F));
        DrawTextLine(font, "DÙNG", {use.x + 76, use.y + 12}, 20,
                     enabled ? WHITE : Fade(Muted, 0.65F));
        const std::string hint = status.empty()
            ? CombatSystem::UseResultText(result) : std::string(status);
        DrawTextLine(font, hint, {710, 578}, 16,
                     enabled && status.empty() ? Accent : Fade(Paper, 0.72F));
    }
}

std::string_view StorageSpacePanel::SelectedItemId(const GameProgress &progress) const
{
    const int itemCount = static_cast<int>(progress.Inventory().size());
    if (itemCount == 0) return {};
    auto selected = progress.Inventory().begin();
    std::advance(selected, std::min(selectedIndex_, itemCount - 1));
    return selected->first;
}

Rectangle StorageSpacePanel::UseButtonBounds() { return {884, 538, 204, 46}; }

void SystemMenu::Bind(GameProgress &progress)
{
    progress_ = &progress;
    hostPanel_.Load();
    itemIcons_.Load();
}

void SystemMenu::BindCombat(CombatSystem *combat)
{
    combat_ = combat;
}

void SystemMenu::PreviewStorage(float seconds)
{
    page_ = SystemMenuPage::StorageSpace;
    storagePreviewTimer_ = std::max(0.1F, seconds);
    useStatus_.clear();
}

bool SystemMenu::Update(const GameInput &input, float deltaTime)
{
    if (progress_ == nullptr) return false;
    if (storagePreviewTimer_ > 0.0F)
    {
        storagePreviewTimer_ = std::max(0.0F, storagePreviewTimer_ - deltaTime);
        page_ = storagePreviewTimer_ > 0.0F
            ? SystemMenuPage::StorageSpace : SystemMenuPage::Closed;
        return true;
    }
    const bool hostUnlocked = HostProfileUnlocked(*progress_);
    const bool storageUnlocked = StorageSpaceUnlocked(*progress_);
    if ((page_ == SystemMenuPage::HostProfile && !hostUnlocked) ||
        (page_ == SystemMenuPage::StorageSpace && !storageUnlocked))
        page_ = SystemMenuPage::Closed;
    const bool wasOpen = IsOpen();
    if (hostUnlocked && (input.hostMenuPressed ||
        (input.pointerPressed && CheckCollisionPointRec(input.pointerScreen, HostButtonBounds())))
       )
        page_ = page_ == SystemMenuPage::HostProfile ? SystemMenuPage::Closed : SystemMenuPage::HostProfile;
    else if (storageUnlocked && (input.storageMenuPressed ||
             (input.pointerPressed && CheckCollisionPointRec(input.pointerScreen, StorageButtonBounds())))
            )
        page_ = page_ == SystemMenuPage::StorageSpace ? SystemMenuPage::Closed : SystemMenuPage::StorageSpace;
    else if (IsOpen() && (input.menuBackPressed ||
             (input.pointerPressed && CheckCollisionPointRec(input.pointerScreen, CloseButtonBounds()))))
        page_ = SystemMenuPage::Closed;

    if (page_ == SystemMenuPage::StorageSpace)
    {
        storagePanel_.Update(input, *progress_);
        const bool usePressed = input.interactPressed ||
            (input.advanceDialoguePressed && !input.pointerPressed) ||
            (input.pointerPressed &&
             CheckCollisionPointRec(input.pointerScreen, StorageSpacePanel::UseButtonBounds()));
        if (usePressed)
        {
            const std::string_view itemId = storagePanel_.SelectedItemId(*progress_);
            const CombatUseResult result = combat_ && !itemId.empty()
                ? combat_->QueueItemUse(*progress_, itemId)
                : CombatUseResult::NoEncounter;
            useStatus_ = CombatSystem::UseResultText(result);
            if (result == CombatUseResult::Ready) page_ = SystemMenuPage::Closed;
        }
    }
    return wasOpen || IsOpen();
}

void SystemMenu::Draw(const Ui &ui) const
{
    if (progress_ == nullptr) return;
    const Font font = ui.GetFont();
    const auto drawLauncher = [&](Rectangle bounds, bool active, bool hostIcon) {
        DrawRectangleRounded(bounds, 0.20F, 6, active ? Fade(Accent, 0.38F) : Fade(Ink, 0.76F));
        DrawRectangleRoundedLinesEx(bounds, 0.20F, 6, 1.0F, active ? Accent : Fade(WHITE, 0.18F));
        const Color color = active ? WHITE : Paper;
        const Vector2 center = {bounds.x + bounds.width / 2, bounds.y + bounds.height / 2};
        if (hostIcon)
        {
            DrawCircleV({center.x, center.y - 9}, 7, color);
            DrawRectangleRounded({center.x - 12, center.y + 1, 24, 15}, 0.45F, 6, color);
        }
        else
        {
            DrawCircleLinesV(center, 15, color);
            DrawCircleLinesV(center, 9, Fade(color, 0.78F));
            DrawCircleV(center, 3, color);
        }
    };
    if (HostProfileUnlocked(*progress_))
        drawLauncher(HostButtonBounds(), page_ == SystemMenuPage::HostProfile, true);
    if (StorageSpaceUnlocked(*progress_))
        drawLauncher(StorageButtonBounds(), page_ == SystemMenuPage::StorageSpace, false);

    if (page_ == SystemMenuPage::HostProfile) hostPanel_.Draw(*progress_, font);
    else if (page_ == SystemMenuPage::StorageSpace)
        storagePanel_.Draw(*progress_, itemIcons_, font, combat_, useStatus_);
    if (IsOpen())
    {
        const Rectangle close = CloseButtonBounds();
        const Vector2 center = {close.x + close.width / 2, close.y + close.height / 2};
        DrawCircleV(center, 18, Fade(BLACK, 0.32F));
        DrawLineEx({center.x - 6, center.y - 6}, {center.x + 6, center.y + 6}, 3, Paper);
        DrawLineEx({center.x + 6, center.y - 6}, {center.x - 6, center.y + 6}, 3, Paper);
    }
}

bool SystemMenu::IsOpen() const { return page_ != SystemMenuPage::Closed; }
Rectangle SystemMenu::HostButtonBounds() { return {20, 78, 52, 52}; }
Rectangle SystemMenu::StorageButtonBounds() { return {20, 140, 52, 52}; }
Rectangle SystemMenu::CloseButtonBounds() { return {1124, 86, 36, 36}; }
} // namespace game
