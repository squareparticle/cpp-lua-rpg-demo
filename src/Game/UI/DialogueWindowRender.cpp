#include "DialogueWindow.h"

#include "RpgMenuStyle.h"
#include "../../Platform/Graphics.h"
#include "../../Platform/ImageAsset.h"
#include "../../Framework/Resources/ResourceManager.h"

#include <algorithm>

namespace
{
constexpr GraphicsRect Bounds{64.0F, 500.0F, 1152.0F, 172.0F};
constexpr float MinimumOpenHeight = 8.0F;
constexpr float Padding = 24.0F;
constexpr float PortraitSize = 112.0F;
constexpr float PortraitGap = 20.0F;
constexpr float TextSize = 26.0F;
constexpr float PromptWidth = 150.0F;
constexpr float PromptHeight = 24.0F;
}

void DialogueWindow::Render(
    const Graphics& graphics,
    const ResourceManager& resources) const
{
    if (state_ == DialogueWindowState::Closed)
    {
        return;
    }

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

    float contentX = Bounds.x + Padding;
    const float contentY = Bounds.y + Padding;
    float contentWidth = Bounds.width - Padding * 2.0F;
    if (!portraitImageId_.empty())
    {
        if (const ImageAsset* portrait = resources.GetImage(portraitImageId_);
            portrait != nullptr)
        {
            graphics.DrawImageInRectangle(
                *portrait,
                {contentX, contentY, PortraitSize, PortraitSize});
        }
        contentX += PortraitSize + PortraitGap;
        contentWidth -= PortraitSize + PortraitGap;
    }

    graphics.DrawTextLayout(
        text_,
        {contentX, contentY, contentWidth,
         Bounds.height - Padding * 2.0F - PromptHeight},
        TextSize,
        visibleCharacters_);
    if (state_ == DialogueWindowState::Waiting)
    {
        graphics.DrawTextLayout(
            "[E] Continue",
            {Bounds.x + Bounds.width - Padding - PromptWidth,
             Bounds.y + Bounds.height - Padding - PromptHeight,
             PromptWidth, PromptHeight},
            18.0F,
            ShowAllCharacters);
    }
    graphics.PopClip();
}
