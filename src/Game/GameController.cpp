#include "GameController.h"

#include "Input/InputState.h"
#include "../Framework/Scenes/BaseScene.h"
#include "../Platform/Graphics.h"
#include "../Platform/DebugLog.h"
#include "Scenes/SceneBuilder.h"
#include "Items/ItemDefinitionLoad.h"
#include "State/SaveGame.h"
#include "Scenes/BattleArenaScene.h"
#include "Scenes/BoatScene.h"
#include "Scenes/StorageRoomScene.h"
#include "Scenes/EndingScene.h"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <memory>
#include <cmath>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace
{
constexpr char MainMenuPointerId[] = "mainMenuPointer";
constexpr char MainMenuPointerPath[] = "assets/props/pointer.png";
constexpr char WeatherRainId[] = "weatherRain";
constexpr char WeatherRainPath[] = "assets/world/weather/rain.png";
constexpr char WeatherFairiesId[] = "weatherFairies";
constexpr char WeatherFairiesPath[] = "assets/world/weather/fairies.png";
constexpr char GameDataPath[] = "game.yaml";
constexpr float MusicFadeOutSeconds = 0.2F;
constexpr float MusicFadeInSeconds = 0.2F;
const std::filesystem::path SaveGamePath = "saves/save.yaml";
}

GameController::GameController(float dialogCharactersPerSecond)
    : dialogueWindow_(dialogCharactersPerSecond),
      luaSystem_(globalState_, inventory_, dialogueWindow_),
      mainMenu_(
          inventory_,
          playerState_,
          experienceToNextLevelByLevel_,
          itemDefinitions_,
          MainMenuPointerId)
{
}

GameController::~GameController()
{
    if (currentScene_ != nullptr)
    {
        // Shutdown runs object DESTROY hooks before member destruction reaches
        // the shared Lua runtime those objects use.
        currentScene_->Shutdown();
    }
}

bool GameController::InitGame(
    ID2D1RenderTarget* renderTarget,
    std::string_view startSceneId,
    bool loadSaveWhenPresent)
{
    if (renderTarget == nullptr)
    {
        return false;
    }

    if (!audio_.Initialize())
    {
        WriteDebugMessage("Could not initialize XAudio2.");
        return false;
    }

    if (!LoadGameData())
    {
        WriteDebugMessage("Could not load game.yaml.");
        return false;
    }

    if (!luaSystem_.Initialize())
    {
        return false;
    }

    graphics_ = std::make_unique<Graphics>(renderTarget);
    if (!graphics_->Initialize())
    {
        return false;
    }
    if (!resources_.LoadImage(
            renderTarget,
            MainMenuPointerId,
            MainMenuPointerPath,
            ResourceScope::Game)
        || !resources_.LoadImage(
            renderTarget,
            WeatherRainId,
            WeatherRainPath,
            ResourceScope::Game)
        || !resources_.LoadImage(
            renderTarget,
            WeatherFairiesId,
            WeatherFairiesPath,
            ResourceScope::Game))
    {
        return false;
    }
    if (loadSaveWhenPresent)
    {
        std::error_code error;
        const bool saveExists = std::filesystem::exists(SaveGamePath, error);
        if (error)
        {
            WriteDebugMessage("Could not check for saves/save.yaml.");
            return false;
        }
        if (saveExists)
        {
            // An existing but invalid save is an error rather than silently
            // starting over and making the player think their save vanished.
            if (!LoadGame())
            {
                WriteDebugMessage("Could not load startup save.");
                return false;
            }
            return true;
        }
    }

    // Startup and in-game transitions share the same scene-ID routing. The
    // fallback request has no placement override, so the scene uses its
    // authored starting player position.
    return ChangeScene(SceneChangeRequest{{}, std::string(startSceneId), {}});
}

