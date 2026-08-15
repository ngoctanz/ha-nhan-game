#pragma once

#include <raylib.h>

#include <string>
#include <string_view>

namespace game
{
class GameProgress;
class CombatSystem;
class Ui;
struct GameInput;

enum class SystemMenuPage
{
    Closed,
    HostProfile,
    StorageSpace
};

class ItemIconLibrary
{
public:
    ~ItemIconLibrary();
    bool Load();
    [[nodiscard]] const Texture2D *Find(std::string_view itemId) const;

private:
    Texture2D cornCake_ = {};
    Texture2D riceBall_ = {};
    Texture2D candy_ = {};
    Texture2D pastry_ = {};
    Texture2D clothShoes_ = {};
    Texture2D strawSandals_ = {};
};

class HostProfilePanel
{
public:
    ~HostProfilePanel();
    bool Load();
    void Draw(const GameProgress &progress, Font font) const;

private:
    Texture2D avatarBody_ = {};
    Texture2D avatarFace_ = {};
};

class StorageSpacePanel
{
public:
    void Update(const GameInput &input, const GameProgress &progress);
    void Draw(const GameProgress &progress, const ItemIconLibrary &icons, Font font,
              const CombatSystem *combat, std::string_view status) const;
    [[nodiscard]] std::string_view SelectedItemId(const GameProgress &progress) const;
    [[nodiscard]] static Rectangle UseButtonBounds();

private:
    int selectedIndex_ = 0;
};

class SystemMenu
{
public:
    void Bind(GameProgress &progress);
    void BindCombat(CombatSystem *combat);
    void PreviewStorage(float seconds);
    bool Update(const GameInput &input, float deltaTime);
    void Draw(const Ui &ui) const;
    [[nodiscard]] bool IsOpen() const;

private:
    static Rectangle HostButtonBounds();
    static Rectangle StorageButtonBounds();
    static Rectangle CloseButtonBounds();

    GameProgress *progress_ = nullptr;
    CombatSystem *combat_ = nullptr;
    SystemMenuPage page_ = SystemMenuPage::Closed;
    float storagePreviewTimer_ = 0.0F;
    std::string useStatus_;
    HostProfilePanel hostPanel_;
    StorageSpacePanel storagePanel_;
    ItemIconLibrary itemIcons_;
};
} // namespace game
