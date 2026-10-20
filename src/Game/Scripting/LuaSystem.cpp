#include "LuaSystem.h"

#include "../../Platform/DebugLog.h"
#include "../State/GlobalState.h"
#include "../State/Inventory.h"
#include "../UI/DialogueWindow.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <cmath>
#include <limits>
#include <utility>

namespace
{
constexpr std::string_view DialogTrigger = "dialog";
constexpr std::string_view ObjectFadeTrigger = "objectFade";
constexpr std::string_view WeatherFadeTrigger = "weatherFade";

float ReadPositiveFloat(lua_State* state, int index)
{
    const lua_Number value = luaL_checknumber(state, index);
    if (!std::isfinite(value) || value <= 0.0
        || value > std::numeric_limits<float>::max()
        || static_cast<float>(value) <= 0.0F)
    {
        luaL_argerror(state, index, "value must be a positive finite float");
    }
    return static_cast<float>(value);
}

const char* ReadIdentifier(lua_State* state, int index)
{
    const char* value = luaL_checkstring(state, index);
    if (value[0] == '\0')
    {
        luaL_argerror(state, index, "identifier cannot be empty");
    }
    return value;
}

float ReadOptionalOffset(lua_State* state, int index)
{
    const lua_Number value = luaL_optnumber(state, index, 0.0);
    if (!std::isfinite(value)
        || std::abs(value) > std::numeric_limits<float>::max())
    {
        luaL_argerror(state, index, "offset must be a finite float");
    }
    return static_cast<float>(value);
}

void SetStringField(
    lua_State* state,
    const char* name,
    const std::string& value)
{
    lua_pushlstring(state, value.c_str(), value.size());
    lua_setfield(state, -2, name);
}

void SetIntegerField(lua_State* state, const char* name, int value)
{
    lua_pushinteger(state, static_cast<lua_Integer>(value));
    lua_setfield(state, -2, name);
}

void PushBattleActor(lua_State* state, const BattleActorSnapshot& actor)
{
    lua_createtable(state, 0, 10);
    SetStringField(state, "id", actor.id);
    SetStringField(state, "name", actor.name);
    SetIntegerField(state, "level", actor.level);
    SetIntegerField(state, "hp", actor.hp);
    SetIntegerField(state, "maxHp", actor.maxHp);
    SetIntegerField(state, "mp", actor.mp);
    SetIntegerField(state, "maxMp", actor.maxMp);
    SetIntegerField(state, "attack", actor.attack);
    SetIntegerField(state, "defense", actor.defense);

    lua_createtable(
        state,
        static_cast<int>(actor.moveIds.size()),
        0);
    lua_Integer moveIndex = 1;
    for (const std::string& moveId : actor.moveIds)
    {
        lua_pushlstring(state, moveId.c_str(), moveId.size());
        lua_seti(state, -2, moveIndex++);
    }
    lua_setfield(state, -2, "moves");
}

void PushBattleContext(
    lua_State* state,
    const BattleDecisionContext& context)
{
    lua_createtable(state, 0, 3);
    PushBattleActor(state, context.actor);
    lua_setfield(state, -2, "actor");
    PushBattleActor(state, context.player);
    lua_setfield(state, -2, "player");

    lua_createtable(
        state,
        static_cast<int>(context.enemies.size()),
        0);
    lua_Integer enemyIndex = 1;
    for (const BattleActorSnapshot& enemy : context.enemies)
    {
        PushBattleActor(state, enemy);
        lua_seti(state, -2, enemyIndex++);
    }
    lua_setfield(state, -2, "enemies");
}

bool ReadStringField(
    lua_State* state,
    int tableIndex,
    const char* name,
    std::string& value)
{
    tableIndex = lua_absindex(state, tableIndex);
    lua_getfield(state, tableIndex, name);
    if (lua_type(state, -1) != LUA_TSTRING)
    {
        lua_pop(state, 1);
        return false;
    }

    std::size_t length = 0;
    const char* text = lua_tolstring(state, -1, &length);
    value.assign(text, length);
    lua_pop(state, 1);
    return !value.empty();
}
}

LuaSystem::LuaSystem(
    GlobalState& globalState,
    Inventory& inventory,
    DialogueWindow& dialogueWindow)
    : globalState_(globalState),
      inventory_(inventory),
      dialogueWindow_(dialogueWindow)
{
}

LuaSystem::~LuaSystem()
{
    if (state_ != nullptr)
    {
        coroutines_.clear();
        lua_close(state_);
    }
}

