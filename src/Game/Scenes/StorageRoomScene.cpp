#include "StorageRoomScene.h"

#include <utility>

StorageRoomScene::StorageRoomScene(
    ResourceManager& resources,
    GlobalState& globalState,
    LuaSystem& luaSystem,
    std::string playerSpawnPointId,
    std::optional<PlayerSpawnPosition> playerPosition)
    : WorldScene(
          resources,
          "assets/scenes/storageroom/map.yaml",
          "walkableArea",
          globalState,
          luaSystem,
          std::move(playerSpawnPointId),
          playerPosition)
{
}
