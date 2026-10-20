#include "../src/Platform/Input/DeviceInput.h"

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
}

int main()
{
    constexpr std::uint32_t LeftKey = 10;
    constexpr std::uint32_t AlternateLeftKey = 11;
    constexpr std::uint32_t SharedKey = 12;

    DeviceInput input;
    if (!Expect(input.BindKey("moveLeft", LeftKey),
            "Could not bind the primary key")
        || !Expect(input.BindKey("moveLeft", AlternateLeftKey),
            "Could not bind an alternate key")
        || !Expect(input.BindKey("primaryAction", SharedKey),
            "Could not bind the shared key to the first action")
        || !Expect(input.BindKey("confirm", SharedKey),
            "Could not bind the shared key to the second action")
        || !Expect(!input.BindKey("moveLeft", LeftKey),
            "Duplicate binding was accepted")
        || !Expect(!input.BindKey("", LeftKey),
            "Empty action name was accepted")
        || !Expect(!input.BindKey("invalid", 256),
            "Out-of-range key was accepted"))
    {
        return 1;
    }

    input.HandleKeyDown(LeftKey);
    if (!Expect(input.IsActionHeld("moveLeft"),
            "Key-down did not hold its action")
        || !Expect(input.WasActionPressed("moveLeft"),
            "Key-down did not create a press edge"))
    {
        return 1;
    }

    input.EndFrame();
    input.HandleKeyDown(LeftKey);
    if (!Expect(input.IsActionHeld("moveLeft"),
            "Held action was cleared at frame end")
        || !Expect(!input.WasActionPressed("moveLeft"),
            "Repeated key-down created another press edge"))
    {
        return 1;
    }

    input.HandleKeyDown(AlternateLeftKey);
    input.HandleKeyUp(LeftKey);
    if (!Expect(input.IsActionHeld("moveLeft"),
            "Releasing one binding cleared an alternate held binding")
        || !Expect(input.WasActionPressed("moveLeft"),
            "Alternate binding did not create a press edge"))
    {
        return 1;
    }

    input.HandleKeyDown(SharedKey);
    if (!Expect(input.IsActionHeld("primaryAction")
            && input.IsActionHeld("confirm"),
            "One key did not hold both bound actions")
        || !Expect(input.WasActionPressed("primaryAction")
            && input.WasActionPressed("confirm"),
            "One key did not press both bound actions"))
    {
        return 1;
    }

    input.Reset();
    return Expect(!input.IsActionHeld("moveLeft")
            && !input.WasActionPressed("moveLeft")
            && !input.IsActionHeld("primaryAction")
            && !input.WasActionPressed("confirm"),
        "Reset did not clear held and pressed input")
        && Expect(!input.IsActionHeld("unknown")
                && !input.WasActionPressed("unknown"),
            "Unknown action reported input")
        ? 0
        : 1;
}
