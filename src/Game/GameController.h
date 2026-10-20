#pragma once

#include "../Framework/Resources/ResourceManager.h"
#include "../Platform/Audio/Audio.h"
#include "State/GlobalState.h"
#include "State/Inventory.h"
#include "Items/ItemDefinition.h"
#include "State/PlayerState.h"
#include "../Framework/Scenes/SceneChangeRequest.h"
#include "Scripting/LuaSystem.h"
#include "UI/DialogueWindow.h"
#include "UI/MainMenu.h"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

struct ID2D1RenderTarget;
class BaseScene;
class Graphics;
struct InputState;

class GameController
{
public:
    explicit GameController(float dialogCharactersPerSecond);
    ~GameController();

    GameController(const GameController&) = delete;
    GameController& operator=(const GameController&) = delete;

    bool InitGame(
        ID2D1RenderTarget* renderTarget,
        std::string_view startSceneId,
        bool loadSaveWhenPresent);
    bool LoadScene(std::unique_ptr<BaseScene> newScene);
    bool SetRenderTarget(ID2D1RenderTarget* renderTarget);
    void Update(float deltaTime, const InputState& input);
    void Render();
    bool ConsumeQuitRequest() noexcept;

private:
    enum class MusicFadePhase
    {
        None,
        Out,
        In
    };

    bool ChangeScene(const SceneChangeRequest& request);
    bool LoadGameData();
    bool ProcessAudioRequests();
    bool RequestMusic(std::string assetId, bool loop = true);
    void RequestMusicStop() noexcept;
    void UpdateMusic(float deltaTime);
    void PlaySoundAsset(std::string_view assetId);
    void ProcessMainMenuFeedback();
    void ProcessDialogueFeedback();
    bool UseInventoryItem(const std::string& itemId);
    bool SaveGame();
    bool LoadGame();
    void UnloadCurrentScene();

    GlobalState globalState_;
    Inventory inventory_;
    // Long-lived player data belongs above scenes so battle rewards survive
    // scene replacement and can later be serialized by Save/Load.
    PlayerState playerState_;
    std::unordered_map<int, int> experienceToNextLevelByLevel_;
    std::unordered_map<std::string, ItemDefinition> itemDefinitions_;
    DialogueWindow dialogueWindow_;
    LuaSystem luaSystem_;
    MainMenu mainMenu_;
    ResourceManager resources_;
    Audio audio_;
    std::string currentMusicId_;
    std::string pendingMusicId_;
    float musicFadeSecondsRemaining_ = 0.0F;
    float musicFadeStartVolume_ = 1.0F;
    float currentMusicVolume_ = 1.0F;
    bool currentMusicLoop_ = true;
    bool pendingMusicLoop_ = true;
    MusicFadePhase musicFadePhase_ = MusicFadePhase::None;
    std::unique_ptr<BaseScene> currentScene_;
    std::unique_ptr<Graphics> graphics_;
    LuaCoroutineHandle itemUseCoroutine_ = InvalidLuaCoroutineHandle;
    bool quitRequested_ = false;
};
