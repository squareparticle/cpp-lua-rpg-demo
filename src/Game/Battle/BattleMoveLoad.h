#pragma once

#include "BattleMove.h"

#include <string>

bool LoadBattleMove(
    const std::string& id,
    const std::string& filename,
    BattleMove& move);
