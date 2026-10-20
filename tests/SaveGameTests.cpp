#include "../src/Game/State/SaveGame.h"

#include <filesystem>
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
    const std::filesystem::path path = "SaveGameTests.save.yaml";
    std::error_code error;
    std::filesystem::remove(path, error);

    GlobalState globalState;
    globalState.SetTag("mission_started", true);
    globalState.SetTag("door_open", false);
    globalState.SetTag("weather=fairies", true);
    Inventory inventory;
    inventory.AddItem("storageKey");
    inventory.AddItem("rustyDagger");
    PlayerState player;
    if (!player.Initialize("Cecil", 2, 35, 150, 40, 14, 7, 19))
    {
        return 1;
    }
    player.SetHp(91);
    player.SetMp(13);

    if (!Expect(WriteSaveGame(
            path,
            "storageRoom",
            412.5F,
            633.25F,
            globalState,
            inventory,
            player),
            "Could not write the test save"))
    {
        return 1;
    }

    SaveGameData loaded;
    const bool read = ReadSaveGame(path, loaded);
    std::filesystem::remove(path, error);
    return Expect(read, "Could not read the test save")
        && Expect(!loaded.savedAt.empty(), "Save timestamp was empty")
        && Expect(loaded.sceneId == "storageRoom"
                && loaded.playerX == 412.5F
                && loaded.playerY == 633.25F,
            "World save location changed during round trip")
        && Expect(loaded.globalState.GetTag("mission_started")
                && loaded.globalState.GetTag("weather=fairies")
                && loaded.globalState.HasTag("door_open")
                && !loaded.globalState.GetTag("door_open"),
            "Global tags changed during round trip")
        && Expect(loaded.inventory.HasItem("storageKey")
                && loaded.inventory.HasItem("rustyDagger"),
            "Inventory changed during round trip")
        && Expect(loaded.playerState.GetName() == "Cecil"
                && loaded.playerState.GetLevel() == 2
                && loaded.playerState.GetExperience() == 35
                && loaded.playerState.GetHp() == 91
                && loaded.playerState.GetMaxHp() == 150
                && loaded.playerState.GetMp() == 13
                && loaded.playerState.GetMaxMp() == 40
                && loaded.playerState.GetAttack() == 14
                && loaded.playerState.GetDefense() == 7
                && loaded.playerState.GetGold() == 19,
            "Player state changed during round trip")
        ? 0
        : 1;
}