bool LuaSystem::Initialize()
{
    if (state_ != nullptr)
    {
        return true;
    }

    state_ = luaL_newstate();
    if (state_ == nullptr)
    {
        WriteDebugMessage("Lua could not create its state.");
        return false;
    }

    luaL_openlibs(state_);
    lua_pushboolean(state_, true);
    lua_setglobal(state_, "playerInput");
    RegisterFunction("SetTag", SetTag);
    RegisterFunction("GetTag", GetTag);
    RegisterFunction("AddInventoryItem", AddInventoryItem);
    RegisterFunction("RemoveInventoryItem", RemoveInventoryItem);
    RegisterFunction("HasInventoryItem", HasInventoryItem);
    RegisterFunction("PlaySound", PlaySound);
    RegisterFunction("PlayMusic", PlayMusic);
    RegisterFunction("StopMusic", StopMusic);
    RegisterFunction("SpawnObject", SpawnObject);
    RegisterFunction("DestroyObject", DestroyObject);
    RegisterFunction("FadeOutObject", FadeOutObject);
    RegisterFunction("FadeOutWeather", FadeOutWeather);
    RegisterFunction("MoveObjectTo", MoveObjectTo);
    RegisterFunction("SetObjectPosition", SetObjectPosition);
    RegisterFunction("StartCutscene", StartCutscene);
    RegisterFunction("PlayAnimation", PlayAnimation);
    RegisterFunction("FollowObject", FollowObject);
    RegisterFunction("MoveCameraTo", MoveCameraTo);
    RegisterFunction("Wait", Wait);
    RegisterFunction("FadeOutScene", FadeOutScene);
    RegisterFunction("ChangeScene", ChangeScene);
    RegisterFunction("SetGroundBehavior", SetGroundBehavior);
    RegisterFunction("SetGroundParams", SetGroundParams);
    RegisterFunction("StartBattle", StartBattle);
    RegisterFunction("DebugMessage", DebugMessage);
    RegisterFunction("Say", Say);
    RegisterFunction("WaitForTrigger", WaitForTrigger);
    return true;
}

bool LuaSystem::ExecuteFile(
    const std::string& filename,
    const std::string& objectId)
{
    if (state_ == nullptr)
    {
        WriteDebugMessage("Lua execution requested before initialization.");
        return false;
    }

    SetCurrentContext(objectId, {}, {}, {});

    if (luaL_loadfile(state_, filename.c_str()) != LUA_OK
        || lua_pcall(state_, 0, 0, 0) != LUA_OK)
    {
        const char* error = lua_tostring(state_, -1);
        WriteDebugMessage(
            "Lua script failed '" + filename + "': "
            + (error != nullptr ? error : "unknown error"));
        lua_pop(state_, 1);
        return false;
    }

    return true;
}

bool LuaSystem::ExecuteSceneFile(
    const std::string& filename,
    const std::string& sceneId)
{
    if (state_ == nullptr || filename.empty() || sceneId.empty())
    {
        WriteDebugMessage("Scene Lua execution requested without context.");
        return false;
    }

    // A scene script is not an object or item hook. Preserve only its real
    // context so it cannot accidentally target a made-up runtime object.
    SetCurrentContext({}, {}, sceneId, {});

    if (luaL_loadfile(state_, filename.c_str()) != LUA_OK
        || lua_pcall(state_, 0, 0, 0) != LUA_OK)
    {
        const char* error = lua_tostring(state_, -1);
        WriteDebugMessage(
            "Scene Lua script failed for '" + sceneId + "' ("
            + filename + "): "
            + (error != nullptr ? error : "unknown error"));
        lua_pop(state_, 1);
        return false;
    }

    return true;
}

bool LuaSystem::ChooseBattleAction(
    const std::string& filename,
    const BattleDecisionContext& context,
    BattleActionIntent& intent)
{
    intent = {};
    if (state_ == nullptr || filename.empty())
    {
        WriteDebugMessage("Battle tactics requested without Lua context.");
        return false;
    }

    const int originalStackTop = lua_gettop(state_);
    SetCurrentContext({}, {}, {}, {});

    if (luaL_loadfile(state_, filename.c_str()) != LUA_OK)
    {
        const char* error = lua_tostring(state_, -1);
        WriteDebugMessage(
            "Battle tactics failed to load '" + filename + "': "
            + (error != nullptr ? error : "unknown error"));
        lua_settop(state_, originalStackTop);
        return false;
    }

    // The loaded Lua file is called as a chunk with one read-only-by-copy
    // snapshot argument. It must return exactly one move/target intent table.
    PushBattleContext(state_, context);
    if (lua_pcall(state_, 1, 1, 0) != LUA_OK)
    {
        const char* error = lua_tostring(state_, -1);
        WriteDebugMessage(
            "Battle tactics failed '" + filename + "': "
            + (error != nullptr ? error : "unknown error"));
        lua_settop(state_, originalStackTop);
        return false;
    }

    BattleActionIntent selected;
    const bool valid = lua_istable(state_, -1)
        && ReadStringField(state_, -1, "move", selected.moveId)
        && ReadStringField(state_, -1, "target", selected.targetId);
    lua_settop(state_, originalStackTop);
    if (!valid)
    {
        WriteDebugMessage(
            "Battle tactics must return non-empty move and target strings: "
            + filename);
        return false;
    }

    intent = std::move(selected);
    return true;
}

bool LuaSystem::StartCoroutine(
    const std::string& filename,
    const std::string& objectId,
    LuaCoroutineHandle& handle)
{
    return StartCoroutineWithContext(
        filename,
        CoroutineState{nullptr, 0, objectId, {}, {}, {}, {}},
        handle);
}

bool LuaSystem::StartSceneCoroutine(
    const std::string& filename,
    const std::string& sceneId,
    LuaCoroutineHandle& handle)
{
    if (sceneId.empty())
    {
        return false;
    }
    return StartCoroutineWithContext(filename,
        CoroutineState{nullptr, 0, {}, {}, sceneId, {}, {}}, handle);
}

