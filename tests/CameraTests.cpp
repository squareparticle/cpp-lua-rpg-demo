#include "../src/Framework/Rendering/Camera.h"

#include <iostream>

namespace
{
bool ExpectPosition(
    const Camera& camera,
    int expectedX,
    int expectedY,
    const char* description)
{
    if (camera.GetX() == expectedX && camera.GetY() == expectedY)
    {
        return true;
    }

    std::cerr << description << ": expected ("
        << expectedX << ", " << expectedY << "), got ("
        << camera.GetX() << ", " << camera.GetY() << ")\n";
    return false;
}
}

int main()
{
    Camera freeCamera;
    if (!freeCamera.Init()
        || !ExpectPosition(freeCamera, 0, 0, "Free camera"))
    {
        return 1;
    }

    Camera shipCamera;
    if (!shipCamera.SetWindow(0.0F, 0.0F, 1254.0F, 1254.0F, 1280.0F, 720.0F)
        || !shipCamera.Init())
    {
        return 1;
    }

    shipCamera.Follow(627.0F, 100.0F);
    if (!ExpectPosition(shipCamera, 0, 0, "Top boundary"))
    {
        return 1;
    }

    shipCamera.Follow(627.0F, 627.0F);
    if (!ExpectPosition(shipCamera, 0, 267, "Vertical tracking"))
    {
        return 1;
    }

    shipCamera.Follow(627.0F, 1200.0F);
    if (!ExpectPosition(shipCamera, 0, 534, "Bottom boundary"))
    {
        return 1;
    }

    Camera wideCamera;
    if (!wideCamera.SetWindow(0.0F, 0.0F, 2000.0F, 1000.0F, 1280.0F, 720.0F)
        || !wideCamera.Init())
    {
        return 1;
    }

    wideCamera.Follow(100.0F, 360.0F);
    if (!ExpectPosition(wideCamera, 0, 0, "Left boundary"))
    {
        return 1;
    }

    wideCamera.Follow(1000.0F, 360.0F);
    if (!ExpectPosition(wideCamera, 360, 0, "Horizontal tracking"))
    {
        return 1;
    }

    wideCamera.Follow(1900.0F, 360.0F);
    if (!ExpectPosition(wideCamera, 720, 0, "Right boundary")
        || !wideCamera.PanTo(640.0F, 160.0F, 2.0F))
    {
        return 1;
    }
    wideCamera.Update(1.0F);
    if (wideCamera.IsPanComplete()
        || !ExpectPosition(wideCamera, 360, -100, "Halfway through pan"))
    {
        return 1;
    }
    wideCamera.Update(5.0F);
    if (!wideCamera.IsPanComplete()
        || !ExpectPosition(wideCamera, 0, -200, "Exact centered pan arrival"))
    {
        return 1;
    }
    wideCamera.Follow(1000.0F, 360.0F);
    return ExpectPosition(wideCamera, 360, 0, "Normal bounded follow restored") ? 0 : 1;
}
