#include "MainMenu.h"

#include "RpgMenuStyle.h"
#include "../../Platform/Graphics.h"
#include "../../Platform/ImageAsset.h"
#include "../../Framework/Resources/ResourceManager.h"

#include <algorithm>

namespace
{
constexpr GraphicsRect Bounds{420.0F, 216.0F, 440.0F, 268.0F};
constexpr float MinimumOpenHeight = 8.0F;
constexpr float Padding = 24.0F;
constexpr float OptionHeight = 44.0F;
constexpr float PointerWidth = 52.0F;
constexpr float PointerHeight = 33.0F;
constexpr float PointerGap = 10.0F;
}

void MainMenu::Render(
    const Graphics& graphics,
    const ResourceManager& resources) const
{
    if (!open_)
    {
        return;
    }
    if (page_ == MainMenuPage::Inventory)
    {
        inventoryMenu_.Render(
            graphics, resources, openProgress_, pointerImageId_);
        return;
    }
    if (page_ == MainMenuPage::Status)
    {
        statusMenu_.Render(graphics, openProgress_);
        return;
    }
    RenderRoot(graphics, resources);
}

void MainMenu::RenderRoot(
    const Graphics& graphics,
    const ResourceManager& resources) const
{
    const float height = std::max(
        MinimumOpenHeight,
        Bounds.height * openProgress_);
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
    for (std::size_t index = 0; index < rootOptions_.size(); ++index)
    {
        const GraphicsRect row{
            contentX,
            contentY + static_cast<float>(index) * OptionHeight,
            contentWidth,
            OptionHeight};
        if (index == rootSelectedIndex_)
        {
            graphics.FillRectangle(row, {0x275DB8, 0.9F});
            graphics.DrawRectangle(row, {0x6FA2FF, 1.0F}, 2.0F);
            if (const ImageAsset* pointer =
                    resources.GetImage(pointerImageId_);
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
            rootOptions_[index].text,
            {row.x + 8.0F + PointerWidth + PointerGap,
             row.y + 5.0F,
             row.width - PointerWidth - PointerGap - 20.0F,
             row.height},
            26.0F,
            ShowAllCharacters,
            rootOptions_[index].enabled
                ? GraphicsColor{0xFFFFFF, 1.0F}
                : GraphicsColor{0x8B93AA, 1.0F});
    }
    graphics.PopClip();
}