bool LuaSystem::StartItemCoroutine(
    const std::string& filename,
    const std::string& itemId,
    const std::string& sceneId,
    const std::string& groundRegionId,
    LuaCoroutineHandle& handle)
{
    if (itemId.empty() || sceneId.empty())
    {
        WriteDebugMessage("Item Lua coroutine requested without context.");
        return false;
    }

    return StartCoroutineWithContext(
        filename,
        CoroutineState{
            nullptr,
            0,
            {},
            itemId,
            sceneId,
            groundRegionId,
            {}},
        handle);
}

bool LuaSystem::StartCoroutineWithContext(
    const std::string& filename,
    CoroutineState runtime,
    LuaCoroutineHandle& handle)
{
    StopCoroutine(handle);
    if (state_ == nullptr || filename.empty())
    {
        WriteDebugMessage("Lua coroutine requested before initialization.");
        return false;
    }

    lua_State* thread = lua_newthread(state_);
    const int registryReference = luaL_ref(state_, LUA_REGISTRYINDEX);

    handle = nextCoroutineHandle_++;
    if (handle == InvalidLuaCoroutineHandle)
    {
        handle = nextCoroutineHandle_++;
    }

    runtime.thread = thread;
    runtime.registryReference = registryReference;
    coroutines_.emplace(handle, std::move(runtime));

    if (luaL_loadfile(thread, filename.c_str()) != LUA_OK)
    {
        const char* error = lua_tostring(thread, -1);
        WriteDebugMessage(
            "Lua coroutine failed to load '" + filename + "': "
            + (error != nullptr ? error : "unknown error"));
        lua_pop(thread, 1);
        ReleaseCoroutine(handle);
        return false;
    }

    return RunCoroutine(handle);
}

bool LuaSystem::ResumeCoroutine(
    LuaCoroutineHandle& handle,
    std::string_view trigger)
{
    const auto coroutine = coroutines_.find(handle);
    if (coroutine == coroutines_.end())
    {
        handle = InvalidLuaCoroutineHandle;
        return false;
    }

    if (trigger != coroutine->second.awaitedTrigger)
    {
        return true;
    }

    return RunCoroutine(handle);
}

void LuaSystem::StopCoroutine(LuaCoroutineHandle& handle) noexcept
{
    if (handle != InvalidLuaCoroutineHandle && handle == dialogCoroutineHandle_)
    {
        dialogueWindow_.Close();
        dialogExitSoundOverride_.reset();
    }
    ReleaseCoroutine(handle);
}

bool LuaSystem::IsCoroutineActive(LuaCoroutineHandle handle) const noexcept
{
    return coroutines_.contains(handle);
}

bool LuaSystem::IsCoroutineWaitingFor(
    LuaCoroutineHandle handle,
    std::string_view trigger) const noexcept
{
    const auto coroutine = coroutines_.find(handle);
    return coroutine != coroutines_.end()
        && coroutine->second.awaitedTrigger == trigger;
}

bool LuaSystem::CompleteDialog()
{
    if (dialogCoroutineHandle_ == InvalidLuaCoroutineHandle)
    {
        dialogueWindow_.Close();
        return false;
    }

    const bool resumed = ResumeCoroutine(
        dialogCoroutineHandle_,
        DialogTrigger);
    const bool hasNextPage =
        dialogCoroutineHandle_ != InvalidLuaCoroutineHandle
        && IsCoroutineWaitingFor(dialogCoroutineHandle_, DialogTrigger);
    if (!hasNextPage)
    {
        // A following Say runs synchronously during ResumeCoroutine and opens
        // the next page while the window is Advancing. If Lua instead finishes
        // or waits for a different trigger, this dialog sequence is over.
        dialogCoroutineHandle_ = InvalidLuaCoroutineHandle;
        dialogueWindow_.Close();
    }
    return resumed;
}

std::optional<std::string> LuaSystem::TakeDialogExitSoundOverride() noexcept
{
    return std::exchange(dialogExitSoundOverride_, std::nullopt);
}

bool LuaSystem::PopSceneAction(SceneActionRequest& request)
{
    if (sceneActions_.empty())
    {
        return false;
    }

    request = std::move(sceneActions_.front());
    sceneActions_.pop_front();
    return true;
}

bool LuaSystem::PopBattleRequest(BattleRequest& request)
{
    if (battleRequests_.empty())
    {
        return false;
    }

    // "Pop" refers only to consuming the oldest queued Lua request. This is
    // not a scene stack and does not remember which scene started the battle.
    request = std::move(battleRequests_.front());
    battleRequests_.pop_front();
    return true;
}

bool LuaSystem::PopAudioRequest(AudioRequest& request)
{
    if (audioRequests_.empty())
    {
        return false;
    }

    request = std::move(audioRequests_.front());
    audioRequests_.pop_front();
    return true;
}

bool LuaSystem::IsPlayerInputEnabled() const noexcept
{
    if (state_ == nullptr)
    {
        return true;
    }

    // Only an explicit Boolean false disables control. This is transient
    // script state, not a persistent quest tag or a physical keyboard setting.
    lua_getglobal(state_, "playerInput");
    const bool enabled = !lua_isboolean(state_, -1)
        || lua_toboolean(state_, -1) != 0;
    lua_pop(state_, 1);
    return enabled;
}

