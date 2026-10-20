#include "../src/Game/Battle/BattleMoveLoad.h"
#include "../src/Game/Battle/BattleRules.h"
#include "../src/Game/Items/ItemDefinitionLoad.h"
#include "../src/Game/State/PlayerState.h"
#include "../src/Game/Objects/BattleObjectLoad.h"
#include "../src/Game/Battle/BattleStats.h"

#include <yaml-cpp/yaml.h>

#include <iostream>

namespace
{
bool Expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << message << '\n';
        return false;
    }
    return true;
}
}

int main()
{
    BattleMove strike;
    if (!Expect(LoadBattleMove(
            "strike",
            "assets/battle/moves/strike.move.yaml",
            strike),
            "The shared strike move did not load")
        || !Expect(strike.id == "strike"
                && strike.effect == BattleMoveEffect::Damage
                && strike.target == BattleMoveTarget::Opponent
                && strike.impactFrame == 3
                && strike.impactSoundId == "strikeImpact",
            "The shared strike move loaded incorrect mechanics"))
    {
        return 1;
    }

    BattleMove swordStrike;
    BattleMove clubStrike;
    if (!Expect(LoadBattleMove(
            "swordStrike",
            "assets/battle/moves/sword_strike.move.yaml",
            swordStrike),
            "The sword strike move did not load")
        || !Expect(swordStrike.id == "swordStrike"
                && swordStrike.impactSoundId == "swordStrikeImpact",
            "The sword strike loaded incorrect sound data")
        || !Expect(LoadBattleMove(
            "clubStrike",
            "assets/battle/moves/club_strike.move.yaml",
            clubStrike),
            "The club strike move did not load")
        || !Expect(clubStrike.id == "clubStrike"
                && clubStrike.impactSoundId == "clubStrikeImpact",
            "The club strike loaded incorrect sound data"))
    {
        return 1;
    }

    const YAML::Node playerRoot = YAML::LoadFile(
        "assets/battle/characters/player/player_battle.obj.yaml");
    YAML::Node playerDefinition = YAML::Clone(playerRoot["init"]);
    // SceneBuilder normally adds the resource ID while resolving the object
    // template before dispatching to this type-specific loader.
    playerDefinition["id"] = "battlePlayerObj";
    const std::optional<BattleObjectLoadData> player =
        ReadBattleObjectLoadData(playerDefinition, false);
    if (!Expect(player.has_value(),
            "The battle-player definition did not load")
        || !Expect(player->moveIds.size() == 1
                && player->moveIds.front() == "strike",
            "The battle-player definition loaded incorrect move data"))
    {
        return 1;
    }

    const YAML::Node enemyRoot = YAML::LoadFile(
        "assets/battle/characters/goblin_sword/goblin_sword.obj.yaml");
    YAML::Node enemyDefinition = YAML::Clone(enemyRoot["init"]);
    enemyDefinition["id"] = "goblinSword";
    const std::optional<BattleObjectLoadData> enemy =
        ReadBattleObjectLoadData(enemyDefinition);
    if (!Expect(enemy.has_value(),
            "The battle-enemy definition did not load")
        || !Expect(
            enemy->battleScript
                == "assets/battle/characters/goblin_sword/"
                   "goblin_sword.battle.lua",
            "The battle enemy loaded the wrong tactics script")
        || !Expect(enemy->moveIds.size() == 1
                && enemy->moveIds.front() == "swordStrike",
            "The sword goblin loaded the wrong move")
        || !Expect(enemy->experienceReward == 12
                && enemy->itemDropIds.size() == 1
                && enemy->itemDropIds.front() == "rustyDagger",
            "The battle enemy loaded incorrect reward data"))
    {
        return 1;
    }

    ItemDefinition dagger;
    if (!Expect(LoadItemDefinition(
            "rustyDagger",
            "assets/items/rusty_dagger.item.yaml",
            dagger),
            "The item definition did not load")
        || !Expect(dagger.name == "Rusty Dagger"
                && dagger.value == 6,
            "The item definition loaded incorrect data"))
    {
        return 1;
    }

    ItemDefinition lantern;
    if (!Expect(LoadItemDefinition(
            "guiding_lantern",
            "assets/items/guiding_lantern.item.yaml",
            lantern),
            "The usable item definition did not load")
        || !Expect(
            lantern.useScript == "assets/items/guiding_lantern.lua",
            "The usable item loaded the wrong Lua script"))
    {
        return 1;
    }

    PlayerState playerState;
    if (!Expect(playerState.Initialize(
            "Cecil", 1, 0, 142, 36, 12, 5, 0),
            "Player state rejected valid starting values")
        || !Expect(playerState.AddExperience(27),
            "Player state rejected a valid reward")
        || !Expect(playerState.GetExperience() == 27
                && playerState.GetExperienceUntilNextLevel(100) == 73,
            "Player state stored incorrect XP values"))
    {
        return 1;
    }

    const BattleStats& playerBattleStats = playerState.GetBattleStats();
    playerState.SetHp(100);
    playerState.SetMp(20);
    if (!Expect(&playerBattleStats == &playerState.GetBattleStats(),
            "Player state did not retain one canonical battle-stats object")
        || !Expect(playerBattleStats.name == "Cecil"
                && playerBattleStats.level == 1
                && playerBattleStats.hp == 100
                && playerBattleStats.mp == 20
                && playerBattleStats.attack == 12
                && playerBattleStats.defense == 5,
            "Player battle stats did not reflect persistent state changes"))
    {
        return 1;
    }

    if (!Expect(ScaleBattleStat(10, 1) == 10,
            "Level 1 did not preserve the base stat")
        || !Expect(ScaleBattleStat(10, 2) == 11,
            "Level 2 did not add ten percent")
        || !Expect(ScaleBattleStat(10, 5) == 14,
            "Level 5 did not use linear level growth")
        || !Expect(ScaleBattleStat(0, 20) == 0,
            "A zero base stat did not remain zero"))
    {
        return 1;
    }

    const BattleStats attacker{"Attacker", 1, 20, 20, 0, 0, 10, 2};
    const BattleStats target{"Target", 1, 20, 20, 0, 0, 1, 3};
    return Expect(CalculateBattleDamage(attacker, target, strike.power) == 7,
            "Physical damage did not apply move power and defense")
        && Expect(CalculateBattleDamage(attacker, target, 0.1F) == 1,
            "Physical damage did not preserve the one-damage minimum")
        ? 0
        : 1;
}
