#include "game/Ui.hpp"
#include "game/AssetLocator.hpp"
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

    // Try the bundled font first via ResolveAssetPath (works on Android where
    // the plain relative path may not match the process CWD).
    const std::string bundledPath = ResolveAssetPath("assets/fonts/NotoSans-Regular.ttf");
    const std::array<std::string, 4> candidates = {
        bundledPath,
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf"};
    for (const std::string &path : candidates)
    {
        // On Android, FileExists does not reliably work for APK assets,
        // so we just attempt to load and check if it succeeded.
        int glyphCount = 0;
        int *codepoints = LoadCodepoints(glyphs, &glyphCount);
        Font font = LoadFontEx(path.c_str(), 72, codepoints, glyphCount);
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
    if (ownsFont_) UnloadFont(font_);
}

bool Ui::Load()
{
    font_ = LoadVietnameseFont(ownsFont_);
    sneakIcon_ = LoadTextureAsset("assets/ui/mobile/sneak.png");
    if (sneakIcon_.id != 0) SetTextureFilter(sneakIcon_, TEXTURE_FILTER_BILINEAR);
    return font_.texture.id != 0 && sneakIcon_.id != 0;
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
        const Rectangle controls = {18, 16, MobileControlsEnabled() ? 480.0F : 580.0F, 45};
        DrawRectangleRounded({controls.x + 3, controls.y + 5, controls.width, controls.height},
                             0.18F, 10, Fade(BLACK, 0.13F * alpha));
        DrawRectangleRounded(controls, 0.18F, 10, Fade(Ink, 0.54F * alpha));
        DrawRectangleRoundedLinesEx(controls, 0.18F, 10, 1.0F,
                                    Fade(WHITE, 0.13F * alpha));
        DrawCircleV({controls.x + 22, controls.y + 22.5F}, 4.5F,
                    Fade(Accent, 0.9F * alpha));
        const char *controlsText = MobileControlsEnabled()
            ? "Chạm nhân vật để tương tác  ·  Nhấn NHẸ để rón rén"
            : "A D  đi   ·   Shift  chạy   ·   Ctrl  rón rén   ·   E  nói";
        DrawTextLine(font_, controlsText,
                     {controls.x + 36, controls.y + 12}, 18,
                     Fade(Paper, 0.82F * alpha));
    }

    if (objective.empty()) return;

    const Rectangle questPanel = {ScreenWidth - 400.0F, 16, 380, 95};
    DrawGlassPanel(questPanel, 0.62F);
    DrawRectangleRounded({questPanel.x + 12, questPanel.y + 12, 4, questPanel.height - 24},
                         1.0F, 4, Fade(Accent, 0.86F));
    DrawTextLine(font_, "HỆ THỐNG  ·  NHIỆM VỤ",
                 {questPanel.x + 28, questPanel.y + 14}, 17, Fade(Accent, 0.88F));
    DrawWrappedText(font_, objective,
                    {questPanel.x + 28, questPanel.y + 38, questPanel.width - 42, 45},
                    24, 28, Paper);
}

void Ui::DrawSystemNotice(const std::string &text, float timer, const Texture2D *mascot) const
{
    if (timer <= 0.0F || text.empty()) return;

    const float alpha = std::clamp(timer * 2.4F, 0.0F, 1.0F);
    const Rectangle panel = {ScreenWidth / 2.0F - 260.0F, 104, 520, 96};
    DrawRectangleRounded({panel.x + 3, panel.y + 5, panel.width, panel.height},
                         0.22F, 10, Fade(BLACK, 0.14F * alpha));
    DrawRectangleRounded(panel, 0.22F, 10, Fade(Ink, 0.66F * alpha));
    DrawRectangleRoundedLinesEx(panel, 0.22F, 10, 1.0F, Fade(WHITE, 0.12F * alpha));
    DrawTextLine(font_, "HỆ THỐNG",
                 {panel.x + 24, panel.y + 12}, 16, Fade(Accent, 0.9F * alpha));
    DrawWrappedText(font_, text,
                    {panel.x + 24, panel.y + 35, panel.width - 44, 48},
                    24, 28, Fade(Paper, alpha));

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
    const Vector2 size = MeasureTextEx(font_, prompt.c_str(), 24, 0.5F);
    const Rectangle panel = {ScreenWidth / 2.0F - size.x / 2.0F - 20,
                             ScreenHeight - 70.0F, size.x + 40, 48};
    DrawGlassPanel(panel, 0.62F);
    DrawTextLine(font_, prompt, {panel.x + 20, panel.y + 12}, 24, Paper);
}

