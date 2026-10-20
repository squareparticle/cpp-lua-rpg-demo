#include "SaveGame.h"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

namespace
{
constexpr int CurrentSaveVersion = 1;

std::string GetUtcTimestamp()
{
    const std::time_t now = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());
    std::tm utc{};
    if (gmtime_s(&utc, &now) != 0)
    {
        return {};
    }

    std::ostringstream timestamp;
    timestamp << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return timestamp.str();
}

bool IsPlayerNodeValid(const YAML::Node& player)
{
    return player.IsMap()
        && player["name"].IsScalar()
        && player["level"].IsScalar()
        && player["experience"].IsScalar()
        && player["hp"].IsScalar()
        && player["maxHp"].IsScalar()
        && player["mp"].IsScalar()
        && player["maxMp"].IsScalar()
        && player["attack"].IsScalar()
        && player["defense"].IsScalar()
        && player["gold"].IsScalar();
}
}

bool WriteSaveGame(
    const std::filesystem::path& path,
    std::string_view sceneId,
    float playerX,
    float playerY,
    const GlobalState& globalState,
    const Inventory& inventory,
    const PlayerState& playerState)
{
    if (path.empty()
        || sceneId.empty()
        || !std::isfinite(playerX)
        || !std::isfinite(playerY)
        || !playerState.IsInitialized())
    {
        return false;
    }

    const std::string savedAt = GetUtcTimestamp();
    if (savedAt.empty())
    {
        return false;
    }

    std::error_code error;
    if (!path.parent_path().empty())
    {
        std::filesystem::create_directories(path.parent_path(), error);
        if (error)
        {
            return false;
        }
    }

    YAML::Emitter output;
    output << YAML::BeginMap
        << YAML::Key << "version" << YAML::Value << CurrentSaveVersion
        << YAML::Key << "savedAt" << YAML::Value << savedAt
        << YAML::Key << "scene" << YAML::Value << YAML::BeginMap
        << YAML::Key << "id" << YAML::Value << std::string(sceneId)
        << YAML::Key << "playerX" << YAML::Value << playerX
        << YAML::Key << "playerY" << YAML::Value << playerY
        << YAML::EndMap
        << YAML::Key << "player" << YAML::Value << YAML::BeginMap
        << YAML::Key << "name" << YAML::Value << playerState.GetName()
        << YAML::Key << "level" << YAML::Value << playerState.GetLevel()
        << YAML::Key << "experience" << YAML::Value
        << playerState.GetExperience()
        << YAML::Key << "hp" << YAML::Value << playerState.GetHp()
        << YAML::Key << "maxHp" << YAML::Value << playerState.GetMaxHp()
        << YAML::Key << "mp" << YAML::Value << playerState.GetMp()
        << YAML::Key << "maxMp" << YAML::Value << playerState.GetMaxMp()
        << YAML::Key << "attack" << YAML::Value << playerState.GetAttack()
        << YAML::Key << "defense" << YAML::Value << playerState.GetDefense()
        << YAML::Key << "gold" << YAML::Value << playerState.GetGold()
        << YAML::EndMap
        << YAML::Key << "inventory" << YAML::Value << YAML::BeginSeq;
    for (const std::string& itemId : inventory.GetItemIds())
    {
        output << itemId;
    }
    output << YAML::EndSeq
        << YAML::Key << "tags" << YAML::Value << YAML::BeginMap;

    std::vector<std::pair<std::string, bool>> tags(
        globalState.GetTags().begin(),
        globalState.GetTags().end());
    std::ranges::sort(tags, {}, &std::pair<std::string, bool>::first);
    for (const auto& [name, value] : tags)
    {
        output << YAML::Key << name << YAML::Value << value;
    }
    output << YAML::EndMap << YAML::EndMap;
    if (!output.good())
    {
        return false;
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file << output.c_str();
    return file.good();
}

bool ReadSaveGame(
    const std::filesystem::path& path,
    SaveGameData& saveGame)
{
    try
    {
        const YAML::Node root = YAML::LoadFile(path.string());
        const YAML::Node scene = root["scene"];
        const YAML::Node player = root["player"];
        const YAML::Node inventory = root["inventory"];
        const YAML::Node tags = root["tags"];
        if (!root.IsMap()
            || !root["version"].IsScalar()
            || root["version"].as<int>() != CurrentSaveVersion
            || !root["savedAt"].IsScalar()
            || !scene.IsMap()
            || !scene["id"].IsScalar()
            || !scene["playerX"].IsScalar()
            || !scene["playerY"].IsScalar()
            || !IsPlayerNodeValid(player)
            || !inventory.IsSequence()
            || !tags.IsMap())
        {
            return false;
        }

        SaveGameData loaded;
        loaded.savedAt = root["savedAt"].as<std::string>();
        loaded.sceneId = scene["id"].as<std::string>();
        loaded.playerX = scene["playerX"].as<float>();
        loaded.playerY = scene["playerY"].as<float>();
        if (loaded.savedAt.empty()
            || loaded.sceneId.empty()
            || !std::isfinite(loaded.playerX)
            || !std::isfinite(loaded.playerY))
        {
            return false;
        }

        const int hp = player["hp"].as<int>();
        const int maxHp = player["maxHp"].as<int>();
        const int mp = player["mp"].as<int>();
        const int maxMp = player["maxMp"].as<int>();
        if (hp < 0 || hp > maxHp || mp < 0 || mp > maxMp
            || !loaded.playerState.Initialize(
                player["name"].as<std::string>(),
                player["level"].as<int>(),
                player["experience"].as<int>(),
                maxHp,
                maxMp,
                player["attack"].as<int>(),
                player["defense"].as<int>(),
                player["gold"].as<int>()))
        {
            return false;
        }
        loaded.playerState.SetHp(hp);
        loaded.playerState.SetMp(mp);

        for (const YAML::Node& item : inventory)
        {
            if (!item.IsScalar()
                || !loaded.inventory.AddItem(item.as<std::string>()))
            {
                return false;
            }
        }
        for (const auto& tag : tags)
        {
            if (!tag.first.IsScalar() || !tag.second.IsScalar())
            {
                return false;
            }
            const std::string name = tag.first.as<std::string>();
            if (name.empty() || loaded.globalState.HasTag(name))
            {
                return false;
            }
            loaded.globalState.SetTag(name, tag.second.as<bool>());
        }

        saveGame = std::move(loaded);
        return true;
    }
    catch (const YAML::Exception&)
    {
        return false;
    }
}
