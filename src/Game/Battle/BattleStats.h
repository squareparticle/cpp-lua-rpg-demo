#pragma once

#include <string>

// The combat-facing portion of an actor's state. PlayerState owns the
// player's instance alongside persistent progression data; BattleEnemy owns
// its own instance because enemies do not have persistent player progression.
struct BattleStats
{
    std::string name;
    int level = 1;
    int hp = 0;
    int maxHp = 0;
    int mp = 0;
    int maxMp = 0;
    int attack = 0;
    int defense = 0;
};
