#pragma once

#include <string>

enum class TriggerRegionShape
{
    Circle,
    Rectangle
};

struct TriggerRegion
{
    std::string name;
    TriggerRegionShape shape = TriggerRegionShape::Circle;
    float x = 0.0F;
    float y = 0.0F;
    float radius = 0.0F;
    float width = 0.0F;
    float height = 0.0F;

    bool Contains(
        float objectX,
        float objectY,
        float objectScale,
        float pointX,
        float pointY) const noexcept;
    float GetDistanceSquared(
        float objectX,
        float objectY,
        float objectScale,
        float pointX,
        float pointY) const noexcept;
};
