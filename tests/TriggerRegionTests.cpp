#include "../src/Game/Objects/TriggerRegion.h"

#include <cmath>
#include <iostream>

namespace
{
bool Expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << message << '\n';
        return false;
    }
    return true;
}

bool NearlyEqual(float left, float right) noexcept
{
    return std::abs(left - right) < 0.001F;
}

bool TestScaledCircle()
{
    TriggerRegion circle;
    circle.name = "interaction";
    circle.shape = TriggerRegionShape::Circle;
    circle.x = 10.0F;
    circle.y = 20.0F;
    circle.radius = 5.0F;

    return Expect(
            circle.Contains(100.0F, 200.0F, 2.0F, 120.0F, 240.0F),
            "Circle did not contain its scaled center")
        && Expect(
            circle.Contains(100.0F, 200.0F, 2.0F, 130.0F, 240.0F),
            "Circle did not include its scaled boundary")
        && Expect(
            !circle.Contains(100.0F, 200.0F, 2.0F, 130.1F, 240.0F),
            "Circle included a point beyond its scaled radius")
        && Expect(NearlyEqual(
                circle.GetDistanceSquared(
                    100.0F,
                    200.0F,
                    2.0F,
                    123.0F,
                    244.0F),
                25.0F),
            "Circle distance was not measured from its scaled center");
}

bool TestScaledRectangle()
{
    TriggerRegion rectangle;
    rectangle.name = "interaction";
    rectangle.shape = TriggerRegionShape::Rectangle;
    rectangle.x = -20.0F;
    rectangle.y = 10.0F;
    rectangle.width = 40.0F;
    rectangle.height = 20.0F;

    return Expect(
            rectangle.Contains(100.0F, 200.0F, 0.5F, 90.0F, 205.0F),
            "Rectangle did not include its top-left boundary")
        && Expect(
            rectangle.Contains(100.0F, 200.0F, 0.5F, 110.0F, 215.0F),
            "Rectangle did not include its bottom-right boundary")
        && Expect(
            !rectangle.Contains(100.0F, 200.0F, 0.5F, 110.1F, 210.0F),
            "Rectangle included a point beyond its right edge")
        && Expect(NearlyEqual(
                rectangle.GetDistanceSquared(
                    100.0F,
                    200.0F,
                    0.5F,
                    103.0F,
                    214.0F),
                25.0F),
            "Rectangle distance was not measured from its scaled center");
}

bool TestStorageDoorRectangleUsesWorldBounds()
{
    TriggerRegion rectangle;
    rectangle.name = "interaction";
    rectangle.shape = TriggerRegionShape::Rectangle;

    // The map authors the door at (501, 308), while trigger coordinates are
    // object-local. These offsets reproduce the requested world-space bounds.
    rectangle.x = -51.0F;
    rectangle.y = 92.0F;
    rectangle.width = 130.0F;
    rectangle.height = 180.0F;

    return Expect(
            rectangle.Contains(501.0F, 308.0F, 1.0F, 450.0F, 400.0F),
            "Storage-door rectangle did not include its world top-left boundary")
        && Expect(
            rectangle.Contains(501.0F, 308.0F, 1.0F, 580.0F, 580.0F),
            "Storage-door rectangle did not include its world bottom-right boundary")
        && Expect(
            !rectangle.Contains(501.0F, 308.0F, 1.0F, 449.9F, 400.0F),
            "Storage-door rectangle extended left of its requested world bounds")
        && Expect(
            !rectangle.Contains(501.0F, 308.0F, 1.0F, 580.1F, 580.0F),
            "Storage-door rectangle extended right of its requested world bounds");
}
}

int main()
{
    return TestScaledCircle()
            && TestScaledRectangle()
            && TestStorageDoorRectangleUsesWorldBounds()
        ? 0
        : 1;
}
