#include "BoatScene.h"

#include <utility>

BoatScene::BoatScene(
    ResourceManager& resources,
    GlobalState& globalState,
    LuaSystem& luaSystem,
    std::string playerSpawnPointId,
    std::optional<PlayerSpawnPosition> playerPosition)
    : WorldScene(
          resources,
          "assets/scenes/boatscene/map.yaml",
          "walkableArea",
          globalState,
          luaSystem,
          std::move(playerSpawnPointId),
          playerPosition)
{
}