void Ui::DrawDialogue(const DialogueLine &line, const Texture2D *portrait,
                      const Texture2D *portraitFace) const
{
    DrawRectangle(0, 0, ScreenWidth, ScreenHeight, Fade(BLACK, 0.06F));
    const Rectangle panel = {82, ScreenHeight - 200.0F, ScreenWidth - 164.0F, 164};
    DrawGlassPanel(panel, 0.76F);

    float textX = panel.x + 27;
    if (portrait && portrait->id != 0)
    {
        const Rectangle portraitBox = {panel.x + 4, panel.y - 45, 172, 200};
        const bool elderPortrait = line.portrait.rfind("elder:", 0) == 0;
        const bool neighborPortrait = line.portrait.rfind("neighbor:", 0) == 0;
        const bool ruffianPortrait = line.portrait.rfind("ruffian", 0) == 0;
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
            else if (neighborPortrait)
                anchor = {0.555F, 0.314F, 0.22F, 0.16F,
                          Color{250, 250, 247, 255}, 0.0F, false, 0.025F};
            else if (ruffianPortrait)
            {
                static constexpr FaceAnchor ruffianAnchors[] = {
                    {0.519F, 0.300F, 0.19F, 0.15F, Color{250, 238, 220, 255}, 0.0F, false},
                    {0.506F, 0.318F, 0.20F, 0.15F, Color{250, 238, 220, 255}, 0.0F, false},
                    {0.428F, 0.246F, 0.14F, 0.15F, Color{250, 238, 220, 255}, 0.0F, false},
                    {0.495F, 0.280F, 0.20F, 0.16F, Color{250, 238, 220, 255}, 0.0F, false}};
                const int index = line.portrait.size() > 7 && line.portrait[7] >= '0' &&
                                  line.portrait[7] <= '3'
                                      ? line.portrait[7] - '0' : 0;
                anchor = ruffianAnchors[index];
            }
            // Elder body and face assets both naturally lean left, so both
            // must flip to look right toward the dialogue text.
            const bool portraitFaceFlipped = elderPortrait || neighborPortrait;
            DrawFaceOverlay(portraitFace, portraitDestination, anchor,
                            portraitBodyFlipped, portraitFaceFlipped);
        }
        textX = panel.x + 186;
    }

    DrawRectangleRounded({textX, panel.y + 20, 4, 30}, 1.0F, 4, Accent);
    DrawTextLine(font_, line.speaker, {textX + 16, panel.y + 22}, 24, Accent);
    DrawWrappedText(font_, line.text,
                    {textX + 16, panel.y + 60, panel.x + panel.width - textX - 45, 70},
                    28, 34, Paper);
    const char *continueHint = MobileControlsEnabled()
        ? "Chạm để tiếp tục  ›"
        : "E / Space  ›";
    const Vector2 hintSize = MeasureTextEx(font_, continueHint, 18, 0.5F);
    DrawTextLine(font_, continueHint,
                 {panel.x + panel.width - hintSize.x - 22.0F,
                  panel.y + panel.height - 30},
                 18, Fade(Muted, 0.72F));
}

