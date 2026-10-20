#pragma once

#include "WorldScene.h"

#include <string>

class StorageRoomScene final : public WorldScene
{
public:
    StorageRoomScene(
        ResourceManager& resources,
        GlobalState& globalState,
        LuaSystem& luaSystem,
        std::string playerSpawnPointId = {},
        std::optional<PlayerSpawnPosition> playerPosition = {});
};
