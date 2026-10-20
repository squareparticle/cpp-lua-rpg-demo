#pragma once

#include "../../Framework/Scenes/SceneChangeRequest.h"

#include <string>
#include <vector>

struct BattleRequest
{
    // Lua produces this one-shot request to describe which YAML-defined
    // enemies should be active in the next BattleArenaScene. GameController
    // consumes the request and replaces the current scene with that arena.
    //
    // This remains a value request rather than a scene pointer or stack entry.
    // WorldScene records its ID and the player's exact object origin before
    // GameController replaces it with the battle arena.
    std::vector<std::string> enemyIds;
    std::string returnSceneId;
    SceneParameterMap returnParameters;
};
