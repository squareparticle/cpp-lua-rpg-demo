#include "GameObjectLoad.h"

#include "BattleEnemy.h"
#include "BattleObjectLoad.h"

#include <memory>
#include <utility>

std::unique_ptr<BaseGameObject> GameObjectLoad::BattleEnemy(
    const YAML::Node& node)
{
    std::optional<BattleObjectLoadData> data =
        ReadBattleObjectLoadData(node);
    if (!data.has_value())
    {
        return nullptr;
    }
    if (data->battleScript.empty() || data->experienceReward < 0)
    {
        return nullptr;
    }

    return std::make_unique<::BattleEnemy>(
        std::move(data->id),
        std::move(data->spriteId),
        std::move(data->animationName),
        data->x,
        data->y,
        data->scale,
        std::move(data->stats),
        std::move(data->moveIds),
        std::move(data->battleScript),
        data->experienceReward,
        std::move(data->itemDropIds),
        data->active);
}
