#include "GroundRegionMap.h"

#include <utility>

namespace
{
bool HasNonEmptyStringParameter(
    const GroundParameterMap& parameters,
    std::string_view name) noexcept
{
    const auto parameter = parameters.find(std::string(name));
    if (parameter == parameters.end())
    {
        return false;
    }

    const auto* value = std::get_if<std::string>(&parameter->second);
    return value != nullptr && !value->empty();
}

bool IsValidBehavior(
    GroundBehavior behavior,
    const GroundParameterMap& parameters,
    std::string_view hook) noexcept
{
    // Parameters belong to the region, but each C++ behavior still validates
    // the values it consumes. Collision and walkable ignore extra data.
    return (behavior != GroundBehavior::Warp
            || HasNonEmptyStringParameter(parameters, "destination"))
        && (behavior != GroundBehavior::Trigger || !hook.empty());
}
}

std::optional<GroundBehavior> ParseGroundBehavior(
    std::string_view text) noexcept
{
    if (text == "walkable")
    {
        return GroundBehavior::Walkable;
    }
    if (text == "collision")
    {
        return GroundBehavior::Collision;
    }
    if (text == "warp")
    {
        return GroundBehavior::Warp;
    }
    if (text == "trigger")
    {
        return GroundBehavior::Trigger;
    }
    return std::nullopt;
}

bool GroundRegionMap::AddRegion(
    std::string id,
    std::uint32_t color,
    GroundBehavior behavior,
    GroundParameterMap parameters,
    std::string hook)
{
    if (id.empty()
        || regionsById_.contains(id)
        || idsByColor_.contains(color)
        || !IsValidBehavior(behavior, parameters, hook))
    {
        return false;
    }

    GroundRegion region{
        id,
        color,
        behavior,
        std::move(parameters),
        std::move(hook)};
    regionsById_.emplace(
        id,
        std::move(region));
    idsByColor_.emplace(color, std::move(id));
    return true;
}

bool GroundRegionMap::SetBehavior(
    std::string_view id,
    GroundBehavior behavior)
{
    const auto region = regionsById_.find(std::string(id));
    if (region == regionsById_.end()
        || !IsValidBehavior(
            behavior,
            region->second.parameters,
            region->second.hook))
    {
        return false;
    }

    region->second.behavior = behavior;
    return true;
}

bool GroundRegionMap::SetParameters(
    std::string_view id,
    GroundParameterMap parameters)
{
    const auto region = regionsById_.find(std::string(id));
    if (region == regionsById_.end()
        || !IsValidBehavior(
            region->second.behavior,
            parameters,
            region->second.hook))
    {
        return false;
    }

    region->second.parameters = std::move(parameters);
    return true;
}

const GroundRegion* GroundRegionMap::FindById(
    std::string_view id) const noexcept
{
    const auto region = regionsById_.find(std::string(id));
    return region != regionsById_.end() ? &region->second : nullptr;
}

const GroundRegion* GroundRegionMap::FindByColor(
    std::uint32_t color) const noexcept
{
    const auto id = idsByColor_.find(color);
    return id != idsByColor_.end() ? FindById(id->second) : nullptr;
}

bool GroundRegionMap::BlocksMovement(std::uint32_t color) const noexcept
{
    const GroundRegion* region = FindByColor(color);
    return region != nullptr
        && region->behavior == GroundBehavior::Collision;
}
