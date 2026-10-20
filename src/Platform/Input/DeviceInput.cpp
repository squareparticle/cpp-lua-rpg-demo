#include "DeviceInput.h"

#include <algorithm>

bool DeviceInput::BindKey(std::string_view action, std::uint32_t key)
{
    if (action.empty() || key >= KeyCount)
    {
        return false;
    }

    auto& keys = bindings_[std::string(action)];
    if (std::ranges::find(keys, key) != keys.end())
    {
        return false;
    }

    keys.push_back(key);
    return true;
}

void DeviceInput::HandleKeyDown(std::uint32_t key) noexcept
{
    if (key >= KeyCount || keyDown_[key])
    {
        return;
    }

    keyDown_[key] = true;
    keyPressed_[key] = true;
}

void DeviceInput::HandleKeyUp(std::uint32_t key) noexcept
{
    if (key < KeyCount)
    {
        keyDown_[key] = false;
    }
}

void DeviceInput::Reset() noexcept
{
    keyDown_.fill(false);
    keyPressed_.fill(false);
}

void DeviceInput::EndFrame() noexcept
{
    keyPressed_.fill(false);
}

bool DeviceInput::IsActionHeld(std::string_view action) const
{
    const auto* keys = FindBindings(action);
    return keys != nullptr && std::ranges::any_of(
        *keys,
        [this](std::uint32_t key) { return keyDown_[key]; });
}

bool DeviceInput::WasActionPressed(std::string_view action) const
{
    const auto* keys = FindBindings(action);
    return keys != nullptr && std::ranges::any_of(
        *keys,
        [this](std::uint32_t key) { return keyPressed_[key]; });
}

const std::vector<std::uint32_t>* DeviceInput::FindBindings(
    std::string_view action) const
{
    const auto binding = bindings_.find(std::string(action));
    return binding == bindings_.end() ? nullptr : &binding->second;
}
