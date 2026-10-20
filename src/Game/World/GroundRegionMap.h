#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

using GroundParameterValue = std::variant<std::string, double, bool>;
using GroundParameterMap =
    std::unordered_map<std::string, GroundParameterValue>;

enum class GroundBehavior
{
    Walkable,
    Collision,
    Warp,
    Trigger
};

std::optional<GroundBehavior> ParseGroundBehavior(
    std::string_view text) noexcept;

struct GroundRegion
{
    std::string id;
    std::uint32_t color = 0;
    GroundBehavior behavior = GroundBehavior::Walkable;
    GroundParameterMap parameters;
    std::string hook;
};

class GroundRegionMap
{
public:
    // Mask colors identify authored places; this table owns their current
    // gameplay meaning so opening a door never requires repainting the mask.
    bool AddRegion(
        std::string id,
        std::uint32_t color,
        GroundBehavior behavior,
        GroundParameterMap parameters = {},
        std::string hook = {});
    bool SetBehavior(
        std::string_view id,
        GroundBehavior behavior);
    bool SetParameters(
        std::string_view id,
        GroundParameterMap parameters);
    const GroundRegion* FindById(std::string_view id) const noexcept;
    const GroundRegion* FindByColor(std::uint32_t color) const noexcept;
    bool BlocksMovement(std::uint32_t color) const noexcept;

private:
    std::unordered_map<std::string, GroundRegion> regionsById_;
    std::unordered_map<std::uint32_t, std::string> idsByColor_;
};