void LuaSystem::SetPlayerInputEnabled(bool enabled) noexcept
{
    if (state_ != nullptr)
    {
        lua_pushboolean(state_, enabled);
        lua_setglobal(state_, "playerInput");
    }
}

void LuaSystem::ClearSceneActions() noexcept
{
    sceneActions_.clear();
    battleRequests_.clear();
    audioRequests_.clear();
    if (state_ != nullptr)
    {
        lua_pushboolean(state_, true);
        lua_setglobal(state_, "playerInput");
    }
}

LuaSystem* LuaSystem::GetSelf(lua_State* state)
{
    return static_cast<LuaSystem*>(lua_touserdata(state, lua_upvalueindex(1)));
}

int LuaSystem::SetTag(lua_State* state)
{
    LuaSystem* self = GetSelf(state);
    const char* name = luaL_checkstring(state, 1);
    const bool value = lua_toboolean(state, 2) != 0;
    self->globalState_.SetTag(name, value);
    return 0;
}

int LuaSystem::GetTag(lua_State* state)
{
    LuaSystem* self = GetSelf(state);
    const char* name = luaL_checkstring(state, 1);
    lua_pushboolean(state, self->globalState_.GetTag(name));
    return 1;
}

int LuaSystem::AddInventoryItem(lua_State* state)
{
    LuaSystem* self = GetSelf(state);
    std::size_t length = 0;
    const char* itemId = luaL_checklstring(state, 1, &length);
    if (length == 0)
    {
        return luaL_argerror(state, 1, "inventory item ID cannot be empty");
    }

    lua_pushboolean(
        state,
        self->inventory_.AddItem(std::string(itemId, length)));
    return 1;
}

int LuaSystem::RemoveInventoryItem(lua_State* state)
{
    LuaSystem* self = GetSelf(state);
    std::size_t length = 0;
    const char* itemId = luaL_checklstring(state, 1, &length);
    if (length == 0)
    {
        return luaL_argerror(state, 1, "inventory item ID cannot be empty");
    }

    lua_pushboolean(
        state,
        self->inventory_.RemoveItem(std::string(itemId, length)));
    return 1;
}

int LuaSystem::HasInventoryItem(lua_State* state)
{
    LuaSystem* self = GetSelf(state);
    std::size_t length = 0;
    const char* itemId = luaL_checklstring(state, 1, &length);
    if (length == 0)
    {
        return luaL_argerror(state, 1, "inventory item ID cannot be empty");
    }

    lua_pushboolean(
        state,
        self->inventory_.HasItem(std::string(itemId, length)));
    return 1;
}

int LuaSystem::PlaySound(lua_State* state)
{
    LuaSystem* self = GetSelf(state);
    std::size_t length = 0;
    const char* assetId = luaL_checklstring(state, 1, &length);
    if (length == 0)
    {
        return luaL_argerror(state, 1, "sound asset ID cannot be empty");
    }

    self->audioRequests_.push_back(AudioRequest{
        AudioRequestType::PlaySound,
        std::string(assetId, length),
        false});
    return 0;
}

int LuaSystem::PlayMusic(lua_State* state)
{
    LuaSystem* self = GetSelf(state);
    std::size_t length = 0;
    const char* assetId = luaL_checklstring(state, 1, &length);
    if (length == 0)
    {
        return luaL_argerror(state, 1, "music asset ID cannot be empty");
    }

    const bool loop = lua_gettop(state) < 2
        || lua_toboolean(state, 2) != 0;
    self->audioRequests_.push_back(AudioRequest{
        AudioRequestType::PlayMusic,
        std::string(assetId, length),
        loop});
    return 0;
}

int LuaSystem::StopMusic(lua_State* state)
{
    GetSelf(state)->audioRequests_.push_back(AudioRequest{
        AudioRequestType::StopMusic,
        {},
        false});
    return 0;
}

int LuaSystem::SpawnObject(lua_State* state)
{
    LuaSystem* self = GetSelf(state);
    std::size_t length = 0;
    const char* definitionPath = luaL_checklstring(state, 1, &length);
    if (length == 0)
    {
        return luaL_argerror(state, 1, "object definition path cannot be empty");
    }

    ObjectActionRequest request{
        ObjectActionType::Spawn,
        std::string(definitionPath, length)};

    if (!lua_isnoneornil(state, 2))
    {
        luaL_checktype(state, 2, LUA_TTABLE);
        const int overridesIndex = lua_absindex(state, 2);
        lua_pushnil(state);
        while (lua_next(state, overridesIndex) != 0)
        {
            if (lua_type(state, -2) != LUA_TSTRING)
            {
                return luaL_error(
                    state,
                    "SpawnObject override names must be strings");
            }

            const char* name = lua_tostring(state, -2);
            if (std::string_view(name) == "id")
            {
                std::size_t idLength = 0;
                const char* id = luaL_checklstring(state, -1, &idLength);
                if (idLength == 0)
                {
                    return luaL_error(
                        state,
                        "SpawnObject id override cannot be empty");
                }
                request.idOverride = std::string(id, idLength);
            }
            else if (std::string_view(name) == "x"
                || std::string_view(name) == "y")
            {
                const lua_Number value = luaL_checknumber(state, -1);
                if (!std::isfinite(value))
                {
                    return luaL_error(
                        state,
                        "SpawnObject position overrides must be finite");
                }
                if (std::string_view(name) == "x")
                {
                    request.xOverride = static_cast<float>(value);
                }
                else
                {
                    request.yOverride = static_cast<float>(value);
                }
            }
            else
            {
                return luaL_error(
                    state,
                    "Unknown SpawnObject override: %s",
                    name);
            }
            lua_pop(state, 1);
        }
    }

    // Lua requests scene mutation; WorldScene applies it only after its
    // current update/interaction traversal has finished.
    self->sceneActions_.push_back(std::move(request));
    return 0;
}