bool GameController::LoadScene(std::unique_ptr<BaseScene> newScene)
{
    if (newScene == nullptr || newScene->GetMapPath().empty())
    {
        return false;
    }

    UnloadCurrentScene();

    if (!SceneBuilder::BuildScene(
            newScene->GetMapPath(),
            graphics_->GetRenderTarget(),
            resources_,
            *newScene))
    {
        resources_.ClearScene();
        return false;
    }

    if (!newScene->Init())
    {
        newScene->Shutdown();
        luaSystem_.ClearSceneActions();
        resources_.ClearScene();
        return false;
    }

    currentScene_ = std::move(newScene);
    const std::string& musicId = currentScene_->GetMusicId();
    if (!musicId.empty() && !RequestMusic(musicId))
    {
        WriteDebugMessage("Could not request scene music: " + musicId);
    }
    return true;
}

bool GameController::SetRenderTarget(ID2D1RenderTarget* renderTarget)
{
    if (graphics_ == nullptr)
    {
        return false;
    }

    if (renderTarget == nullptr)
    {
        resources_.ReleaseAssets();
        graphics_->SetRenderTarget(nullptr);
        return true;
    }

    graphics_->SetRenderTarget(renderTarget);
    return resources_.ReloadAssets(renderTarget);
}

void GameController::Update(float deltaTime, const InputState& input)
{
    audio_.Update();
    UpdateMusic(deltaTime);
    ProcessAudioRequests();
    ProcessDialogueFeedback();

    if (mainMenu_.IsOpen())
    {
        // The pause boundary is here: the GUI still advances from real frame
        // time, but neither the scene nor its Lua-owned gameplay can update.
        mainMenu_.Update(deltaTime, input);
        ProcessMainMenuFeedback();
        if (mainMenu_.ConsumeNavigationFeedback())
        {
            PlaySoundAsset("menuMove");
        }
        if (mainMenu_.ConsumeSelectionFeedback())
        {
            PlaySoundAsset("menuSelect");
        }
        if (mainMenu_.ConsumeSaveRequest())
        {
            if (SaveGame())
            {
                mainMenu_.SetLoadEnabled(true);
                // Closing the menu is the visible confirmation that the file
                // was written. A failed save deliberately leaves it open.
                mainMenu_.Close();
                ProcessMainMenuFeedback();
                WriteDebugMessage("Game saved to saves/save.yaml.");
            }
            else
            {
                WriteDebugMessage("Could not save the game.");
            }
        }
        if (mainMenu_.ConsumeLoadRequest() && !LoadGame())
        {
            WriteDebugMessage("Could not load saves/save.yaml.");
        }
        if (mainMenu_.ConsumeQuitRequest())
        {
            // Application owns the Win32 window, so the game requests a quit
            // instead of crossing that platform boundary.
            quitRequested_ = true;
        }
        if (std::optional<std::string> itemId =
                mainMenu_.ConsumeItemUseRequest())
        {
            // Item scripts may open modal dialogue immediately. Close the
            // paused menu first so it cannot cover or intercept that dialogue.
            mainMenu_.Close();
            if (!UseInventoryItem(*itemId))
            {
                WriteDebugMessage("Could not use inventory item: " + *itemId);
            }
            // Resolve close feedback after the script has had a chance to
            // open dialogue. This lets the generic feedback coordinator treat
            // a direct menu-to-dialogue handoff as one transition.
            ProcessMainMenuFeedback();
        }
        ProcessDialogueFeedback();
        ProcessAudioRequests();
        return;
    }

    if (currentScene_ != nullptr)
    {
        if (dialogueWindow_.IsModalActive())
        {
            // The world keeps animating, but an empty game-level input state
            // prevents menu navigation from also moving or interacting.
            currentScene_->Update(deltaTime, InputState{});
            dialogueWindow_.Update(deltaTime, input);
        }
        else if (!luaSystem_.IsPlayerInputEnabled())
        {
            // Cutscenes keep advancing while world controls and menu-opening
            // input are disabled. Dialogue above still accepts Continue.
            currentScene_->Update(deltaTime, InputState{});
        }
        else if (input.inventoryPressed)
        {
            std::error_code error;
            mainMenu_.SetSaveEnabled(
                dynamic_cast<WorldScene*>(currentScene_.get()) != nullptr);
            mainMenu_.SetLoadEnabled(
                std::filesystem::exists(SaveGamePath, error) && !error);
            if (!mainMenu_.OpenInventory())
            {
                WriteDebugMessage("Could not open the inventory menu.");
            }
            ProcessMainMenuFeedback();
            return;
        }
        else if (input.menuPressed)
        {
            // Dialogue remains the active modal when it is open. Otherwise
            // Escape pauses immediately; the world does not receive this
            // frame before the menu appears.
            std::error_code error;
            mainMenu_.SetSaveEnabled(
                dynamic_cast<WorldScene*>(currentScene_.get()) != nullptr);
            mainMenu_.SetLoadEnabled(
                std::filesystem::exists(SaveGamePath, error) && !error);
            mainMenu_.Open();
            ProcessMainMenuFeedback();
            return;
        }
        else
        {
            currentScene_->Update(deltaTime, input);
        }

        ProcessAudioRequests();

        ProcessDialogueFeedback();

        std::optional<SceneChangeRequest> sceneChange =
            currentScene_->TakeSceneChangeRequest();
        if (sceneChange && !ChangeScene(*sceneChange))
        {
            WriteDebugMessage(
                "Could not change scene requested by: "
                + sceneChange->source);
        }
    }

    if (dialogueWindow_.ConsumeAdvanceRequest())
    {
        luaSystem_.CompleteDialog();
        if (!dialogueWindow_.IsModalActive())
        {
            const std::optional<std::string> exitSoundOverride =
                luaSystem_.TakeDialogExitSoundOverride();
            PlaySoundAsset(exitSoundOverride.value_or("uiExit"));
        }
        ProcessDialogueFeedback();
        ProcessAudioRequests();
    }

    BattleRequest battleRequest;
    if (luaSystem_.PopBattleRequest(battleRequest))
    {
        auto* worldScene = dynamic_cast<WorldScene*>(currentScene_.get());
        if (worldScene == nullptr
            || !worldScene->AddReturnContext(battleRequest)
            || !LoadScene(std::make_unique<BattleArenaScene>(
                resources_,
                audio_,
                luaSystem_,
                inventory_,
                playerState_,
                experienceToNextLevelByLevel_,
                itemDefinitions_,
                std::move(battleRequest))))
        {
            WriteDebugMessage("Could not start the requested battle.");
        }
    }

    ProcessAudioRequests();
}

