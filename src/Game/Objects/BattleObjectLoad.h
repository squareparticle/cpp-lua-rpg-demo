#pragma once

#include "../Battle/BattleStats.h"

#include <optional>
#include <string>
#include <vector>

namespace YAML
{
class Node;
}

struct BattleObjectLoadData
{
    std::string id;
    std::string spriteId;
    std::string animationName;
    float x = 0.0F;
    float y = 0.0F;
    float scale = 1.0F;
    BattleStats stats;
    std::vector<std::string> moveIds;
    std::string battleScript;
    int experienceReward = 0;
    std::vector<std::string> itemDropIds;
    bool active = true;
};

std::optional<BattleObjectLoadData> ReadBattleObjectLoadData(
    const YAML::Node& node,
    bool requiresStats = true);
