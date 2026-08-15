#include "game/Ui.hpp"
#include "game/Config.hpp"
#include "game/FaceRenderer.hpp"
#include "game/TextureAsset.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <sstream>

namespace game
{
namespace
{
constexpr Color Ink = {12, 20, 27, 255};
constexpr Color Paper = {235, 242, 239, 255};
constexpr Color Muted = {184, 198, 197, 255};
constexpr Color Accent = {87, 211, 204, 255};

Font LoadVietnameseFont(bool &ownsFont)
{
    const char *glyphs =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
        " ÀÁẢÃẠĂẰẮẲẴẶÂẦẤẨẪẬĐÈÉẺẼẸÊỀẾỂỄỆÌÍỈĨỊÒÓỎÕỌÔỒỐỔỖỘƠỜỚỞỠỢ"
        "ÙÚỦŨỤƯỪỨỬỮỰỲÝỶỸỴàáảãạăằắẳẵặâầấẩẫậđèéẻẽẹêềếểễệìíỉĩị"
        "òóỏõọôồốổỗộơờớởỡợùúủũụưừứửữựỳýỷỹỵ"
        " .,!?;:()[]+-/=%'\"…·›✓";
    const std::array<const char *, 6> candidates = {
        "assets/fonts/NotoSans-Regular.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/Library/Fonts/Arial.ttf"};
    for (const char *path : candidates)
    {
        if (!FileExists(path)) continue;
        int glyphCount = 0;
        int *codepoints = LoadCodepoints(glyphs, &glyphCount);
        Font font = LoadFontEx(path, 42, codepoints, glyphCount);
        UnloadCodepoints(codepoints);
        if (font.texture.id != 0)
        {
            SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
            ownsFont = true;
            return font;
        }
    }
    ownsFont = false;
    return GetFontDefault();
}

Rectangle DrawPortrait(const Texture2D &texture, Rectangle box,
                       float yOffset = 0.0F, bool flipped = false)
{
    const float scale = std::min(box.width / texture.width, box.height / texture.height);
    const float width = texture.width * scale;
    const float height = texture.height * scale;
    const Rectangle destination = {
        box.x + (box.width - width) / 2.0F,
        box.y + box.height - height + yOffset, width, height};
    Rectangle source = {0, 0, static_cast<float>(texture.width),
                        static_cast<float>(texture.height)};
    if (flipped) source.width = -source.width;
    DrawTexturePro(texture,
                   source,
                   destination,
                   {0, 0}, 0.0F, WHITE);
    return destination;
}

void DrawGlassPanel(Rectangle bounds, float opacity = 0.68F)
{
    DrawRectangleRounded({bounds.x + 3, bounds.y + 5, bounds.width, bounds.height},
                         0.18F, 10, Fade(BLACK, 0.13F));
    DrawRectangleRounded(bounds, 0.18F, 10, Fade(Ink, opacity));
    DrawRectangleRoundedLinesEx(bounds, 0.18F, 10, 1.0F, Fade(WHITE, 0.13F));
}

void DrawTouchButtonBase(Vector2 center, float radius, bool held)
{
    const Color fill = held ? Fade(Accent, 0.38F) : Fade(Ink, 0.32F);
    const Color border = held ? Fade(WHITE, 0.72F) : Fade(WHITE, 0.22F);
    DrawCircleV(center, radius + 3.0F, Fade(BLACK, 0.12F));
    DrawCircleV(center, radius, fill);
    DrawRing(center, radius - 1.5F, radius, 0.0F, 360.0F, 48, border);
}

void DrawDirectionButton(Vector2 center, float radius, bool pointsRight, bool held)
{
    DrawTouchButtonBase(center, radius, held);
    const float direction = pointsRight ? 1.0F : -1.0F;
    const Color color = Fade(Paper, held ? 1.0F : 0.82F);
    const Vector2 tip = {center.x + 9.0F * direction, center.y};
    DrawLineEx({center.x - 7.0F * direction, center.y - 13.0F}, tip, 4.0F, color);
    DrawLineEx(tip, {center.x - 7.0F * direction, center.y + 13.0F}, 4.0F, color);
}

void DrawIconButton(const Texture2D &icon, Vector2 center, float radius, bool held)
{
    DrawTouchButtonBase(center, radius, held);
    if (icon.id == 0) return;
    const float size = held ? radius * 1.24F : radius * 1.15F;
    DrawTexturePro(icon,
                   {0, 0, static_cast<float>(icon.width), static_cast<float>(icon.height)},
                   {center.x - size / 2.0F, center.y - size / 2.0F, size, size},
                   {0, 0}, 0.0F, Fade(WHITE, held ? 1.0F : 0.86F));
}
} // namespace

Ui::~Ui()
{
    UnloadTextureAsset(sneakIcon_);
    UnloadTextureAsset(goofyIcon_);
    if (ownsFont_) UnloadFont(font_);
}

bool Ui::Load()
{
    font_ = LoadVietnameseFont(ownsFont_);
    sneakIcon_ = LoadTextureAsset("assets/ui/mobile/sneak.png");
    goofyIcon_ = LoadTextureAsset("assets/ui/mobile/goofy.png");
    if (sneakIcon_.id != 0) SetTextureFilter(sneakIcon_, TEXTURE_FILTER_BILINEAR);
    if (goofyIcon_.id != 0) SetTextureFilter(goofyIcon_, TEXTURE_FILTER_BILINEAR);
    return font_.texture.id != 0 && sneakIcon_.id != 0 && goofyIcon_.id != 0;
}

void DrawPanel(Rectangle bounds, Color fill, Color border)
{
    DrawRectangleRounded(bounds, 0.13F, 12, fill);
    DrawRectangleRoundedLinesEx(bounds, 0.13F, 12, 1.0F, border);
}

void DrawTextLine(Font font, const std::string &text, Vector2 position, float size, Color color)
{
    DrawTextEx(font, text.c_str(), position, size, 0.5F, color);
}

void DrawWrappedText(Font font, const std::string &text, Rectangle area,
                     float size, float lineHeight, Color color)
{
    std::istringstream stream(text);
    std::string word;
    std::string line;
    float y = area.y;
    while (stream >> word)
    {
        const std::string trial = line.empty() ? word : line + " " + word;
        if (!line.empty() && MeasureTextEx(font, trial.c_str(), size, 0.5F).x > area.width)
        {
            DrawTextLine(font, line, {area.x, y}, size, color);
            line = word;
            y += lineHeight;
            if (y + lineHeight > area.y + area.height) break;
        }
        else
        {
            line = trial;
        }
    }
    if (!line.empty() && y <= area.y + area.height)
        DrawTextLine(font, line, {area.x, y}, size, color);
}

void Ui::DrawHud(const std::string &objective, float controlsHintTimer) const
{
    if (controlsHintTimer > 0.0F)
    {
        const float alpha = std::min(1.0F, controlsHintTimer);
        const Rectangle controls = {18, 16, MobileControlsEnabled() ? 390.0F : 432.0F, 35};
        DrawRectangleRounded({controls.x + 3, controls.y + 5, controls.width, controls.height},
                             0.18F, 10, Fade(BLACK, 0.13F * alpha));
        DrawRectangleRounded(controls, 0.18F, 10, Fade(Ink, 0.54F * alpha));
        DrawRectangleRoundedLinesEx(controls, 0.18F, 10, 1.0F,
                                    Fade(WHITE, 0.13F * alpha));
        DrawCircleV({controls.x + 17, controls.y + 17.5F}, 3.5F,
                    Fade(Accent, 0.9F * alpha));
        const char *controlsText = MobileControlsEnabled()
            ? "Chạm nhân vật để tương tác  ·  Giữ icon để dùng kỹ năng"
            : "A D  đi   ·   Shift  chạy   ·   Ctrl  rón rén   ·   C  tấu hài   ·   E  nói";
        DrawTextLine(font_, controlsText,
                     {controls.x + 29, controls.y + 9}, 14,
                     Fade(Paper, 0.82F * alpha));
    }

    const Rectangle questPanel = {ScreenWidth - 354.0F, 16, 336, 70};
    DrawGlassPanel(questPanel, 0.62F);
    DrawRectangleRounded({questPanel.x + 12, questPanel.y + 12, 3, questPanel.height - 24},
                         1.0F, 4, Fade(Accent, 0.86F));
    DrawTextLine(font_, "HỆ THỐNG  ·  NHIỆM VỤ",
                 {questPanel.x + 26, questPanel.y + 11}, 13, Fade(Accent, 0.88F));
    DrawWrappedText(font_, objective,
                    {questPanel.x + 26, questPanel.y + 32, questPanel.width - 42, 30},
                    17, 20, Paper);
}

void Ui::DrawSystemNotice(const std::string &text, float timer, const Texture2D *mascot) const
{
    if (timer <= 0.0F || text.empty()) return;

    const float alpha = std::clamp(timer * 2.4F, 0.0F, 1.0F);
    const Rectangle panel = {ScreenWidth / 2.0F - 190.0F, 104, 420, 68};
    DrawRectangleRounded({panel.x + 3, panel.y + 5, panel.width, panel.height},
                         0.22F, 10, Fade(BLACK, 0.14F * alpha));
    DrawRectangleRounded(panel, 0.22F, 10, Fade(Ink, 0.66F * alpha));
    DrawRectangleRoundedLinesEx(panel, 0.22F, 10, 1.0F, Fade(WHITE, 0.12F * alpha));
    DrawTextLine(font_, "HỆ THỐNG",
                 {panel.x + 21, panel.y + 10}, 12, Fade(Accent, 0.9F * alpha));
    DrawWrappedText(font_, text,
                    {panel.x + 21, panel.y + 29, panel.width - 39, 34},
                    17, 19, Fade(Paper, alpha));

    if (mascot && mascot->id != 0)
    {
        const float hover = std::sin(static_cast<float>(GetTime()) * 2.5F) * 3.0F;
        DrawEllipse(static_cast<int>(panel.x - 38), static_cast<int>(panel.y + 72),
                    38, 7, Fade(BLACK, 0.13F * alpha));
        DrawPortrait(*mascot, {panel.x - 104, panel.y - 49, 132, 126}, hover);
    }
}

void Ui::DrawPrompt(const std::string &prompt) const
{
    if (prompt.empty()) return;
    const Vector2 size = MeasureTextEx(font_, prompt.c_str(), 17, 0.5F);
    const Rectangle panel = {ScreenWidth / 2.0F - size.x / 2.0F - 17,
                             ScreenHeight - 56.0F, size.x + 34, 34};
    DrawGlassPanel(panel, 0.62F);
    DrawTextLine(font_, prompt, {panel.x + 17, panel.y + 8}, 17, Paper);
}

void Ui::DrawDialogue(const DialogueLine &line, const Texture2D *portrait,
                      const Texture2D *portraitFace) const
{
    DrawRectangle(0, 0, ScreenWidth, ScreenHeight, Fade(BLACK, 0.06F));
    const Rectangle panel = {82, ScreenHeight - 168.0F, ScreenWidth - 164.0F, 132};
    DrawGlassPanel(panel, 0.76F);

    float textX = panel.x + 27;
    if (portrait && portrait->id != 0)
    {
        const Rectangle portraitBox = {panel.x + 4, panel.y - 65, 152, 180};
        const bool elderPortrait = line.portrait.rfind("elder:", 0) == 0;
        // Elder body artwork naturally faces left; portraits on the left side
        // of the panel should face right toward the dialogue text.
        const bool portraitBodyFlipped = elderPortrait;
        const Rectangle portraitDestination =
            DrawPortrait(*portrait, portraitBox, 0.0F, portraitBodyFlipped);
        if (portraitFace && portraitFace->id != 0)
        {
            FaceAnchor anchor = {
                0.43F, 0.314F, 0.24F, 0.15F, Color{250, 250, 247, 255}};
            if (elderPortrait)
                anchor = {0.605F, 0.395F, 0.50F, 0.22F,
                          Color{250, 241, 224, 255}, 0.0F, false, -0.090F};
            // Elder body and face assets both naturally lean left, so both
            // must flip to look right toward the dialogue text.
            const bool portraitFaceFlipped = elderPortrait;
            DrawFaceOverlay(portraitFace, portraitDestination, anchor,
                            portraitBodyFlipped, portraitFaceFlipped);
        }
        textX = panel.x + 166;
    }

    DrawRectangleRounded({textX, panel.y + 17, 3, 25}, 1.0F, 4, Accent);
    DrawTextLine(font_, line.speaker, {textX + 13, panel.y + 17}, 18, Accent);
    DrawWrappedText(font_, line.text,
                    {textX + 13, panel.y + 50, panel.x + panel.width - textX - 35, 48},
                    21, 26, Paper);
    const char *continueHint = MobileControlsEnabled()
        ? "Chạm để tiếp tục  ›"
        : "E / Space  ›";
    const Vector2 hintSize = MeasureTextEx(font_, continueHint, 13, 0.5F);
    DrawTextLine(font_, continueHint,
                 {panel.x + panel.width - hintSize.x - 18.0F,
                  panel.y + panel.height - 25},
                 13, Fade(Muted, 0.72F));
}

void Ui::DrawMobileControls(const GameInput &input) const
{
    if (!MobileControlsEnabled()) return;
    DrawDirectionButton(MobileLayout::MoveLeft, MobileLayout::MoveRadius,
                        false, input.moveAxis < -0.1F);
    DrawDirectionButton(MobileLayout::MoveRight, MobileLayout::MoveRadius,
                        true, input.moveAxis > 0.1F);
    DrawIconButton(sneakIcon_, MobileLayout::Sneak, MobileLayout::ActionRadius,
                   input.sneakHeld);
    DrawIconButton(goofyIcon_, MobileLayout::Goofy, MobileLayout::ActionRadius,
                   input.goofyHeld);
}

void Ui::DrawLevelTitle(float timer) const
{
    if (timer <= 0.0F) return;
    const float alpha = std::min(1.0F, std::min(timer, 4.0F - timer) * 1.5F);
    const std::string chapter = "CHƯƠNG 1  ·  GÀ CHẠY MẤT DÉP";
    const Vector2 size = MeasureTextEx(font_, chapter.c_str(), 25, 0.5F);
    const Rectangle panel = {ScreenWidth / 2.0F - size.x / 2.0F - 22, 232,
                             size.x + 44, 45};
    DrawRectangleRounded(panel, 0.28F, 10, Fade(Ink, 0.48F * alpha));
    DrawTextLine(font_, chapter, {panel.x + 22, panel.y + 10}, 25, Fade(Paper, alpha));
}

Font Ui::GetFont() const
{
    return font_;
}
} // namespace game
