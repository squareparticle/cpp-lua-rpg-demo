#pragma once

#include "../World/GroundRegionMap.h"
#include "../Battle/BattleDecision.h"
#include "../Battle/BattleRequest.h"
#include "../../Framework/Scenes/SceneChangeRequest.h"

#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

class GlobalState;
class Inventory;
class DialogueWindow;
struct lua_State;

using LuaCoroutineHandle = std::uint64_t;
inline constexpr LuaCoroutineHandle InvalidLuaCoroutineHandle = 0;

enum class ObjectActionType
{
    Spawn,
    Destroy
};

struct ObjectActionRequest
{
    ObjectActionType type = ObjectActionType::Spawn;
    std::string value;
    std::optional<std::string> idOverride;
    std::optional<float> xOverride;
    std::optional<float> yOverride;
};

struct FadeObjectRequest
{
    std::string objectId;
    float durationSeconds = 0.0F;
    LuaCoroutineHandle coroutineHandle = InvalidLuaCoroutineHandle;
};

struct FadeWeatherRequest
{
    float durationSeconds = 0.0F;
    LuaCoroutineHandle coroutineHandle = InvalidLuaCoroutineHandle;
};

struct MoveObjectRequest
{
    std::string objectId;
    std::string waypoint;
    float speed = 0.0F;
    LuaCoroutineHandle coroutineHandle = InvalidLuaCoroutineHandle;
};

struct PlayAnimationRequest
{
    std::string objectId;
    std::string animation;
};

struct SetObjectPositionRequest
{
    std::string objectId;
    std::string waypoint;
};

struct StartCutsceneRequest
{
    std::string scriptPath;
};

struct FollowObjectRequest
{
    std::string objectId;
    float offsetX = 0.0F;
    float offsetY = 0.0F;
};

struct MoveCameraRequest
{
    std::string waypoint;
    float durationSeconds = 0.0F;
    LuaCoroutineHandle coroutineHandle = InvalidLuaCoroutineHandle;
};

struct WaitRequest
{
    float durationSeconds = 0.0F;
    LuaCoroutineHandle coroutineHandle = InvalidLuaCoroutineHandle;
};

struct FadeSceneRequest
{
    float durationSeconds = 0.0F;
    LuaCoroutineHandle coroutineHandle = InvalidLuaCoroutineHandle;
};

using GroundRegionChange =
    std::variant<GroundBehavior, GroundParameterMap>;

struct GroundRegionRequest
{
    std::string regionId;
    GroundRegionChange change = GroundBehavior::Walkable;
};

using SceneActionRequest =
    std::variant<ObjectActionRequest, FadeObjectRequest,
        FadeWeatherRequest, GroundRegionRequest, MoveObjectRequest,
        PlayAnimationRequest, SetObjectPositionRequest, StartCutsceneRequest,
        FollowObjectRequest, MoveCameraRequest,
        WaitRequest, FadeSceneRequest, SceneChangeRequest>;

enum class AudioRequestType
{
    PlaySound,
    PlayMusic,
    StopMusic
};

struct AudioRequest
{
    AudioRequestType type = AudioRequestType::PlaySound;
    std::string assetId;
    bool loop = false;
};

class LuaSystem
{
public:
    LuaSystem(
        GlobalState& globalState,
        Inventory& inventory,
        DialogueWindow& dialogueWindow);
    ~LuaSystem();

    LuaSystem(const LuaSystem&) = delete;
    LuaSystem& operator=(const LuaSystem&) = delete;