int LuaSystem::DestroyObject(lua_State* state)
{
    LuaSystem* self = GetSelf(state);
    std::size_t length = 0;
    const char* objectId = luaL_checklstring(state, 1, &length);
    if (length == 0)
    {
        return luaL_argerror(state, 1, "object ID cannot be empty");
    }

    self->sceneActions_.push_back(ObjectActionRequest{
        ObjectActionType::Destroy,
        std::string(objectId, length)});
    return 0;
}

int LuaSystem::FadeOutObject(lua_State* state)
{
    LuaSystem* self = GetSelf(state);
    std::size_t length = 0;
    const char* objectId = luaL_checklstring(state, 1, &length);
    if (length == 0)
    {
        return luaL_argerror(state, 1, "object ID cannot be empty");
    }

    const lua_Number duration = luaL_checknumber(state, 2);
    if (!std::isfinite(duration) || duration <= 0.0)
    {
        return luaL_argerror(
            state,
            2,
            "fade duration must be a positive finite number");
    }
    if (lua_isyieldable(state) == 0)
    {
        return luaL_error(
            state,
            "FadeOutObject can only be used by a coroutine");
    }

    LuaCoroutineHandle coroutineHandle = InvalidLuaCoroutineHandle;
    for (const auto& [handle, coroutine] : self->coroutines_)
    {
        if (coroutine.thread == state)
        {
            coroutineHandle = handle;
            break;
        }
    }
    if (coroutineHandle == InvalidLuaCoroutineHandle)
    {
        return luaL_error(
            state,
            "FadeOutObject could not identify its coroutine");
    }

    // WorldScene owns the visual transition. This coroutine resumes only when
    // that exact object's fade reaches zero opacity; object lifetime remains
    // under explicit Lua control after this call returns.
    self->sceneActions_.push_back(FadeObjectRequest{
        std::string(objectId, length),
        static_cast<float>(duration),
        coroutineHandle});
    lua_pushlstring(
        state,
        ObjectFadeTrigger.data(),
        ObjectFadeTrigger.size());
    return lua_yield(state, 1);
}

int LuaSystem::FadeOutWeather(lua_State* state)
{
    LuaSystem* self = GetSelf(state);
    const lua_Number duration = luaL_checknumber(state, 1);
    if (!std::isfinite(duration) || duration <= 0.0
        || duration > std::numeric_limits<float>::max()
        || static_cast<float>(duration) <= 0.0F)
    {
        return luaL_argerror(
            state, 1, "fade duration must be a positive finite float");
    }
    if (lua_isyieldable(state) == 0)
    {
        return luaL_error(state,
            "FadeOutWeather can only be used by a coroutine");
    }

    LuaCoroutineHandle coroutineHandle = InvalidLuaCoroutineHandle;
    for (const auto& [handle, coroutine] : self->coroutines_)
    {
        if (coroutine.thread == state)
        {
            coroutineHandle = handle;
            break;
        }
    }
    if (coroutineHandle == InvalidLuaCoroutineHandle)
    {
        return luaL_error(state,
            "FadeOutWeather could not identify its coroutine");
    }

    self->sceneActions_.push_back(FadeWeatherRequest{
        static_cast<float>(duration), coroutineHandle});
    lua_pushlstring(state, WeatherFadeTrigger.data(), WeatherFadeTrigger.size());
    return lua_yield(state, 1);
}

LuaCoroutineHandle LuaSystem::GetCallingCoroutine(lua_State* state)
{
    if (lua_isyieldable(state) == 0)
    {
        luaL_error(state, "This operation requires a coroutine");
    }
    for (const auto& [handle, coroutine] : GetSelf(state)->coroutines_)
    {
        if (coroutine.thread == state)
        {
            return handle;
        }
    }
    luaL_error(state, "Could not identify the calling coroutine");
    return InvalidLuaCoroutineHandle;
}

int LuaSystem::MoveObjectTo(lua_State* state)
{
    const char* objectId = ReadIdentifier(state, 1);
    const char* waypoint = ReadIdentifier(state, 2);
    const float speed = ReadPositiveFloat(state, 3);
    const auto handle = GetCallingCoroutine(state);
    GetSelf(state)->sceneActions_.push_back(
        MoveObjectRequest{objectId, waypoint, speed, handle});
    lua_pushliteral(state, "objectMove");
    return lua_yield(state, 1);
}

int LuaSystem::StartCutscene(lua_State* state)
{
    const char* scriptPath = ReadIdentifier(state, 1);
    GetSelf(state)->sceneActions_.push_back(StartCutsceneRequest{scriptPath});
    return 0;
}

