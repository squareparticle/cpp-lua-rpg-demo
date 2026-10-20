#include "GameObjectLoad.h"

#include "BattleObjectLoad.h"
#include "PlayerBattle.h"

#include <memory>
#include <utility>

std::unique_ptr<BaseGameObject> GameObjectLoad::PlayerBattle(
    const YAML::Node& node)
{
    std::optional<BattleObjectLoadData> data =
        ReadBattleObjectLoadData(node, false);
    if (!data.has_value())
    {
        return nullptr;
    }

    return std::make_unique<::PlayerBattle>(
        std::move(data->id),
        std::move(data->spriteId),
        std::move(data->animationName),
        data->x,
        data->y,
        data->scale,
        std::move(data->moveIds),
        data->active);
}
