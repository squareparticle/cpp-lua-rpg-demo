#pragma once

#include <cstddef>
#include <string>

enum class BattleMoveEffect
{
    Damage,
    Heal
};

enum class BattleMoveTarget
{
    Opponent,
    Ally
};

struct BattleMove
{
    std::string id;
    BattleMoveEffect effect = BattleMoveEffect::Damage;
    BattleMoveTarget target = BattleMoveTarget::Opponent;
    float power = 1.0F;
    int mpCost = 0;
    std::string actorAnimation;
    std::string targetAnimation;
    std::size_t impactFrame = 0;
    std::string impactSoundId;
};