int LuaSystem::SetObjectPosition(lua_State* state)
{
    const char* objectId = ReadIdentifier(state, 1);
    const char* waypoint = ReadIdentifier(state, 2);
    GetSelf(state)->sceneActions_.push_back(
        SetObjectPositionRequest{objectId, waypoint});
    return 0;
}

int LuaSystem::PlayAnimation(lua_State* state)
{
    const char* objectId = ReadIdentifier(state, 1);
    const char* animation = ReadIdentifier(state, 2);
    GetSelf(state)->sceneActions_.push_back(
        PlayAnimationRequest{objectId, animation});
    return 0;
}

int LuaSystem::FollowObject(lua_State* state)
{
    const char* objectId = ReadIdentifier(state, 1);
    const float x = ReadOptionalOffset(state, 2);
    const float y = ReadOptionalOffset(state, 3);
    GetSelf(state)->sceneActions_.push_back(FollowObjectRequest{objectId, x, y});
    return 0;
}

int LuaSystem::MoveCameraTo(lua_State* state)
{
    const char* waypoint = ReadIdentifier(state, 1);
    const float duration = ReadPositiveFloat(state, 2);
    const auto handle = GetCallingCoroutine(state);
    GetSelf(state)->sceneActions_.push_back(
        MoveCameraRequest{waypoint, duration, handle});
    lua_pushliteral(state, "cameraMove");
    return lua_yield(state, 1);
}

int LuaSystem::Wait(lua_State* state)
{
    const float duration = ReadPositiveFloat(state, 1);
    const auto handle = GetCallingCoroutine(state);
    GetSelf(state)->sceneActions_.push_back(WaitRequest{duration, handle});
    lua_pushliteral(state, "wait");
    return lua_yield(state, 1);
}

int LuaSystem::FadeOutScene(lua_State* state)
{
    const float duration = ReadPositiveFloat(state, 1);
    const auto handle = GetCallingCoroutine(state);
    GetSelf(state)->sceneActions_.push_back(FadeSceneRequest{duration, handle});
    lua_pushliteral(state, "sceneFade");
    return lua_yield(state, 1);
}

int LuaSystem::ChangeScene(lua_State* state)
{
    const char* destination = ReadIdentifier(state, 1);
    GetSelf(state)->sceneActions_.push_back(
        SceneChangeRequest{{}, destination, {}});
    return 0;
}

int LuaSystem::SetGroundBehavior(lua_State* state)
{
    LuaSystem* self = GetSelf(state);
    std::size_t regionIdLength = 0;
    const char* regionId = luaL_checklstring(state, 1, &regionIdLength);
    if (regionIdLength == 0)
    {
        return luaL_argerror(state, 1, "ground-region ID cannot be empty");
    }

    std::size_t behaviorNameLength = 0;
    const char* behaviorName = luaL_checklstring(
        state,
        2,
        &behaviorNameLength);
    const std::optional<GroundBehavior> behavior = ParseGroundBehavior(
        std::string_view(behaviorName, behaviorNameLength));
    if (!behavior.has_value())
    {
        return luaL_argerror(
            state,
            2,
            "ground behavior must be walkable, collision, warp, or trigger");
    }

    // Ground state is scene-owned. Lua records a validated request and the
    // WorldScene applies it after the current traversal, just like object
    // lifetime requests.
    self->sceneActions_.push_back(GroundRegionRequest{
        std::string(regionId, regionIdLength),
        *behavior});
    return 0;
}

int LuaSystem::SetGroundParams(lua_State* state)
{
    LuaSystem* self = GetSelf(state);
    std::size_t regionIdLength = 0;
    const char* regionId = luaL_checklstring(state, 1, &regionIdLength);
    if (regionIdLength == 0)
    {
        return luaL_argerror(state, 1, "ground-region ID cannot be empty");
    }
    luaL_checktype(state, 2, LUA_TTABLE);
    const int tableIndex = lua_absindex(state, 2);

    // Validate before constructing the C++ map. Lua argument errors use a
    // long jump, so no non-trivial C++ local should need destruction while an
    // error can still be raised.
    lua_pushnil(state);
    while (lua_next(state, tableIndex) != 0)
    {
        if (lua_type(state, -2) != LUA_TSTRING)
        {
            return luaL_argerror(
                state,
                2,
                "ground parameter names must be strings");
        }

        std::size_t nameLength = 0;
        lua_tolstring(state, -2, &nameLength);
        if (nameLength == 0)
        {
            return luaL_argerror(
                state,
                2,
                "ground parameter names cannot be empty");
        }

        const int valueType = lua_type(state, -1);
        if (valueType != LUA_TSTRING
            && valueType != LUA_TNUMBER
            && valueType != LUA_TBOOLEAN)
        {
            return luaL_argerror(
                state,
                2,
                "ground parameters must be strings, numbers, or booleans");
        }
        if (valueType == LUA_TNUMBER
            && !std::isfinite(lua_tonumber(state, -1)))
        {
            return luaL_argerror(
                state,
                2,
                "ground parameter numbers must be finite");
        }
        lua_pop(state, 1);
    }

    GroundParameterMap parameters;
    lua_pushnil(state);
    while (lua_next(state, tableIndex) != 0)
    {
        std::size_t nameLength = 0;
        const char* name = lua_tolstring(state, -2, &nameLength);
        GroundParameterValue value;
        switch (lua_type(state, -1))
        {
        case LUA_TSTRING:
        {
            std::size_t valueLength = 0;
            const char* text = lua_tolstring(state, -1, &valueLength);
            value = std::string(text, valueLength);
            break;
        }
        case LUA_TNUMBER:
            value = static_cast<double>(lua_tonumber(state, -1));
            break;
        case LUA_TBOOLEAN:
            value = lua_toboolean(state, -1) != 0;
            break;
        default:
            break;
        }

        parameters.emplace(
            std::string(name, nameLength),
            std::move(value));
        lua_pop(state, 1);
    }

    self->sceneActions_.push_back(GroundRegionRequest{
        std::string(regionId, regionIdLength),
        std::move(parameters)});
    return 0;
}

