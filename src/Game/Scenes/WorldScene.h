#pragma once

#include "../../Framework/Scenes/BaseScene.h"
#include "../World/GroundRegionMap.h"
#include "../Effects/WeatherSystem.h"
#include "../Scripting/LuaSystem.h"

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class Player;
class CollisionMask;
class GlobalState;
class InteractableObject;
class LuaSystem;
class ObjectLayer;
struct BattleRequest;
struct ObjectActionRequest;

struct PlayerSpawnPosition
{
    float x = 0.0F;
    float y = 0.0F;
};

class WorldScene : public BaseScene
{
public:
    WorldScene(
        ResourceManager& resources,
        std::string mapPath,
        std::string collisionMaskId,
        GlobalState& globalState,
        LuaSystem& luaSystem,
        std::string requestedPlayerSpawnPointId = {},
        std::optional<PlayerSpawnPosition> requestedPlayerPosition = {});

    bool SetCameraTarget(std::string_view objectId) override;
    bool AddPlayerSpawnPoint(std::string id, float x, float y);
    bool AddWaypoint(std::string id, float x, float y);
    void RenderTransition(const Graphics& graphics) const;
    bool AddGroundRegion(
        std::string id,
        std::uint32_t color,
        GroundBehavior behavior,
        GroundParameterMap parameters,
        std::string hook);
    bool OnGameObjectCreated(
        BaseGameObject& object,
        ObjectLayer& owner) override;
    bool Init() override;
    void Update(float deltaTime, const InputState& input) override;
    void Render(const Graphics& graphics) override;
    void Shutdown() override;
    std::optional<SceneChangeRequest>
        TakeSceneChangeRequest() override;
    bool AddReturnContext(BattleRequest& request) const;
    std::optional<PlayerSpawnPosition> GetPlayerPosition() const noexcept;
    std::string GetPlayerGroundRegionId() const;

private:
    struct PlayerSpawnPoint
    {
        float x = 0.0F;
        float y = 0.0F;
    };

    struct ActiveObjectFade
    {
        std::string objectId;
        std::uint64_t coroutineHandle = 0;
    };

    struct ActiveObjectMove
    {
        std::string objectId;
        PlayerSpawnPosition destination;
        float speed = 0.0F;
        std::uint64_t coroutineHandle = 0;
    };

    struct ActiveWait
    {
        float remainingSeconds = 0.0F;
        std::uint64_t coroutineHandle = 0;
    };

    void UpdateCamera();
    void UpdateGroundBehavior();
    bool ProcessSceneActions();
    bool StartObjectFade(
        const std::string& objectId,
        float durationSeconds,
        std::uint64_t coroutineHandle);
    void CompleteObjectFades();
    void CompleteWeatherFade();
    std::optional<bool> ProcessCutsceneAction(const SceneActionRequest& request);
    void UpdateCutsceneActions(float deltaTime);
    void CancelCutscene(std::uint64_t handle);
    BaseGameObject* FindObject(std::string_view objectId) noexcept;
    bool SpawnObject(const ObjectActionRequest& request);
    bool DestroyObject(std::string_view objectId);
    void UnregisterGameObject(BaseGameObject& object) noexcept;

    std::string collisionMaskId_;
    std::string cameraTargetId_;
    std::string requestedPlayerSpawnPointId_;
    std::optional<PlayerSpawnPosition> requestedPlayerPosition_;
    const CollisionMask* collisionMask_ = nullptr;
    GroundRegionMap groundRegions_;
    Player* player_ = nullptr;
    Player* cameraTarget_ = nullptr;
    GlobalState& globalState_;
    LuaSystem& luaSystem_;
    WeatherSystem weather_;
    std::unordered_map<std::string, PlayerSpawnPosition> waypoints_;
    std::vector<ActiveObjectMove> activeObjectMoves_;
    std::vector<ActiveWait> activeWaits_;
    std::vector<LuaCoroutineHandle> cutsceneCoroutines_;
    std::string scriptedCameraTarget_;
    float scriptedCameraOffsetX_ = 0.0F;
    float scriptedCameraOffsetY_ = 0.0F;
    bool scriptedCamera_ = false;
    bool scriptedPlayerAnimation_ = false;
    std::uint64_t cameraMoveCoroutine_ = 0;
    std::uint64_t sceneFadeCoroutine_ = 0;
    float sceneFadeDuration_ = 0.0F;
    float sceneFadeElapsed_ = 0.0F;
    float sceneFadeOpacity_ = 0.0F;
    std::uint64_t weatherFadeCoroutine_ = 0;
    std::vector<InteractableObject*> interactables_;
    std::vector<ActiveObjectFade> activeObjectFades_;
    std::unordered_map<std::string, ObjectLayer*> objectOwners_;
    std::unordered_map<std::string, PlayerSpawnPoint> playerSpawnPoints_;
    std::string activeGroundRegionId_;
    std::optional<SceneChangeRequest> pendingSceneChange_;
};