bool GameController::ConsumeQuitRequest() noexcept
{
    // This is a one-shot message, not an attempt to cancel quitting. Preserve
    // the answer for the caller, then clear the stored flag so Application
    // handles this particular menu selection exactly once.
    const bool requested = quitRequested_;
    quitRequested_ = false;
    return requested;
}

bool GameController::LoadGameData()
{
    try
    {
        const YAML::Node root = YAML::LoadFile(GameDataPath);
        const YAML::Node player = root["player"];
        const YAML::Node experienceTable =
            player["experienceToNextLevel"];
        const YAML::Node items = root["resources"]["items"];
        const YAML::Node sounds = root["resources"]["sounds"];
        if (!root.IsMap()
            || !player.IsMap()
            || !player["name"].IsScalar()
            || !player["level"].IsScalar()
            || !player["experience"].IsScalar()
            || !player["maxHp"].IsScalar()
            || !player["maxMp"].IsScalar()
            || !player["attack"].IsScalar()
            || !player["defense"].IsScalar()
            || !player["gold"].IsScalar()
            || !experienceTable.IsMap()
            || !items.IsSequence()
            || (sounds && !sounds.IsSequence()))
        {
            return false;
        }

        if (sounds)
        {
            for (const YAML::Node& soundNode : sounds)
            {
                if (!soundNode.IsMap()
                    || !soundNode["id"].IsScalar()
                    || !soundNode["path"].IsScalar()
                    || !resources_.LoadSound(
                        soundNode["id"].as<std::string>(),
                        soundNode["path"].as<std::string>(),
                        ResourceScope::Game))
                {
                    return false;
                }
            }
        }

        std::unordered_map<int, int> loadedExperienceTable;
        for (const auto& entry : experienceTable)
        {
            const int level = entry.first.as<int>();
            const int experience = entry.second.as<int>();
            if (level < 1
                || experience <= 0
                || !loadedExperienceTable.emplace(level, experience).second)
            {
                return false;
            }
        }

        std::unordered_map<std::string, ItemDefinition> loadedItems;
        for (const YAML::Node& itemNode : items)
        {
            if (!itemNode.IsMap()
                || !itemNode["id"].IsScalar()
                || !itemNode["path"].IsScalar())
            {
                return false;
            }

            const std::string id = itemNode["id"].as<std::string>();
            ItemDefinition item;
            if (!LoadItemDefinition(
                    id,
                    itemNode["path"].as<std::string>(),
                    item)
                || !loadedItems.emplace(id, std::move(item)).second)
            {
                return false;
            }
        }

        const int initialLevel = player["level"].as<int>();
        const int initialExperience = player["experience"].as<int>();
        const auto nextLevel = loadedExperienceTable.find(initialLevel);
        if (nextLevel == loadedExperienceTable.end()
            || initialExperience < 0
            || initialExperience >= nextLevel->second
            || !playerState_.Initialize(
                player["name"].as<std::string>(),
                initialLevel,
                initialExperience,
                player["maxHp"].as<int>(),
                player["maxMp"].as<int>(),
                player["attack"].as<int>(),
                player["defense"].as<int>(),
                player["gold"].as<int>()))
        {
            return false;
        }

        experienceToNextLevelByLevel_ =
            std::move(loadedExperienceTable);
        itemDefinitions_ = std::move(loadedItems);
        return true;
    }
    catch (const YAML::Exception&)
    {
        return false;
    }
}

