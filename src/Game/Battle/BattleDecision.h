#pragma once

#include <string>
#include <vector>

// Lua receives copies of these values. Changing the Lua table cannot mutate
// live combatants; the returned intent still has to pass the arena's C++ rules.
struct BattleActorSnapshot
{
    std::string id;
    std::string name;
    int level = 1;
    int hp = 0;
    int maxHp = 0;
    int mp = 0;
    int maxMp = 0;
    int attack = 0;
    int defense = 0;
    std::vector<std::string> moveIds;
};

struct BattleDecisionContext
{
    BattleActorSnapshot actor;
    BattleActorSnapshot player;
    std::vector<BattleActorSnapshot> enemies;
};

struct BattleActionIntent
{
    std::string moveId;
    std::string targetId;
};