void Ui::DrawSkillUnlock(const std::string &skillName, float timer, float duration,
                         const Texture2D *mascot) const
{
    if (timer <= 0.0F || duration <= 0.0F || skillName.empty()) return;

    const float elapsed = std::max(0.0F, duration - timer);
    const float entrance = std::clamp(elapsed / 0.32F, 0.0F, 1.0F);
    const float exit = std::clamp(timer / 0.48F, 0.0F, 1.0F);
    const float alpha = entrance * exit;
    const float reveal = entrance * entrance * (3.0F - 2.0F * entrance);
    const Rectangle panel = {250.0F, 164.0F, 780.0F, 286.0F};
    const Color gold = {246, 190, 72, 255};
    const Color cyan = {75, 218, 207, 255};

    DrawRectangle(0, 0, ScreenWidth, ScreenHeight, Fade(BLACK, 0.45F * alpha));
    DrawRectangleGradientH(0, static_cast<int>(panel.y + 36), ScreenWidth,
                           static_cast<int>(panel.height - 72),
                           Fade(BLACK, 0.0F), Fade(cyan, 0.06F * alpha));
    DrawRectangleRounded({panel.x + 8, panel.y + 12, panel.width, panel.height},
                         0.055F, 10, Fade(BLACK, 0.36F * alpha));
    DrawRectangleRounded(panel, 0.055F, 10, Fade(Color{8, 22, 30, 255}, 0.96F * alpha));
    DrawRectangleRoundedLinesEx(panel, 0.055F, 10, 2.0F, Fade(cyan, 0.78F * alpha));

    const float scanX = panel.x + 18.0F + (panel.width - 36.0F) *
        std::fmod(elapsed * 0.42F, 1.0F);
    DrawRectangleGradientH(static_cast<int>(scanX - 46), static_cast<int>(panel.y + 4),
                           92, static_cast<int>(panel.height - 8),
                           Fade(cyan, 0.0F), Fade(cyan, 0.16F * alpha));
    for (int i = 0; i < 8; ++i)
    {
        const float y = panel.y + 18.0F + i * 34.0F;
        DrawLineEx({panel.x + 18, y}, {panel.x + panel.width - 18, y}, 1.0F,
                   Fade(WHITE, 0.035F * alpha));
    }

    const Vector2 core = {panel.x + 126.0F, panel.y + panel.height / 2.0F};
    const float pulse = 1.0F + std::sin(elapsed * 6.0F) * 0.06F;
    DrawCircleV(core, 74.0F * pulse, Fade(cyan, 0.045F * alpha));
    DrawCircleLinesV(core, 62.0F * pulse, Fade(cyan, 0.80F * alpha));
    DrawCircleLinesV(core, 47.0F, Fade(gold, 0.72F * alpha));
    DrawPoly(core, 6, 31.0F * reveal, elapsed * 72.0F, Fade(gold, 0.22F * alpha));
    DrawPolyLinesEx(core, 6, 31.0F * reveal, elapsed * 72.0F, 3.0F,
                    Fade(gold, alpha));
    DrawLineEx({core.x - 13, core.y}, {core.x + 13, core.y}, 4.0F,
               Fade(WHITE, alpha));
    DrawLineEx({core.x, core.y - 13}, {core.x, core.y + 13}, 4.0F,
               Fade(WHITE, alpha));

    DrawTextLine(font_, "TING  ·  GIAO THỨC KỸ NĂNG HOÀN TẤT",
                 {panel.x + 226, panel.y + 43}, 18, Fade(cyan, 0.90F * alpha));
    DrawTextLine(font_, "KỸ NĂNG ĐÃ MỞ KHÓA",
                 {panel.x + 226, panel.y + 78}, 27, Fade(Paper, alpha));
    DrawLineEx({panel.x + 226, panel.y + 119},
               {panel.x + 632 * reveal, panel.y + 119}, 2.0F, Fade(cyan, alpha));
    DrawTextLine(font_, skillName, {panel.x + 226, panel.y + 139}, 37,
                 Fade(gold, alpha));
    DrawRectangleRounded({panel.x + 226, panel.y + 199, 112, 34}, 0.35F, 8,
                         Fade(gold, 0.14F * alpha));
    DrawRectangleRoundedLinesEx({panel.x + 226, panel.y + 199, 112, 34},
                                0.35F, 8, 1.0F, Fade(gold, 0.60F * alpha));
    DrawTextLine(font_, "SƠ GIAI", {panel.x + 249, panel.y + 207}, 17,
                 Fade(gold, alpha));
    DrawTextLine(font_, "Vật phẩm tương thích: Bánh Ngô  ·  Công năng: Ném",
                 {panel.x + 354, panel.y + 207}, 17, Fade(Muted, 0.88F * alpha));

    const float corner = 22.0F * reveal;
    const auto drawCorner = [&](Vector2 point, float xDirection, float yDirection) {
        DrawLineEx(point, {point.x + corner * xDirection, point.y}, 3.0F,
                   Fade(gold, alpha));
        DrawLineEx(point, {point.x, point.y + corner * yDirection}, 3.0F,
                   Fade(gold, alpha));
    };
    drawCorner({panel.x + 10, panel.y + 10}, 1, 1);
    drawCorner({panel.x + panel.width - 10, panel.y + 10}, -1, 1);
    drawCorner({panel.x + 10, panel.y + panel.height - 10}, 1, -1);
    drawCorner({panel.x + panel.width - 10, panel.y + panel.height - 10}, -1, -1);

    if (mascot && mascot->id != 0)
    {
        const float hover = std::sin(elapsed * 4.0F) * 3.0F;
        DrawPortrait(*mascot, {panel.x + panel.width - 118, panel.y + 10, 92, 112}, hover);
    }
}