bool GameController::ProcessAudioRequests()
{
    bool allSucceeded = true;
    AudioRequest request;
    while (luaSystem_.PopAudioRequest(request))
    {
        if (request.type == AudioRequestType::StopMusic)
        {
            RequestMusicStop();
            continue;
        }

        const SoundAsset* sound = resources_.GetSound(request.assetId);
        const bool succeeded = sound != nullptr
            && (request.type == AudioRequestType::PlaySound
                ? audio_.PlaySound(*sound)
                : RequestMusic(request.assetId, request.loop));
        if (!succeeded)
        {
            allSucceeded = false;
            WriteDebugMessage(
                "Could not play audio asset: " + request.assetId);
        }
    }
    return allSucceeded;
}

bool GameController::RequestMusic(std::string assetId, bool loop)
{
    if (assetId.empty() || resources_.GetSound(assetId) == nullptr)
    {
        return false;
    }

    if (musicFadePhase_ == MusicFadePhase::In)
    {
        if (assetId == currentMusicId_ && loop == currentMusicLoop_)
        {
            return true;
        }

        pendingMusicId_ = std::move(assetId);
        pendingMusicLoop_ = loop;
        musicFadeStartVolume_ = currentMusicVolume_;
        musicFadeSecondsRemaining_ = MusicFadeOutSeconds;
        musicFadePhase_ = MusicFadePhase::Out;
        return true;
    }

    if (musicFadePhase_ == MusicFadePhase::Out)
    {
        if (assetId == currentMusicId_ && loop == currentMusicLoop_)
        {
            pendingMusicId_.clear();
            musicFadeStartVolume_ = currentMusicVolume_;
            musicFadeSecondsRemaining_ = MusicFadeInSeconds;
            musicFadePhase_ = MusicFadePhase::In;
            return true;
        }
        if (assetId == pendingMusicId_ && loop == pendingMusicLoop_)
        {
            return true;
        }

        pendingMusicId_ = std::move(assetId);
        pendingMusicLoop_ = loop;
        return true;
    }

    if (assetId == currentMusicId_ && loop == currentMusicLoop_)
    {
        return true;
    }

    if (currentMusicId_.empty() || !audio_.IsMusicPlaying())
    {
        const SoundAsset* music = resources_.GetSound(assetId);
        if (music == nullptr || !audio_.PlayMusic(*music, loop))
        {
            return false;
        }
        if (!audio_.SetMusicVolume(0.0F))
        {
            audio_.StopMusic();
            return false;
        }
        currentMusicId_ = std::move(assetId);
        currentMusicLoop_ = loop;
        currentMusicVolume_ = 0.0F;
        musicFadeStartVolume_ = 0.0F;
        musicFadeSecondsRemaining_ = MusicFadeInSeconds;
        musicFadePhase_ = MusicFadePhase::In;
        return true;
    }

    pendingMusicId_ = std::move(assetId);
    pendingMusicLoop_ = loop;
    musicFadeStartVolume_ = currentMusicVolume_;
    musicFadeSecondsRemaining_ = MusicFadeOutSeconds;
    musicFadePhase_ = MusicFadePhase::Out;
    return true;
}

