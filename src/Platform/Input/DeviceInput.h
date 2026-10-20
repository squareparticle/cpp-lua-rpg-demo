#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class DeviceInput
{
public:
    bool BindKey(std::string_view action, std::uint32_t key);

    void HandleKeyDown(std::uint32_t key) noexcept;
    void HandleKeyUp(std::uint32_t key) noexcept;
    void Reset() noexcept;
    void EndFrame() noexcept;

    bool IsActionHeld(std::string_view action) const;
    bool WasActionPressed(std::string_view action) const;

private:
    static constexpr std::size_t KeyCount = 256;

    const std::vector<std::uint32_t>* FindBindings(
        std::string_view action) const;

    std::unordered_map<std::string, std::vector<std::uint32_t>> bindings_;
    std::array<bool, KeyCount> keyDown_{};
    std::array<bool, KeyCount> keyPressed_{};
};
