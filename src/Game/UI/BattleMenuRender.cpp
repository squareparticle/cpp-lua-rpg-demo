#include "BattleMenu.h"

#include "RpgMenuStyle.h"
#include "../../Platform/Graphics.h"
#include "../../Platform/ImageAsset.h"
#include "../../Framework/Resources/ResourceManager.h"

#include <algorithm>
#include <array>
#include <string>

namespace
{
constexpr GraphicsRect CommandBounds{16.0F, 522.0F, 340.0F, 182.0F};
constexpr GraphicsRect TargetBounds{366.0F, 522.0F, 520.0F, 182.0F};
constexpr GraphicsRect StatusBounds{896.0F, 522.0F, 368.0F, 182.0F};
constexpr GraphicsRect VictoryBounds{340.0F, 144.0F, 600.0F, 432.0F};
constexpr float RowHeight = 34.0F;
constexpr float PointerWidth = 42.0F;
constexpr float PointerHeight = 27.0F;
constexpr char PointerImageId[] = "mainMenuPointer";

void DrawPointer(
    const Graphics& graphics,
    const ResourceManager& resources,
    GraphicsRect row)
{
    if (const ImageAsset* pointer = resources.GetImage(PointerImageId);
        pointer != nullptr)
    {
        graphics.DrawImageInRectangle(
            *pointer,
            {row.x + 4.0F, row.y + 3.0F, PointerWidth, PointerHeight});
    }
}

void DrawBar(
    const Graphics& graphics,
    GraphicsRect bounds,
    int value,
    int maximum,
    std::uint32_t color)
{
    graphics.FillRectangle(bounds, {0x111827, 1.0F});
    graphics.DrawRectangle(bounds, {0xFFF4C8, 1.0F}, 2.0F);
    const float ratio = maximum > 0
        ? std::clamp(
            static_cast<float>(value) / static_cast<float>(maximum),
            0.0F,
            1.0F)
        : 0.0F;
    graphics.FillRectangle(
        {bounds.x + 4.0F, bounds.y + 4.0F,
         (bounds.width - 8.0F) * ratio, bounds.height - 8.0F},
        {color, 1.0F});
}
}

