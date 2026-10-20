#pragma once

#include "WorldScene.h"

#include <string>

class BoatScene final : public WorldScene
{
public:
    BoatScene(
        ResourceManager& resources,
        GlobalState& globalState,
        LuaSystem& luaSystem,
        std::string playerSpawnPointId = {},
        std::optional<PlayerSpawnPosition> playerPosition = {});
};