void Ui::DrawQuestIssued(const std::string &title, const std::string &objective,
                         const std::string &reward, float timer, float duration,
                         const Texture2D *mascot) const
{
    if (timer <= 0.0F || duration <= 0.0F || title.empty()) return;
    const float elapsed = std::max(0.0F, duration - timer);
    const float alpha = std::clamp(elapsed / 0.28F, 0.0F, 1.0F) *
                        std::clamp(timer / 0.45F, 0.0F, 1.0F);
    const float reveal = std::clamp(elapsed / 0.42F, 0.0F, 1.0F);
    const Rectangle panel = {305, 155, 670, 330};
    const Color gold = {244, 190, 76, 255};

    DrawRectangle(0, 0, ScreenWidth, ScreenHeight, Fade(BLACK, 0.38F * alpha));
    DrawRectangleRounded({panel.x + 8, panel.y + 11, panel.width, panel.height},
                         0.06F, 10, Fade(BLACK, 0.35F * alpha));
    DrawRectangleRounded(panel, 0.06F, 10, Fade(Color{12, 28, 37, 255}, 0.97F * alpha));
    DrawRectangleRoundedLinesEx(panel, 0.06F, 10, 2.0F, Fade(Accent, 0.78F * alpha));
    DrawRectangleRounded({panel.x, panel.y, panel.width, 66}, 0.06F, 10,
                         Fade(Color{31, 76, 78, 255}, alpha));
    DrawRectangle(panel.x, panel.y + 48, panel.width, 18,
                  Fade(Color{31, 76, 78, 255}, alpha));
    DrawTextLine(font_, "TING  ·  HỆ THỐNG PHÁT NHIỆM VỤ",
                 {panel.x + 30, panel.y + 20}, 22, Fade(Paper, alpha));
    DrawTextLine(font_, title, {panel.x + 34, panel.y + 88}, 34, Fade(gold, alpha));
    DrawLineEx({panel.x + 34, panel.y + 137},
               {panel.x + 470 * reveal, panel.y + 137}, 2.0F, Fade(Accent, alpha));
    DrawTextLine(font_, "MỤC TIÊU", {panel.x + 34, panel.y + 158}, 16,
                 Fade(Muted, alpha));
    DrawWrappedText(font_, objective, {panel.x + 34, panel.y + 184, 450, 56},
                    23, 29, Fade(Paper, alpha));
    DrawRectangleRounded({panel.x + 34, panel.y + 253, 460, 48}, 0.16F, 8,
                         Fade(gold, 0.10F * alpha));
    DrawTextLine(font_, "THƯỞNG", {panel.x + 52, panel.y + 267}, 16, Fade(Muted, alpha));
    DrawTextLine(font_, reward, {panel.x + 142, panel.y + 264}, 21, Fade(gold, alpha));
    if (mascot && mascot->id != 0)
        DrawPortrait(*mascot, {panel.x + 510, panel.y + 78, 130, 214},
                     std::sin(elapsed * 4.0F) * 3.0F);
}

