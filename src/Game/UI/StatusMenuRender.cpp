#include "StatusMenu.h"

#include "RpgMenuStyle.h"
#include "../../Platform/Graphics.h"

#include <algorithm>

namespace
{
constexpr GraphicsRect Bounds{300.0F, 110.0F, 680.0F, 500.0F};
constexpr float MinimumOpenHeight = 8.0F;
constexpr float Padding = 24.0F;
}

void StatusMenu::Render(
    const Graphics& graphics,
    float openProgress) const
{
    const float height = std::max(
        MinimumOpenHeight,
        Bounds.height * openProgress);
    const GraphicsRect currentBounds{
        Bounds.x,
        Bounds.y + (Bounds.height - height) * 0.5F,
        Bounds.width,
        height};
    DrawRpgPanel(graphics, currentBounds);
    graphics.PushClip(InsetRpgRectangle(currentBounds, 13.0F));

    const float contentX = Bounds.x + Padding;
    const float contentY = Bounds.y + Padding;
    const float contentWidth = Bounds.width - Padding * 2.0F;
    graphics.DrawTextLayout(
        "Status",
        {contentX, contentY, contentWidth, 44.0F},
        32.0F,
        ShowAllCharacters);
    graphics.FillRectangle(
        {contentX, contentY + 50.0F, contentWidth, 2.0F},
        {0xE8B94E, 1.0F});

    const std::string statusText =
        "Name:  " + status_.name
        + "\nLevel:  " + std::to_string(status_.level)
        + "\nHP:  " + std::to_string(status_.hp)
        + " / " + std::to_string(status_.maxHp)
        + "\nMP:  " + std::to_string(status_.mp)
        + " / " + std::to_string(status_.maxMp)
        + "\nXP:  " + std::to_string(status_.experience)
        + "\nXP to next level:  "
        + std::to_string(status_.experienceUntilNextLevel)
        + "\nGold:  " + std::to_string(status_.gold);
    graphics.DrawTextLayout(
        statusText,
        {contentX + 18.0F, contentY + 76.0F,
         contentWidth - 36.0F, Bounds.height - 134.0F},
        27.0F,
        ShowAllCharacters);
    graphics.DrawTextLayout(
        "[Esc] Back",
        {Bounds.x + Bounds.width - 150.0F,
         Bounds.y + Bounds.height - 42.0F, 120.0F, 24.0F},
        18.0F,
        ShowAllCharacters,
        {0xC7D2FE, 1.0F});
    graphics.PopClip();
}