int LuaSystem::StartBattle(lua_State* state)
{
    LuaSystem* self = GetSelf(state);
    luaL_checktype(state, 1, LUA_TTABLE);
    const lua_Integer count = static_cast<lua_Integer>(lua_rawlen(state, 1));
    if (count <= 0)
    {
        return luaL_argerror(state, 1, "battle requires at least one enemy ID");
    }

    // Validate the complete Lua value before constructing a C++ request.
    // Lua argument errors long-jump, so no C++ object with a destructor may
    // be alive when one is raised.
    for (lua_Integer index = 1; index <= count; ++index)
    {
        lua_geti(state, 1, index);
        std::size_t length = 0;
        const char* id = lua_tolstring(state, -1, &length);
        if (id == nullptr || length == 0)
        {
            lua_pop(state, 1);
            return luaL_argerror(
                state,
                1,
                "battle enemy IDs must be non-empty strings");
        }

        for (lua_Integer previous = 1; previous < index; ++previous)
        {
            lua_geti(state, 1, previous);
            const bool duplicate = lua_compare(state, -1, -2, LUA_OPEQ) != 0;
            lua_pop(state, 1);
            if (duplicate)
            {
                lua_pop(state, 1);
                return luaL_argerror(
                    state,
                    1,
                    "battle enemy IDs must be unique");
            }
        }
        lua_pop(state, 1);
    }

    BattleRequest request;
    request.enemyIds.reserve(static_cast<std::size_t>(count));
    for (lua_Integer index = 1; index <= count; ++index)
    {
        lua_geti(state, 1, index);
        std::size_t length = 0;
        const char* id = lua_tolstring(state, -1, &length);
        request.enemyIds.emplace_back(id, length);
        lua_pop(state, 1);
    }
    self->battleRequests_.push_back(std::move(request));
    return 0;
}

int LuaSystem::DebugMessage(lua_State* state)
{
    const char* message = luaL_checkstring(state, 1);
    WriteDebugMessage(message);
    return 0;
}

int LuaSystem::Say(lua_State* state)
{
    LuaSystem* self = GetSelf(state);
    std::size_t messageLength = 0;
    const char* message = luaL_checklstring(state, 1, &messageLength);
    if (messageLength == 0)
    {
        return luaL_argerror(state, 1, "dialog text cannot be empty");
    }
    std::size_t portraitLength = 0;
    const char* portrait = luaL_optlstring(state, 2, "", &portraitLength);
    std::optional<std::string> exitSoundOverride;
    if (!lua_isnoneornil(state, 3))
    {
        std::size_t exitSoundLength = 0;
        const char* exitSound = luaL_checklstring(
            state,
            3,
            &exitSoundLength);
        if (exitSoundLength == 0)
        {
            return luaL_argerror(
                state,
                3,
                "dialog exit sound asset ID cannot be empty");
        }
        exitSoundOverride.emplace(exitSound, exitSoundLength);
    }
    if (lua_isyieldable(state) == 0)
    {
        return luaL_error(state, "Say can only be used by a coroutine");
    }

    LuaCoroutineHandle coroutineHandle = InvalidLuaCoroutineHandle;
    for (const auto& [handle, coroutine] : self->coroutines_)
    {
        if (coroutine.thread == state)
        {
            coroutineHandle = handle;
            break;
        }
    }
    if (coroutineHandle == InvalidLuaCoroutineHandle)
    {
        return luaL_error(state, "Say could not identify its coroutine");
    }

    lua_getglobal(state, "CurrentObjectId");
    std::size_t speakerLength = 0;
    const char* speaker = lua_tolstring(state, -1, &speakerLength);
    if (speaker == nullptr || speakerLength == 0)
    {
        lua_pop(state, 1);
        lua_getglobal(state, "CurrentItemId");
        speaker = lua_tolstring(state, -1, &speakerLength);
        if (speaker == nullptr || speakerLength == 0)
        {
            lua_pop(state, 1);
            lua_getglobal(state, "CurrentSceneId");
            speaker = lua_tolstring(state, -1, &speakerLength);
            if (speaker == nullptr || speakerLength == 0)
            {
                lua_pop(state, 1);
                return luaL_error(state,
                    "Say requires a current object, item, or scene");
            }
        }
    }

    std::string speakerName(speaker, speakerLength);
    std::string dialogText(message, messageLength);
    std::string portraitImageId(portrait, portraitLength);
    lua_pop(state, 1);

    if (!self->dialogueWindow_.Show(dialogText, portraitImageId))
    {
        return luaL_error(state, "Say could not open the dialog window");
    }

    // Say is the stable Lua-facing boundary. Lua requests a complete dialog
    // action and sleeps; C++ owns opening, typing, input, and presentation.
    self->dialogCoroutineHandle_ = coroutineHandle;
    self->dialogExitSoundOverride_ = std::move(exitSoundOverride);
    std::string output = speakerName;
    output += ": ";
    output += dialogText;
    WriteDebugMessage(output);
    lua_pushlstring(state, DialogTrigger.data(), DialogTrigger.size());
    return lua_yield(state, 1);
}