void Ui::DrawSystemChoice(const SystemChoice &choice, const Texture2D *mascot) const
{
    if (!choice.IsActive()) return;
    DrawRectangle(0, 0, ScreenWidth, ScreenHeight, Fade(BLACK, 0.48F));

    const Rectangle panel = {318.0F, 126.0F, 644.0F, 452.0F};
    DrawRectangleRounded({panel.x + 7.0F, panel.y + 10.0F, panel.width, panel.height},
                         0.08F, 10, Fade(BLACK, 0.30F));
    DrawRectangleRounded(panel, 0.08F, 10, Color{18, 30, 39, 248});
    DrawRectangleRoundedLinesEx(panel, 0.08F, 10, 3.0F, Fade(Accent, 0.78F));
    DrawRectangleRounded({panel.x, panel.y, panel.width, 68.0F}, 0.08F, 10,
                         Color{36, 76, 78, 255});
    DrawRectangle(static_cast<int>(panel.x), static_cast<int>(panel.y + 48.0F),
                  static_cast<int>(panel.width), 20, Color{36, 76, 78, 255});

    const SystemChoiceRequest &request = choice.Request();
    DrawTextLine(font_, "TING!  THÔNG BÁO HỆ THỐNG",
                 {panel.x + 32.0F, panel.y + 20.0F}, 26.0F, Paper);
    DrawTextLine(font_, request.title, {panel.x + 34.0F, panel.y + 92.0F}, 30.0F,
                 Color{244, 190, 76, 255});
    DrawWrappedText(font_, request.message,
                    {panel.x + 34.0F, panel.y + 140.0F, 440.0F, 100.0F},
                    24.0F, 31.0F, Paper);

    DrawRectangleRounded({panel.x + 34.0F, panel.y + 260.0F, 420.0F, 70.0F},
                         0.16F, 8, Fade(WHITE, 0.07F));
    DrawTextLine(font_, "PHẦN THƯỞNG", {panel.x + 54.0F, panel.y + 273.0F},
                 18.0F, Muted);
    DrawTextLine(font_, request.reward, {panel.x + 54.0F, panel.y + 299.0F},
                 23.0F, Color{244, 190, 76, 255});

    if (mascot && mascot->id != 0)
        DrawPortrait(*mascot, {panel.x + 474.0F, panel.y + 105.0F, 140.0F, 224.0F});

    const auto drawButton = [&](Rectangle bounds, const std::string &label, bool selected) {
        DrawRectangleRounded(bounds, 0.22F, 8,
                             selected ? Fade(Accent, 0.40F) : Fade(WHITE, 0.07F));
        DrawRectangleRoundedLinesEx(bounds, 0.22F, 8, selected ? 3.0F : 1.0F,
                                    selected ? Accent : Fade(WHITE, 0.22F));
        const Vector2 textSize = MeasureTextEx(font_, label.c_str(), 26.0F, 0.5F);
        DrawTextLine(font_, label,
                     {bounds.x + (bounds.width - textSize.x) / 2.0F,
                      bounds.y + (bounds.height - textSize.y) / 2.0F},
                     26.0F, selected ? WHITE : Paper);
    };
    drawButton(SystemChoice::YesBounds(), request.yesLabel,
               choice.Selected() == ChoiceOption::Yes);
    drawButton(SystemChoice::NoBounds(), request.noLabel,
               choice.Selected() == ChoiceOption::No);
    DrawTextLine(font_, MobileControlsEnabled() ? "Chạm để lựa chọn" : "A/D chọn  ·  E/Space xác nhận",
                 {panel.x + 172.0F, panel.y + 424.0F}, 18.0F, Fade(Muted, 0.72F));
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
}

void Ui::DrawLevelTitle(float timer) const
{
    if (timer <= 0.0F) return;
    const float alpha = std::min(1.0F, std::min(timer, 4.0F - timer) * 1.5F);
    const std::string chapter = "CHƯƠNG 1  ·  GÀ CHẠY MẤT DÉP";
    const Vector2 size = MeasureTextEx(font_, chapter.c_str(), 34, 0.5F);
    const Rectangle panel = {ScreenWidth / 2.0F - size.x / 2.0F - 30, 210,
                             size.x + 60, 60};
    DrawRectangleRounded(panel, 0.28F, 10, Fade(Ink, 0.48F * alpha));
    DrawTextLine(font_, chapter, {panel.x + 30, panel.y + 13}, 34, Fade(Paper, alpha));
}

Font Ui::GetFont() const
{
    return font_;
}
} // namespace game
