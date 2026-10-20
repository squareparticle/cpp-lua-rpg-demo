#include "TriggerRegion.h"

namespace
{
float GetCenterX(
    const TriggerRegion& region,
    float objectX,
    float objectScale) noexcept
{
    const float localCenter = region.shape == TriggerRegionShape::Circle
        ? region.x
        : region.x + region.width * 0.5F;
    return objectX + localCenter * objectScale;
}

float GetCenterY(
    const TriggerRegion& region,
    float objectY,
    float objectScale) noexcept
{
    const float localCenter = region.shape == TriggerRegionShape::Circle
        ? region.y
        : region.y + region.height * 0.5F;
    return objectY + localCenter * objectScale;
}
}

bool TriggerRegion::Contains(
    float objectX,
    float objectY,
    float objectScale,
    float pointX,
    float pointY) const noexcept
{
    if (shape == TriggerRegionShape::Circle)
    {
        const float deltaX = pointX - GetCenterX(*this, objectX, objectScale);
        const float deltaY = pointY - GetCenterY(*this, objectY, objectScale);
        const float scaledRadius = radius * objectScale;
        return deltaX * deltaX + deltaY * deltaY
            <= scaledRadius * scaledRadius;
    }

    const float left = objectX + x * objectScale;
    const float top = objectY + y * objectScale;
    const float right = left + width * objectScale;
    const float bottom = top + height * objectScale;
    return pointX >= left
        && pointX <= right
        && pointY >= top
        && pointY <= bottom;
}

float TriggerRegion::GetDistanceSquared(
    float objectX,
    float objectY,
    float objectScale,
    float pointX,
    float pointY) const noexcept
{
    const float deltaX = pointX - GetCenterX(*this, objectX, objectScale);
    const float deltaY = pointY - GetCenterY(*this, objectY, objectScale);
    return deltaX * deltaX + deltaY * deltaY;
}