void BattleMenu::Render(
    const Graphics& graphics,
    const ResourceManager& resources) const
{
    if (state_ == BattleMenuState::Victory)
    {
        DrawRpgPanel(graphics, VictoryBounds);
        graphics.DrawTextLayout(
            "Victory!",
            {VictoryBounds.x + 30.0F, VictoryBounds.y + 28.0F,
             VictoryBounds.width - 60.0F, 48.0F},
            34.0F,
            ShowAllCharacters);
        graphics.FillRectangle(
            {VictoryBounds.x + 30.0F, VictoryBounds.y + 82.0F,
             VictoryBounds.width - 60.0F, 2.0F},
            {0xE8B94E, 1.0F});
        graphics.DrawTextLayout(
            "Experience gained: " + std::to_string(experienceGained_),
            {VictoryBounds.x + 34.0F, VictoryBounds.y + 104.0F,
             VictoryBounds.width - 68.0F, 34.0F},
            23.0F,
            ShowAllCharacters);
        graphics.DrawTextLayout(
            "Experience until next level: "
                + std::to_string(experienceUntilNextLevel_),
            {VictoryBounds.x + 34.0F, VictoryBounds.y + 142.0F,
             VictoryBounds.width - 68.0F, 34.0F},
            23.0F,
            ShowAllCharacters);
        graphics.DrawTextLayout(
            "Items received:",
            {VictoryBounds.x + 34.0F, VictoryBounds.y + 194.0F,
             VictoryBounds.width - 68.0F, 34.0F},
            23.0F,
            ShowAllCharacters);

        float itemY = VictoryBounds.y + 232.0F;
        if (itemNames_.empty())
        {
            graphics.DrawTextLayout(
                "None",
                {VictoryBounds.x + 60.0F, itemY,
                 VictoryBounds.width - 94.0F, 30.0F},
                21.0F,
                ShowAllCharacters);
        }
        else
        {
            for (const std::string& itemName : itemNames_)
            {
                graphics.DrawTextLayout(
                    "- " + itemName,
                    {VictoryBounds.x + 60.0F, itemY,
                     VictoryBounds.width - 94.0F, 30.0F},
                    21.0F,
                    ShowAllCharacters);
                itemY += 32.0F;
            }
        }
        graphics.DrawTextLayout(
            "Press Enter to continue",
            {VictoryBounds.x + 34.0F,
             VictoryBounds.y + VictoryBounds.height - 58.0F,
             VictoryBounds.width - 68.0F, 32.0F},
            21.0F,
            ShowAllCharacters);
        return;
    }

    DrawRpgPanel(graphics, CommandBounds);
    DrawRpgPanel(graphics, StatusBounds);
    const bool showTargets = state_ == BattleMenuState::Targets
        || state_ == BattleMenuState::TargetChosen;
    const bool showResult = state_ == BattleMenuState::Victory
        || state_ == BattleMenuState::Defeat;
    if (showTargets || showResult)
    {
        DrawRpgPanel(graphics, TargetBounds);
    }

    constexpr std::array<const char*, 4> commands{
        "Attack", "Magic", "Item", "Run"};
    for (std::size_t index = 0; index < commands.size(); ++index)
    {
        const GraphicsRect row{
            CommandBounds.x + 18.0F,
            CommandBounds.y + 20.0F + static_cast<float>(index) * RowHeight,
            CommandBounds.width - 36.0F,
            RowHeight};
        if (index == commandIndex_)
        {
            graphics.FillRectangle(row, {0x275DB8, 0.9F});
            graphics.DrawRectangle(row, {0x6FA2FF, 1.0F}, 2.0F);
            DrawPointer(graphics, resources, row);
        }
        graphics.DrawTextLayout(
            commands[index],
            {row.x + 54.0F, row.y + 1.0F, row.width - 60.0F, row.height},
            24.0F,
            ShowAllCharacters,
            index == 0
                ? GraphicsColor{0xFFFFFF, 1.0F}
                : GraphicsColor{0x8B93AA, 1.0F});
    }

    if (showTargets)
    {
        graphics.DrawTextLayout(
            "Choose target.",
            {TargetBounds.x + 22.0F, TargetBounds.y + 18.0F,
             TargetBounds.width - 44.0F, 32.0F},
            24.0F,
            ShowAllCharacters);
        graphics.FillRectangle(
            {TargetBounds.x + 22.0F, TargetBounds.y + 52.0F,
             TargetBounds.width - 44.0F, 2.0F},
            {0xE8B94E, 1.0F});

        for (std::size_t index = 0; index < targets_.size(); ++index)
        {
            const GraphicsRect row{
                TargetBounds.x + 18.0F,
                TargetBounds.y + 62.0F + static_cast<float>(index) * RowHeight,
                TargetBounds.width - 36.0F,
                RowHeight};
            if (index == targetIndex_)
            {
                graphics.FillRectangle(row, {0x275DB8, 0.9F});
                graphics.DrawRectangle(row, {0x6FA2FF, 1.0F}, 2.0F);
                DrawPointer(graphics, resources, row);
            }
            graphics.DrawTextLayout(
                targets_[index].name,
                {row.x + 54.0F, row.y + 1.0F,
                 row.width - 60.0F, row.height},
                24.0F,
                ShowAllCharacters);
        }
    }
    else if (showResult)
    {
        graphics.DrawTextLayout(
            state_ == BattleMenuState::Victory ? "Victory!" : "Defeat...",
            {TargetBounds.x + 22.0F, TargetBounds.y + 66.0F,
             TargetBounds.width - 44.0F, 48.0F},
            30.0F,
            ShowAllCharacters);
    }

    graphics.DrawTextLayout(
        playerStats_.name,
        {StatusBounds.x + 22.0F, StatusBounds.y + 18.0F,
         StatusBounds.width - 44.0F, 32.0F},
        24.0F,
        ShowAllCharacters);

    const std::string hp = "HP     " + std::to_string(playerStats_.hp)
        + " / " + std::to_string(playerStats_.maxHp);
    const std::string mp = "MP     " + std::to_string(playerStats_.mp)
        + " / " + std::to_string(playerStats_.maxMp);
    graphics.DrawTextLayout(
        hp,
        {StatusBounds.x + 22.0F, StatusBounds.y + 56.0F, 320.0F, 28.0F},
        21.0F,
        ShowAllCharacters,
        {0xFFFFFF, 1.0F});
    DrawBar(
        graphics,
        {StatusBounds.x + 122.0F, StatusBounds.y + 86.0F, 214.0F, 18.0F},
        playerStats_.hp,
        playerStats_.maxHp,
        0x32E85B);
    graphics.DrawTextLayout(
        mp,
        {StatusBounds.x + 22.0F, StatusBounds.y + 112.0F, 320.0F, 28.0F},
        21.0F,
        ShowAllCharacters,
        {0xFFFFFF, 1.0F});
    DrawBar(
        graphics,
        {StatusBounds.x + 122.0F, StatusBounds.y + 142.0F, 214.0F, 18.0F},
        playerStats_.mp,
        playerStats_.maxMp,
        0x28BFEF);
}
