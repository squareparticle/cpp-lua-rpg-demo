#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class CollisionMask
{
public:
    static constexpr std::uint32_t BlockedColor = 0x000000;
    static constexpr std::uint32_t WalkableColor = 0xFFFFFF;

    bool Load(const std::wstring& filename);
    std::optional<std::uint32_t> GetColorAt(int x, int y) const noexcept;

private:
    std::uint32_t width_ = 0;
    std::uint32_t height_ = 0;
    std::vector<std::uint8_t> pixels_;
};
