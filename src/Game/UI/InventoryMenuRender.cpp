#include "InventoryMenu.h"

#include "RpgMenuStyle.h"
#include "../../Platform/Graphics.h"
#include "../../Platform/ImageAsset.h"
#include "../../Framework/Resources/ResourceManager.h"

#include <algorithm>

namespace
{
constexpr GraphicsRect Bounds{150.0F, 100.0F, 980.0F, 520.0F};
constexpr float MinimumOpenHeight = 8.0F;
constexpr float Padding = 24.0F;
constexpr float ListWidth = 380.0F;
constexpr float OptionHeight = 44.0F;
constexpr float PointerWidth = 52.0F;
constexpr float PointerHeight = 33.0F;
constexpr float PointerGap = 10.0F;
}

void InventoryMenu::Render(
    const Graphics& graphics,
    const ResourceManager& resources,
    float openProgress,
    const std::string& pointerImageId) const
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
        "Inventory",
        {contentX, contentY, ListWidth, 40.0F},
        30.0F,
        ShowAllCharacters);
    graphics.FillRectangle(
        {contentX, contentY + 46.0F, contentWidth, 2.0F},
        {0xE8B94E, 1.0F});

    if (entries_.empty())
    {
        graphics.DrawTextLayout(
            "No items",
            {contentX + 12.0F, contentY + 72.0F,
             ListWidth - 24.0F, OptionHeight},
            26.0F,
            ShowAllCharacters,
            {0x8B93AA, 1.0F});
    }
    else
    {
        for (std::size_t index = 0; index < entries_.size(); ++index)
        {
            const GraphicsRect row{
                contentX,
                contentY + 66.0F + static_cast<float>(index) * OptionHeight,
                ListWidth,
                OptionHeight};
            if (index == selectedIndex_)
            {
                graphics.FillRectangle(row, {0x275DB8, 0.9F});
                graphics.DrawRectangle(row, {0x6FA2FF, 1.0F}, 2.0F);
                if (const ImageAsset* pointer =
                        resources.GetImage(pointerImageId);
                    pointer != nullptr)
                {
                    graphics.DrawImageInRectangle(
                        *pointer,
                        {row.x + 8.0F,
                         row.y + (row.height - PointerHeight) * 0.5F,
                         PointerWidth,
                         PointerHeight});
                }
            }
            graphics.DrawTextLayout(
                entries_[index].name,
                {row.x + 8.0F + PointerWidth + PointerGap,
                 row.y + 5.0F,
                 row.width - PointerWidth - PointerGap - 20.0F,
                 row.height},
                26.0F,
                ShowAllCharacters);
        }

        const InventoryMenuEntry& selected = entries_[selectedIndex_];
        const float detailsX = contentX + ListWidth + 42.0F;
        const float detailsWidth = contentWidth - ListWidth - 42.0F;
        graphics.DrawTextLayout(
            selected.name,
            {detailsX, contentY + 66.0F, detailsWidth, 40.0F},
            28.0F,
            ShowAllCharacters);
        graphics.DrawTextLayout(
            selected.description,
            {detailsX, contentY + 120.0F, detailsWidth, 220.0F},
            23.0F,
            ShowAllCharacters);
        graphics.DrawTextLayout(
            "Value: " + std::to_string(selected.value) + " gold",
            {detailsX, contentY + 360.0F, detailsWidth, 34.0F},
            22.0F,
            ShowAllCharacters,
            {0xE8B94E, 1.0F});
    }

    graphics.DrawTextLayout(
        "[Esc] Back",
        {Bounds.x + Bounds.width - 150.0F,
         Bounds.y + Bounds.height - 42.0F, 120.0F, 24.0F},
        18.0F,
        ShowAllCharacters,
        {0xC7D2FE, 1.0F});
    graphics.PopClip();
}
