#pragma once

struct BattleStats;

inline constexpr float BattleStatGrowthPerLevel = 0.1F;

int ScaleBattleStat(int baseValue, int level) noexcept;
int CalculateBattleDamage(
    const BattleStats& attacker,
    const BattleStats& target,
    float movePower) noexcept;