void GameController::RequestMusicStop() noexcept
{
    pendingMusicId_.clear();
    if (currentMusicId_.empty() || !audio_.IsMusicPlaying())
    {
        currentMusicId_.clear();
        musicFadePhase_ = MusicFadePhase::None;
        musicFadeSecondsRemaining_ = 0.0F;
        currentMusicVolume_ = 1.0F;
        return;
    }

    if (musicFadePhase_ == MusicFadePhase::Out)
    {
        return;
    }

    musicFadeStartVolume_ = currentMusicVolume_;
    musicFadeSecondsRemaining_ = MusicFadeOutSeconds;
    musicFadePhase_ = MusicFadePhase::Out;
}

void GameController::UpdateMusic(float deltaTime)
{
    if (musicFadePhase_ == MusicFadePhase::None)
    {
        if (!currentMusicId_.empty() && !audio_.IsMusicPlaying())
        {
            currentMusicId_.clear();
            currentMusicVolume_ = 1.0F;
        }
        return;
    }

    musicFadeSecondsRemaining_ -= std::max(deltaTime, 0.0F);
    if (musicFadeSecondsRemaining_ > 0.0F)
    {
        const float volume = musicFadePhase_ == MusicFadePhase::Out
            ? musicFadeStartVolume_
                * (musicFadeSecondsRemaining_ / MusicFadeOutSeconds)
            : musicFadeStartVolume_
                + (1.0F - musicFadeStartVolume_)
                    * (1.0F
                        - musicFadeSecondsRemaining_ / MusicFadeInSeconds);
        if (!audio_.SetMusicVolume(volume))
        {
            musicFadeSecondsRemaining_ = 0.0F;
        }
        else
        {
            currentMusicVolume_ = volume;
        }
    }

    if (musicFadeSecondsRemaining_ > 0.0F)
    {
        return;
    }

    if (musicFadePhase_ == MusicFadePhase::In)
    {
        if (audio_.SetMusicVolume(1.0F))
        {
            currentMusicVolume_ = 1.0F;
        }
        musicFadePhase_ = MusicFadePhase::None;
        return;
    }

    audio_.StopMusic();
    currentMusicId_.clear();
    currentMusicVolume_ = 1.0F;
    musicFadePhase_ = MusicFadePhase::None;

    if (pendingMusicId_.empty())
    {
        return;
    }

    std::string nextMusicId = std::move(pendingMusicId_);
    pendingMusicId_.clear();
    const bool nextMusicLoop = pendingMusicLoop_;
    const SoundAsset* music = resources_.GetSound(nextMusicId);
    if (music == nullptr || !audio_.PlayMusic(*music, nextMusicLoop))
    {
        WriteDebugMessage("Could not play music asset: " + nextMusicId);
        return;
    }
    if (!audio_.SetMusicVolume(0.0F))
    {
        audio_.StopMusic();
        WriteDebugMessage("Could not fade in music asset: " + nextMusicId);
        return;
    }

    currentMusicId_ = std::move(nextMusicId);
    currentMusicLoop_ = nextMusicLoop;
    currentMusicVolume_ = 0.0F;
    musicFadeStartVolume_ = 0.0F;
    musicFadeSecondsRemaining_ = MusicFadeInSeconds;
    musicFadePhase_ = MusicFadePhase::In;
}

