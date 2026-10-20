#include "RpgMenuStyle.h"

#include <algorithm>

GraphicsRect InsetRpgRectangle(GraphicsRect rectangle, float amount)
{
    return {
        rectangle.x + amount,
        rectangle.y + amount,
        std::max(0.0F, rectangle.width - amount * 2.0F),
        std::max(0.0F, rectangle.height - amount * 2.0F)};
}

void DrawRpgPanel(const Graphics& graphics, GraphicsRect bounds)
{
    graphics.FillRectangle(bounds, {0x07184F, 0.96F});
    graphics.DrawRectangle(bounds, {0xE8B94E, 1.0F}, 5.0F);
    graphics.DrawRectangle(
        InsetRpgRectangle(bounds, 7.0F),
        {0xFFF4C8, 1.0F},
        2.0F);
    graphics.DrawRectangle(
        InsetRpgRectangle(bounds, 11.0F),
        {0x3D70D9, 1.0F},
        2.0F);
}
