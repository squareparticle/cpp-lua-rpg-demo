#include "GroundRegionSceneChange.h"

#include "../../Framework/Scenes/SceneChangeRequest.h"
#include "GroundRegionMap.h"

std::optional<SceneChangeRequest> BuildSceneChangeRequest(
    const GroundRegion& region)
{
    if (region.id.empty() || region.behavior != GroundBehavior::Warp)
    {
        return std::nullopt;
    }

    const auto destination = region.parameters.find("destination");
    if (destination == region.parameters.end())
    {
        return std::nullopt;
    }

    const auto* destinationName =
        std::get_if<std::string>(&destination->second);
    if (destinationName == nullptr || destinationName->empty())
    {
        return std::nullopt;
    }

    return SceneChangeRequest{
        region.id,
        *destinationName,
        region.parameters};
}
