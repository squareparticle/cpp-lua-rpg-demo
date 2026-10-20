#include "BattleMoveLoad.h"

#include <yaml-cpp/yaml.h>

#include <cmath>

bool LoadBattleMove(
    const std::string& id,
    const std::string& filename,
    BattleMove& move)
{
    try
    {
        const YAML::Node root = YAML::LoadFile(filename);
        const YAML::Node effect = root["effect"];
        const YAML::Node target = root["target"];
        const YAML::Node animations = root["animations"];
        const YAML::Node sounds = root["sounds"];
        if (id.empty()
            || !effect.IsScalar()
            || !target.IsScalar()
            || !root["power"].IsScalar()
            || !root["mpCost"].IsScalar()
            || !animations.IsMap()
            || !animations["actor"].IsScalar()
            || !animations["target"].IsScalar()
            || !animations["impactFrame"].IsScalar()
            || (sounds
                && (!sounds.IsMap()
                    || (sounds["impact"]
                        && !sounds["impact"].IsScalar()))))
        {
            return false;
        }

        BattleMove parsed;
        parsed.id = id;
        const std::string effectName = effect.as<std::string>();
        if (effectName == "damage")
        {
            parsed.effect = BattleMoveEffect::Damage;
        }
        else if (effectName == "heal")
        {
            parsed.effect = BattleMoveEffect::Heal;
        }
        else
        {
            return false;
        }

        const std::string targetName = target.as<std::string>();
        if (targetName == "opponent")
        {
            parsed.target = BattleMoveTarget::Opponent;
        }
        else if (targetName == "ally")
        {
            parsed.target = BattleMoveTarget::Ally;
        }
        else
        {
            return false;
        }

        parsed.power = root["power"].as<float>();
        parsed.mpCost = root["mpCost"].as<int>();
        parsed.actorAnimation = animations["actor"].as<std::string>();
        parsed.targetAnimation = animations["target"].as<std::string>();
        parsed.impactFrame = animations["impactFrame"].as<std::size_t>();
        if (sounds && sounds["impact"])
        {
            parsed.impactSoundId = sounds["impact"].as<std::string>();
        }
        if (!std::isfinite(parsed.power)
            || parsed.power <= 0.0F
            || parsed.mpCost < 0
            || parsed.actorAnimation.empty()
            || parsed.targetAnimation.empty()
            || (sounds && sounds["impact"]
                && parsed.impactSoundId.empty()))
        {
            return false;
        }

        move = std::move(parsed);
        return true;
    }
    catch (const YAML::Exception&)
    {
        return false;
    }
}
