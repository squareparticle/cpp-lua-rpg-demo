#include "../src/Framework/Scenes/SceneChangeRequest.h"
#include "../src/Game/World/GroundRegionMap.h"
#include "../src/Game/World/GroundRegionSceneChange.h"

#include <iostream>
#include <string>

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
    GroundRegion collisionRegion{
        "storageDoor",
        0xFF0000,
        GroundBehavior::Collision,
        GroundParameterMap{
            {"destination", std::string("storageRoom")}}};
    if (!Expect(!BuildSceneChangeRequest(collisionRegion).has_value(),
            "Collision region created a scene change"))
    {
        return 1;
    }

    GroundRegion warpRegion{
        "storageDoor",
        0xFF0000,
        GroundBehavior::Warp,
        GroundParameterMap{
            {"destination", std::string("storageRoom")},
            {"playerSpawnPoint", std::string("mainDoor")},
            {"fade", true}}};
    const std::optional<SceneChangeRequest> request =
        BuildSceneChangeRequest(warpRegion);
    if (!Expect(request.has_value(),
            "Valid warp region did not create a scene change")
        || !Expect(request->source == "storageDoor",
            "Scene change lost its source region")
        || !Expect(request->destination == "storageRoom",
            "Scene change used the wrong destination")
        || !Expect(request->parameters.size() == 3,
            "Scene change did not preserve the complete parameter dictionary"))
    {
        return 1;
    }

    const auto spawnPoint = request->parameters.find("playerSpawnPoint");
    const auto fade = request->parameters.find("fade");
    return Expect(spawnPoint != request->parameters.end()
            && std::get_if<std::string>(&spawnPoint->second) != nullptr
            && *std::get_if<std::string>(&spawnPoint->second) == "mainDoor",
        "Scene change did not preserve its player spawn-point parameter")
        && Expect(fade != request->parameters.end()
                && std::get_if<bool>(&fade->second) != nullptr
                && *std::get_if<bool>(&fade->second),
            "Scene change did not preserve its Boolean parameter")
        ? 0
        : 1;
}
