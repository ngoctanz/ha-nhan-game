#pragma once

#include "game/Dialogue.hpp"
#include "game/GameInput.hpp"
#include "game/SystemChoice.hpp"

#include <raylib.h>

#include <functional>
#include <string>

namespace game
{
class Ui
{
public:
    Ui() = default;
    ~Ui();
    Ui(const Ui &) = delete;
    Ui &operator=(const Ui &) = delete;

    bool Load();
    void DrawHud(const std::string &objective, float controlsHintTimer) const;
    void DrawSystemNotice(const std::string &text, float timer, const Texture2D *mascot) const;
    void DrawSkillUnlock(const std::string &skillName, float timer, float duration,
                         const Texture2D *mascot) const;
    void DrawQuestIssued(const std::string &title, const std::string &objective,
                         const std::string &reward, float timer, float duration,
                         const Texture2D *mascot) const;
    void DrawPrompt(const std::string &prompt) const;
    void DrawDialogue(const DialogueLine &line, const Texture2D *portrait,
                      const Texture2D *portraitFace = nullptr) const;
    void DrawSystemChoice(const SystemChoice &choice, const Texture2D *mascot) const;
    void DrawLevelTitle(float timer) const;
    void DrawMobileControls(const GameInput &input) const;
    [[nodiscard]] Font GetFont() const;

private:
    Font font_ = {};
    Texture2D sneakIcon_ = {};
    bool ownsFont_ = false;
};

void DrawPanel(Rectangle bounds, Color fill, Color border);
void DrawTextLine(Font font, const std::string &text, Vector2 position, float size, Color color);
void DrawWrappedText(Font font, const std::string &text, Rectangle area,
                     float size, float lineHeight, Color color);
} // namespace game
