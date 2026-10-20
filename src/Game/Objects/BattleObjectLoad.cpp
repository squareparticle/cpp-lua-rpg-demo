#include "BattleObjectLoad.h"

#include "../../Platform/DebugLog.h"
#include "../Battle/BattleRules.h"

#include <yaml-cpp/yaml.h>

std::optional<BattleObjectLoadData> ReadBattleObjectLoadData(
    const YAML::Node& node,
    bool requiresStats)
{
    const YAML::Node animation = node["animation"];
    const YAML::Node x = node["x"];
    const YAML::Node y = node["y"];
    const YAML::Node scale = node["scale"];
    const YAML::Node active = node["active"];
    const YAML::Node level = node["level"];
    const YAML::Node moves = node["moves"];
    const YAML::Node battleScript = node["battleScript"];
    const YAML::Node rewards = node["rewards"];
    const YAML::Node stats = node["stats"];
    if (!animation.IsScalar()
        || !x.IsScalar()
        || !y.IsScalar()
        || !moves.IsSequence()
        || moves.size() == 0
        || (requiresStats
            && (!level.IsScalar()
                || !stats.IsMap()
                || !stats["name"].IsScalar()
                || !stats["maxHp"].IsScalar()
                || !stats["maxMp"].IsScalar()
                || !stats["attack"].IsScalar()
                || !stats["defense"].IsScalar()))
        || (scale && !scale.IsScalar())
        || (battleScript && !battleScript.IsScalar())
        || (rewards && !rewards.IsMap())
        || (active && !active.IsScalar()))
    {
        WriteDebugMessage(
            "Battle object is missing its placement or stats.");
        return std::nullopt;
    }

    BattleObjectLoadData result;
    result.id = node["id"].as<std::string>();
    result.spriteId = node["sprite"].as<std::string>();
    result.animationName = animation.as<std::string>();
    result.x = x.as<float>();
    result.y = y.as<float>();
    result.scale = scale ? scale.as<float>() : 1.0F;
    if (requiresStats)
    {
        const int battleLevel = level.as<int>();
        const int baseMaxHp = stats["maxHp"].as<int>();
        const int baseMaxMp = stats["maxMp"].as<int>();
        result.stats = BattleStats{
            stats["name"].as<std::string>(),
            battleLevel,
            ScaleBattleStat(baseMaxHp, battleLevel),
            ScaleBattleStat(baseMaxHp, battleLevel),
            ScaleBattleStat(baseMaxMp, battleLevel),
            ScaleBattleStat(baseMaxMp, battleLevel),
            ScaleBattleStat(stats["attack"].as<int>(), battleLevel),
            ScaleBattleStat(stats["defense"].as<int>(), battleLevel)};
        if (battleLevel < 1
            || result.stats.name.empty()
            || result.stats.maxHp <= 0
            || result.stats.maxMp < 0
            || result.stats.attack <= 0
            || result.stats.defense < 0)
        {
            WriteDebugMessage("Battle object has invalid stats.");
            return std::nullopt;
        }
    }
    for (const YAML::Node& move : moves)
    {
        if (!move.IsScalar())
        {
            return std::nullopt;
        }
        const std::string moveId = move.as<std::string>();
        if (moveId.empty())
        {
            return std::nullopt;
        }
        result.moveIds.push_back(moveId);
    }
    result.battleScript = battleScript
        ? battleScript.as<std::string>()
        : std::string{};
    if (rewards)
    {
        const YAML::Node rewardExperience = rewards["experience"];
        const YAML::Node items = rewards["items"];
        if (!rewardExperience.IsScalar()
            || !items.IsSequence()
            || rewardExperience.as<int>() < 0)
        {
            return std::nullopt;
        }
        result.experienceReward = rewardExperience.as<int>();
        for (const YAML::Node& item : items)
        {
            if (!item.IsScalar())
            {
                return std::nullopt;
            }
            const std::string itemId = item.as<std::string>();
            if (itemId.empty())
            {
                return std::nullopt;
            }
            result.itemDropIds.push_back(itemId);
        }
    }
    result.active = active ? active.as<bool>() : true;

    if (result.scale <= 0.0F)
    {
        WriteDebugMessage("Battle object has invalid stats.");
        return std::nullopt;
    }

    return result;
}