void GameController::PlaySoundAsset(std::string_view assetId)
{
    const SoundAsset* sound = resources_.GetSound(std::string(assetId));
    if (sound == nullptr || !audio_.PlaySound(*sound))
    {
        WriteDebugMessage(
            "Could not play sound asset: " + std::string(assetId));
    }
}

void GameController::ProcessDialogueFeedback()
{
    if (dialogueWindow_.ConsumePageShownFeedback())
    {
        PlaySoundAsset("dialogPage");
    }
}

void GameController::ProcessMainMenuFeedback()
{
    if (mainMenu_.ConsumeOpenFeedback())
    {
        PlaySoundAsset("menuOpen");
    }
    if (mainMenu_.ConsumeCloseFeedback()
        && !dialogueWindow_.IsModalActive())
    {
        PlaySoundAsset("uiExit");
    }
}

bool GameController::UseInventoryItem(const std::string& itemId)
{
    const auto definition = itemDefinitions_.find(itemId);
    if (definition == itemDefinitions_.end()
        || definition->second.useScript.empty()
        || currentScene_ == nullptr
        || luaSystem_.IsCoroutineActive(itemUseCoroutine_))
    {
        return false;
    }

    std::string groundRegionId;
    if (const auto* worldScene =
            dynamic_cast<const WorldScene*>(currentScene_.get()))
    {
        groundRegionId = worldScene->GetPlayerGroundRegionId();
    }

    return luaSystem_.StartItemCoroutine(
        definition->second.useScript,
        itemId,
        currentScene_->GetSceneId(),
        groundRegionId,
        itemUseCoroutine_);
}

bool GameController::SaveGame()
{
    const auto* worldScene = dynamic_cast<const WorldScene*>(
        currentScene_.get());
    if (worldScene == nullptr)
    {
        return false;
    }

    const std::optional<PlayerSpawnPosition> position =
        worldScene->GetPlayerPosition();
    return position.has_value()
        && WriteSaveGame(
            SaveGamePath,
            worldScene->GetSceneId(),
            position->x,
            position->y,
            globalState_,
            inventory_,
            playerState_);
}

bool GameController::LoadGame()
{
    SaveGameData saveGame;
    if (!ReadSaveGame(SaveGamePath, saveGame))
    {
        return false;
    }

    const auto nextLevel = experienceToNextLevelByLevel_.find(
        saveGame.playerState.GetLevel());
    if (nextLevel == experienceToNextLevelByLevel_.end()
        || saveGame.playerState.GetExperience() >= nextLevel->second)
    {
        return false;
    }
    for (const std::string& itemId : saveGame.inventory.GetItemIds())
    {
        if (!itemDefinitions_.contains(itemId))
        {
            return false;
        }
    }

    if (saveGame.sceneId != "boatScene"
        && saveGame.sceneId != "storageRoom")
    {
        // Only WorldScene saves are authored in this first format. Rejecting
        // other destinations before teardown keeps malformed files from
        // discarding the currently running scene.
        return false;
    }

    // Retire Lua-backed objects while they still see the old persistent
    // state. Their DESTROY hooks must not mutate the state being restored.
    UnloadCurrentScene();
    // LuaSystem and MainMenu retain references to these controller-owned
    // objects. Assignment replaces their contents without changing identity.
    globalState_ = std::move(saveGame.globalState);
    inventory_ = std::move(saveGame.inventory);
    playerState_ = std::move(saveGame.playerState);

    SceneParameterMap parameters{
        {"playerX", static_cast<double>(saveGame.playerX)},
        {"playerY", static_cast<double>(saveGame.playerY)}};
    return ChangeScene(SceneChangeRequest{
        "saveGame",
        std::move(saveGame.sceneId),
        std::move(parameters)});
}