int LuaSystem::WaitForTrigger(lua_State* state)
{
    std::size_t length = 0;
    const char* trigger = luaL_checklstring(state, 1, &length);
    if (length == 0)
    {
        return luaL_argerror(state, 1, "trigger name cannot be empty");
    }
    if (lua_isyieldable(state) == 0)
    {
        return luaL_error(
            state,
            "WaitForTrigger can only be used by a coroutine");
    }

    // The yielded string is the entire C++/Lua contract: Lua sleeps until the
    // owning object reports this exact engine trigger.
    lua_pushlstring(state, trigger, length);
    return lua_yield(state, 1);
}

void LuaSystem::RegisterFunction(
    const char* name,
    int (*function)(lua_State*))
{
    lua_pushlightuserdata(state_, this);
    lua_pushcclosure(state_, function, 1);
    lua_setglobal(state_, name);
}

void LuaSystem::SetCurrentContext(
    std::string_view objectId,
    std::string_view itemId,
    std::string_view sceneId,
    std::string_view groundRegionId)
{
    const auto setStringOrNil = [this](
                                    const char* name,
                                    std::string_view value)
    {
        if (value.empty())
        {
            lua_pushnil(state_);
        }
        else
        {
            lua_pushlstring(state_, value.data(), value.size());
        }
        lua_setglobal(state_, name);
    };

    // Context globals are shared by every Lua thread. Restore all of them
    // immediately before execution or resumption rather than assuming another
    // object, scene, battle, or item script has not replaced them.
    setStringOrNil("CurrentObjectId", objectId);
    setStringOrNil("CurrentItemId", itemId);
    setStringOrNil("CurrentSceneId", sceneId);
    setStringOrNil("CurrentGroundRegionId", groundRegionId);
}

bool LuaSystem::RunCoroutine(LuaCoroutineHandle& handle)
{
    const auto coroutine = coroutines_.find(handle);
    if (coroutine == coroutines_.end())
    {
        handle = InvalidLuaCoroutineHandle;
        return false;
    }

    CoroutineState& runtime = coroutine->second;
    runtime.awaitedTrigger.clear();
    SetCurrentContext(
        runtime.objectId,
        runtime.itemId,
        runtime.sceneId,
        runtime.groundRegionId);

    int resultCount = 0;
    const int result = lua_resume(runtime.thread, state_, 0, &resultCount);
    if (result == LUA_YIELD)
    {
        if (resultCount != 1
            || lua_type(runtime.thread, -1) != LUA_TSTRING)
        {
            WriteDebugMessage(
                "Lua coroutine must yield one trigger name: "
                + runtime.objectId);
            if (resultCount > 0)
            {
                lua_pop(runtime.thread, resultCount);
            }
            ReleaseCoroutine(handle);
            return false;
        }

        std::size_t length = 0;
        const char* trigger = lua_tolstring(runtime.thread, -1, &length);
        if (length == 0)
        {
            WriteDebugMessage(
                "Lua coroutine yielded an empty trigger: "
                + runtime.objectId);
            lua_pop(runtime.thread, resultCount);
            ReleaseCoroutine(handle);
            return false;
        }
        runtime.awaitedTrigger.assign(trigger, length);
        lua_pop(runtime.thread, resultCount);
        return true;
    }

    if (result == LUA_OK)
    {
        if (resultCount > 0)
        {
            lua_pop(runtime.thread, resultCount);
        }
        ReleaseCoroutine(handle);
        return true;
    }

    const char* error = lua_tostring(runtime.thread, -1);
    const std::string& contextId = runtime.objectId.empty()
        ? runtime.itemId
        : runtime.objectId;
    WriteDebugMessage(
        "Lua coroutine failed for '" + contextId + "': "
        + (error != nullptr ? error : "unknown error"));
    if (lua_gettop(runtime.thread) > 0)
    {
        lua_pop(runtime.thread, 1);
    }
    ReleaseCoroutine(handle);
    SetPlayerInputEnabled(true);
    return false;
}

void LuaSystem::ReleaseCoroutine(LuaCoroutineHandle& handle) noexcept
{
    const LuaCoroutineHandle releasedHandle = handle;
    const auto coroutine = coroutines_.find(handle);
    if (coroutine != coroutines_.end())
    {
        if (state_ != nullptr)
        {
            luaL_unref(
                state_,
                LUA_REGISTRYINDEX,
                coroutine->second.registryReference);
        }
        coroutines_.erase(coroutine);
    }
    if (dialogCoroutineHandle_ == releasedHandle)
    {
        dialogCoroutineHandle_ = InvalidLuaCoroutineHandle;
    }
    handle = InvalidLuaCoroutineHandle;
}
