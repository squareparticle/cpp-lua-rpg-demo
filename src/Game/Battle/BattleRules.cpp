#include "BattleRules.h"

#include "BattleStats.h"

#include <algorithm>
#include <cmath>

int ScaleBattleStat(int baseValue, int level) noexcept
{
    if (baseValue <= 0)
    {
        return 0;
    }

    const int safeLevel = std::max(1, level);
    const float multiplier = 1.0F
        + BattleStatGrowthPerLevel * static_cast<float>(safeLevel - 1);
    return std::max(
        1,
        static_cast<int>(std::lround(
            static_cast<float>(baseValue) * multiplier)));
}

int CalculateBattleDamage(
    const BattleStats& attacker,
    const BattleStats& target,
    float movePower) noexcept
{
    const int rawDamage = static_cast<int>(std::lround(
        static_cast<float>(attacker.attack) * movePower));
    return std::max(1, rawDamage - target.defense);
}