    bool Initialize();
    bool ExecuteFile(const std::string& filename, const std::string& objectId);
    bool ExecuteSceneFile(
        const std::string& filename,
        const std::string& sceneId);
    bool ChooseBattleAction(
        const std::string& filename,
        const BattleDecisionContext& context,
        BattleActionIntent& intent);
    bool StartCoroutine(
        const std::string& filename,
        const std::string& objectId,
        LuaCoroutineHandle& handle);
    bool StartSceneCoroutine(
        const std::string& filename,
        const std::string& sceneId,
        LuaCoroutineHandle& handle);
    bool StartItemCoroutine(
        const std::string& filename,
        const std::string& itemId,
        const std::string& sceneId,
        const std::string& groundRegionId,
        LuaCoroutineHandle& handle);
    bool ResumeCoroutine(
        LuaCoroutineHandle& handle,
        std::string_view trigger);
    void StopCoroutine(LuaCoroutineHandle& handle) noexcept;
    bool IsCoroutineActive(LuaCoroutineHandle handle) const noexcept;
    bool IsCoroutineWaitingFor(
        LuaCoroutineHandle handle,
        std::string_view trigger) const noexcept;
    bool CompleteDialog();
    std::optional<std::string> TakeDialogExitSoundOverride() noexcept;
    bool IsPlayerInputEnabled() const noexcept;
    void SetPlayerInputEnabled(bool enabled) noexcept;
    bool PopSceneAction(SceneActionRequest& request);
    bool PopBattleRequest(BattleRequest& request);
    bool PopAudioRequest(AudioRequest& request);
    void ClearSceneActions() noexcept;

private:
    struct CoroutineState
    {
        lua_State* thread = nullptr;
        int registryReference = 0;
        std::string objectId;
        std::string itemId;
        std::string sceneId;
        std::string groundRegionId;
        std::string awaitedTrigger;
    };

    static LuaSystem* GetSelf(lua_State* state);
    static int SetTag(lua_State* state);
    static int GetTag(lua_State* state);
    static int AddInventoryItem(lua_State* state);
    static int RemoveInventoryItem(lua_State* state);
    static int HasInventoryItem(lua_State* state);
    static int PlaySound(lua_State* state);
    static int PlayMusic(lua_State* state);
    static int StopMusic(lua_State* state);
    static int SpawnObject(lua_State* state);
    static int DestroyObject(lua_State* state);
    static int FadeOutObject(lua_State* state);
    static int FadeOutWeather(lua_State* state);
    static int MoveObjectTo(lua_State* state);
    static int SetObjectPosition(lua_State* state);
    static int StartCutscene(lua_State* state);
    static int PlayAnimation(lua_State* state);
    static int FollowObject(lua_State* state);
    static int MoveCameraTo(lua_State* state);
    static int Wait(lua_State* state);
    static int FadeOutScene(lua_State* state);
    static int ChangeScene(lua_State* state);
    static LuaCoroutineHandle GetCallingCoroutine(lua_State* state);
    static int SetGroundBehavior(lua_State* state);
    static int SetGroundParams(lua_State* state);
    static int StartBattle(lua_State* state);
    static int DebugMessage(lua_State* state);
    static int Say(lua_State* state);
    static int WaitForTrigger(lua_State* state);
    void RegisterFunction(const char* name, int (*function)(lua_State*));
    bool StartCoroutineWithContext(
        const std::string& filename,
        CoroutineState runtime,
        LuaCoroutineHandle& handle);
    void SetCurrentContext(
        std::string_view objectId,
        std::string_view itemId,
        std::string_view sceneId,
        std::string_view groundRegionId);
    bool RunCoroutine(LuaCoroutineHandle& handle);
    void ReleaseCoroutine(LuaCoroutineHandle& handle) noexcept;

    GlobalState& globalState_;
    Inventory& inventory_;
    DialogueWindow& dialogueWindow_;
    lua_State* state_ = nullptr;
    std::unordered_map<LuaCoroutineHandle, CoroutineState> coroutines_;
    std::deque<SceneActionRequest> sceneActions_;
    std::deque<BattleRequest> battleRequests_;
    std::deque<AudioRequest> audioRequests_;
    LuaCoroutineHandle nextCoroutineHandle_ = 1;
    LuaCoroutineHandle dialogCoroutineHandle_ = InvalidLuaCoroutineHandle;
    std::optional<std::string> dialogExitSoundOverride_;
};
