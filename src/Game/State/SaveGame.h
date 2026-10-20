#pragma once

#include "GlobalState.h"
#include "Inventory.h"
#include "PlayerState.h"

#include <filesystem>
#include <string>

struct SaveGameData
{
    std::string savedAt;
    std::string sceneId;
    float playerX = 0.0F;
    float playerY = 0.0F;
    GlobalState globalState;
    Inventory inventory;
    PlayerState playerState;
};

bool WriteSaveGame(
    const std::filesystem::path& path,
    std::string_view sceneId,
    float playerX,
    float playerY,
    const GlobalState& globalState,
    const Inventory& inventory,
    const PlayerState& playerState);

bool ReadSaveGame(
    const std::filesystem::path& path,
    SaveGameData& saveGame);