void GameController::UnloadCurrentScene()
{
    if (currentScene_ != nullptr)
    {
        // Scene resources and Lua-backed objects must be retired together;
        // objects cannot keep references into the next scene's resource scope.
        currentScene_->Shutdown();
        currentScene_.reset();
    }
    // DESTROY hooks may request object work while a scene is shutting down.
    // Those requests belong to the retired scene and must not leak into the
    // next WorldScene.
    luaSystem_.ClearSceneActions();
    resources_.ClearScene();
}

bool GameController::ChangeScene(const SceneChangeRequest& request)
{
    std::string playerSpawnPointId;
    const auto spawnPoint = request.parameters.find("playerSpawnPoint");
    if (spawnPoint != request.parameters.end())
    {
        const auto* value = std::get_if<std::string>(&spawnPoint->second);
        if (value == nullptr || value->empty())
        {
            WriteDebugMessage(
                "Scene change has an invalid playerSpawnPoint parameter.");
            return false;
        }
        playerSpawnPointId = *value;
    }

    // exact player placement during a scene change
    std::optional<PlayerSpawnPosition> playerPosition;
    const auto playerX = request.parameters.find("playerX");
    const auto playerY = request.parameters.find("playerY");
    if ((playerX == request.parameters.end())
        != (playerY == request.parameters.end()))
    {
        WriteDebugMessage(
            "Scene change must provide both playerX and playerY.");
        return false;
    }
    if (playerX != request.parameters.end())
    {
        // Scene parameters are variants, so get_if returns nullptr unless
        // each value really is a YAML number. Exact return coordinates must
        // also be finite and cannot be combined with playerSpawnPoint: those
        // are two different, conflicting instructions for placing the player.
        const auto* x = std::get_if<double>(&playerX->second);
        const auto* y = std::get_if<double>(&playerY->second);
        if (x == nullptr
            || y == nullptr
            || !std::isfinite(*x)
            || !std::isfinite(*y)
            || !playerSpawnPointId.empty())
        {
            WriteDebugMessage(
                "Scene change has invalid or conflicting player coordinates.");
            return false;
        }
        playerPosition = PlayerSpawnPosition{
            static_cast<float>(*x),
            static_cast<float>(*y)};
    }

    std::unique_ptr<BaseScene> newScene;
    if (request.destination == "boatScene")
    {
        newScene = std::make_unique<BoatScene>(
            resources_,
            globalState_,
            luaSystem_,
            playerSpawnPointId,
            playerPosition);
    }
    else if (request.destination == "storageRoom")
    {
        newScene = std::make_unique<StorageRoomScene>(
            resources_,
            globalState_,
            luaSystem_,
            playerSpawnPointId,
            playerPosition);
    }
    else if (request.destination == "ending")
    {
        newScene = std::make_unique<EndingScene>(resources_, luaSystem_);
    }
    else if (request.destination == "battleArena")
    {
        // Direct config startup uses the map-authored active enemy fallback.
        BattleRequest battleRequest;
        battleRequest.returnSceneId = "storageRoom";
        newScene = std::make_unique<BattleArenaScene>(
            resources_,
            audio_,
            luaSystem_,
            inventory_,
            playerState_,
            experienceToNextLevelByLevel_,
            itemDefinitions_,
            std::move(battleRequest));
    }
    else
    {
        WriteDebugMessage(
            "Unknown scene destination: " + request.destination);
        return false;
    }

    return LoadScene(std::move(newScene));
}

void GameController::Render()
{
    if (currentScene_ != nullptr && graphics_ != nullptr)
    {
        currentScene_->Render(*graphics_);
        // This final screen-space pass is the demo's GUI layer. It never uses
        // the world camera and therefore remains fixed to the render surface.
        dialogueWindow_.Render(*graphics_, resources_);
        mainMenu_.Render(*graphics_, resources_);
        if (const auto* world = dynamic_cast<const WorldScene*>(currentScene_.get()))
        {
            world->RenderTransition(*graphics_);
        }
    }
}
