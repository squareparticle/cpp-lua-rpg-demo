#include "../src/Game/State/GlobalState.h"
#include "../src/Game/State/Inventory.h"
#include "../src/Game/Input/InputState.h"
#include "../src/Game/Scripting/RuleTable.h"
#include "../src/Game/Scripting/LuaSystem.h"
#include "../src/Game/UI/DialogueWindow.h"

#include <iostream>
#include <string>
#include <string_view>
#include <utility>

namespace
{
using MenuWindow = DialogueWindow;
using MenuWindowState = DialogueWindowState;

bool ExpectScript(
    const RuleTable& rules,
    const GlobalState& state,
    const std::string& expected)
{
    const ScriptRule* rule = rules.Resolve(state);
    if (rule == nullptr || rule->scriptPath != expected)
    {
        std::cerr << "Expected " << expected << '\n';
        return false;
    }
    return true;
}

bool Expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << message << '\n';
        return false;
    }
    return true;
}

bool PopObjectAction(LuaSystem& lua, ObjectActionRequest& request)
{
    SceneActionRequest sceneAction;
    if (!lua.PopSceneAction(sceneAction))
    {
        return false;
    }

    auto* objectAction = std::get_if<ObjectActionRequest>(&sceneAction);
    if (objectAction == nullptr)
    {
        return false;
    }
    request = std::move(*objectAction);
    return true;
}

bool PopGroundRegionRequest(LuaSystem& lua, GroundRegionRequest& request)
{
    SceneActionRequest sceneAction;
    if (!lua.PopSceneAction(sceneAction))
    {
        return false;
    }

    auto* groundRequest = std::get_if<GroundRegionRequest>(&sceneAction);
    if (groundRequest == nullptr)
    {
        return false;
    }
    request = std::move(*groundRequest);
    return true;
}

bool PopFadeObjectRequest(LuaSystem& lua, FadeObjectRequest& request)
{
    SceneActionRequest sceneAction;
    if (!lua.PopSceneAction(sceneAction))
    {
        return false;
    }

    auto* fadeRequest = std::get_if<FadeObjectRequest>(&sceneAction);
    if (fadeRequest == nullptr)
    {
        return false;
    }
    request = std::move(*fadeRequest);
    return true;
}

bool HasNoSceneActions(LuaSystem& lua)
{
    SceneActionRequest request;
    return !lua.PopSceneAction(request);
}

bool AdvanceDialogPage(MenuWindow& menu)
{
    if (menu.GetState() == MenuWindowState::Opening)
    {
        menu.Update(1.0F, {});
    }

    InputState confirm;
    confirm.primaryActionPressed = true;
    if (menu.GetState() == MenuWindowState::Typing)
    {
        menu.Update(0.0F, confirm);
    }
    if (!Expect(menu.GetState() == MenuWindowState::Waiting,
            "Dialog page did not reach its continue prompt"))
    {
        return false;
    }

    menu.Update(0.0F, confirm);
    return Expect(menu.GetState() == MenuWindowState::Advancing,
            "Dialog page closed before Lua resumed")
        && Expect(menu.ConsumeAdvanceRequest(),
            "Dialog page did not request an advance");
}

bool TestPlayerInputControl()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    LuaCoroutineHandle cutscene = InvalidLuaCoroutineHandle;
    if (!lua.Initialize()
        || !Expect(lua.IsPlayerInputEnabled(),
            "Player input did not default to enabled")
        || !lua.StartCoroutine("tests/player_input.lua", "crew1", cutscene)
        || !Expect(!lua.IsPlayerInputEnabled(),
            "Lua assignment did not disable player input")
        || !Expect(lua.IsCoroutineWaitingFor(cutscene, "dialog"),
            "Disabled player input prevented cutscene dialogue")
        || !AdvanceDialogPage(menu)
        || !lua.CompleteDialog()
        || !Expect(lua.IsPlayerInputEnabled(),
            "Resumed Lua did not re-enable player input")
        || !Expect(!lua.IsCoroutineActive(cutscene),
            "Cutscene did not finish after dialogue"))
    {
        return false;
    }

    if (!lua.StartCoroutine("tests/player_input.lua", "crew1", cutscene)
        || !Expect(!lua.IsPlayerInputEnabled(),
            "Second cutscene did not disable player input"))
    {
        return false;
    }
    lua.StopCoroutine(cutscene);
    lua.ClearSceneActions();
    return Expect(lua.IsPlayerInputEnabled(),
        "Scene teardown left player input disabled");
}

bool TestSceneCutsceneCoroutine()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    SceneActionRequest request;
    LuaCoroutineHandle handle = InvalidLuaCoroutineHandle;
    if (!lua.Initialize()
        || !lua.ExecuteSceneFile("tests/start_scene_cutscene.lua", "boatScene")
        || !Expect(lua.PopSceneAction(request)
                && std::holds_alternative<StartCutsceneRequest>(request)
                && std::get<StartCutsceneRequest>(request).scriptPath
                    == "tests/scene_cutscene.lua",
            "StartCutscene did not queue its script path")
        || !Expect(lua.IsPlayerInputEnabled() && !menu.IsModalActive(),
            "StartCutscene ran inside the requesting script")
        || !lua.StartSceneCoroutine("tests/scene_cutscene.lua", "boatScene", handle)
        || !Expect(!lua.IsPlayerInputEnabled()
                && lua.IsCoroutineWaitingFor(handle, "dialog"),
            "Scene cutscene could not disable control and show dialogue")
        || !AdvanceDialogPage(menu) || !lua.CompleteDialog()
        || !Expect(lua.IsPlayerInputEnabled() && !lua.IsCoroutineActive(handle),
            "Scene cutscene did not finish after dialogue")
        || !lua.StartSceneCoroutine("tests/scene_cutscene.lua", "boatScene", handle))
    {
        return false;
    }
    lua.StopCoroutine(handle);
    return Expect(!menu.IsModalActive() && handle == InvalidLuaCoroutineHandle,
        "Stopping a scene cutscene left its dialogue open");
}

bool TestCutsceneArguments()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    return lua.Initialize()
        && lua.ExecuteFile("tests/cutscene_arguments.lua", "crew3")
        && Expect(state.GetTag("test_cutscene_arguments_checked"),
            "Cutscene argument validation did not finish")
        && Expect(HasNoSceneActions(lua),
            "Rejected cutscene commands queued scene actions");
}

bool TestWeatherFadeLuaBinding()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    LuaCoroutineHandle handle = InvalidLuaCoroutineHandle;
    SceneActionRequest request;
    if (!lua.Initialize()
        || !Expect(!lua.ExecuteFile("tests/weather_fade.lua", "crew1"),
            "Weather fade accepted a non-coroutine caller")
        || !Expect(HasNoSceneActions(lua),
            "Invalid weather fade queued a scene action")
        || !lua.StartCoroutine("tests/weather_fade.lua", "crew1", handle)
        || !Expect(lua.PopSceneAction(request)
                && std::holds_alternative<FadeWeatherRequest>(request),
            "Weather fade did not queue a typed request")
        || !Expect(std::get<FadeWeatherRequest>(request).durationSeconds == 0.25F
                && std::get<FadeWeatherRequest>(request).coroutineHandle == handle,
            "Weather fade lost its duration or caller")
        || !Expect(HasNoSceneActions(lua),
            "Invalid durations queued weather fades")
        || !Expect(!state.GetTag("test_weather_fade_complete"),
            "Weather fade did not suspend its caller")
        || !lua.ResumeCoroutine(handle, "weatherFade"))
    {
        return false;
    }
    return Expect(state.GetTag("test_weather_fade_complete")
            && !lua.IsCoroutineActive(handle),
        "Weather fade completion did not finish its caller");
}

bool RunDialogScript(
    LuaSystem& lua,
    MenuWindow& menu,
    const std::string& filename,
    const std::string& objectId,
    std::size_t expectedPages,
    std::string_view expectedExitSound = {})
{
    LuaCoroutineHandle interaction = InvalidLuaCoroutineHandle;
    if (!Expect(lua.StartCoroutine(filename, objectId, interaction),
            "Dialog coroutine did not start")
        || !Expect(menu.IsModalActive(), "Say did not open a modal window")
        || !Expect(lua.IsCoroutineWaitingFor(interaction, "dialog"),
            "Say did not yield for dialog completion"))
    {
        return false;
    }

    std::size_t pages = 0;
    while (lua.IsCoroutineActive(interaction))
    {
        if (++pages > 20
            || !AdvanceDialogPage(menu)
            || !Expect(lua.CompleteDialog(),
                "Dialog completion did not resume Lua"))
        {
            return false;
        }
    }

    const std::optional<std::string> exitSound =
        lua.TakeDialogExitSoundOverride();
    const bool hasExpectedExitSound = expectedExitSound.empty()
        ? !exitSound.has_value()
        : exitSound.has_value() && *exitSound == expectedExitSound;
    return Expect(pages == expectedPages,
            "Dialog script produced an unexpected page count")
        && Expect(!menu.IsModalActive(),
            "Dialog window stayed open after the script finished")
        && Expect(hasExpectedExitSound,
            "Dialog script produced the wrong exit sound override");
}

bool TestConsecutiveDialogPages()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    if (!lua.Initialize())
    {
        return false;
    }

    LuaCoroutineHandle interaction = InvalidLuaCoroutineHandle;
    if (!lua.StartCoroutine(
            "tests/dialog_sequence.lua",
            "crew1",
            interaction)
        || !AdvanceDialogPage(menu)
        || !lua.CompleteDialog())
    {
        return false;
    }

    if (!Expect(state.GetTag("test_dialog_first_complete"),
            "Lua did not continue after the first page")
        || !Expect(!state.GetTag("test_dialog_second_complete"),
            "Lua ran past the second Say")
        || !Expect(menu.IsModalActive(),
            "Window closed between consecutive Say calls")
        || !Expect(menu.GetState() == MenuWindowState::Typing,
            "Consecutive Say replayed the opening animation")
        || !Expect(lua.IsCoroutineWaitingFor(interaction, "dialog"),
            "Second Say did not yield the same interaction coroutine"))
    {
        return false;
    }

    if (!AdvanceDialogPage(menu) || !lua.CompleteDialog())
    {
        return false;
    }
    const std::optional<std::string> exitSound =
        lua.TakeDialogExitSoundOverride();
    return Expect(exitSound.has_value()
            && *exitSound == "customDialogExit",
            "Final page did not expose its exit sound override")
        && Expect(state.GetTag("test_dialog_second_complete"),
            "Lua did not finish after the second page")
        && Expect(!lua.IsCoroutineActive(interaction),
            "Consecutive dialog coroutine remained active")
        && Expect(!menu.IsModalActive(),
            "Window did not close after the final page");
}

bool TestDutiesCoroutine()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    if (!lua.Initialize())
    {
        return false;
    }

    LuaCoroutineHandle duties = InvalidLuaCoroutineHandle;
    if (!Expect(
            lua.StartCoroutine(
                "tests/duties_coroutine.lua",
                "crew1",
                duties),
            "Duties coroutine did not start")
        || !Expect(state.GetTag("test_duties_started"),
            "Duties did not execute before its first wait")
        || !Expect(lua.IsCoroutineWaitingFor(duties, "mop_right"),
            "Duties did not yield for mop_right"))
    {
        return false;
    }

    if (!Expect(lua.ResumeCoroutine(duties, "unrelated"),
            "Unrelated trigger reported an error")
        || !Expect(!state.GetTag("test_duties_reached_right"),
            "Unrelated trigger resumed duties")
        || !Expect(lua.IsCoroutineWaitingFor(duties, "mop_right"),
            "Unrelated trigger changed the awaited trigger"))
    {
        return false;
    }

    if (!Expect(lua.ResumeCoroutine(duties, "mop_right"),
            "mop_right failed to resume duties")
        || !Expect(state.GetTag("test_duties_reached_right"),
            "Duties did not run after mop_right")
        || !Expect(!state.GetTag("test_duties_reached_left"),
            "Duties ran past its second wait")
        || !Expect(lua.IsCoroutineWaitingFor(duties, "mop_left"),
            "Duties did not yield for mop_left"))
    {
        return false;
    }

    if (!Expect(lua.ResumeCoroutine(duties, "mop_left"),
            "mop_left failed to resume duties")
        || !Expect(state.GetTag("test_duties_reached_left"),
            "Duties did not finish after mop_left")
        || !Expect(duties == InvalidLuaCoroutineHandle,
            "Completed duties retained a handle"))
    {
        return false;
    }

    LuaCoroutineHandle unfinished = InvalidLuaCoroutineHandle;
    if (!lua.StartCoroutine(
            "tests/duties_coroutine.lua",
            "crew1",
            unfinished))
    {
        return false;
    }

    // InteractableObject::Shutdown uses this same stop operation before firing
    // DESTROY. Stopping must release the thread without closing the LuaSystem.
    lua.StopCoroutine(unfinished);
    return Expect(unfinished == InvalidLuaCoroutineHandle,
            "Stopped duties retained a handle")
        && Expect(RunDialogScript(
                lua,
                menu,
                "assets/scenes/boatscene/characters/crew1/dialog_start_mission.lua",
                "crew1",
                3),
            "LuaSystem was not usable after duties stopped")
        && Expect(state.GetTag("mission_storageLantern"),
            "Lua did not execute after duties stopped");
}

bool TestInventoryLuaBindings()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    if (!lua.Initialize())
    {
        return false;
    }

    return Expect(!inventory.AddItem(""),
            "Inventory accepted an empty item ID")
        && Expect(lua.ExecuteFile(
                "tests/inventory_api.lua",
                "inventory_test"),
            "Lua inventory API script failed")
        && Expect(!inventory.HasItem("storage_door_key"),
            "Lua inventory test did not remove its item")
        && Expect(inventory.AddItem("storage_door_key"),
            "C++ could not add an inventory item")
        && Expect(inventory.HasItem("storage_door_key"),
            "C++ inventory did not retain its item")
        && Expect(inventory.RemoveItem("storage_door_key"),
            "C++ could not remove an inventory item")
        && Expect(!inventory.RemoveItem("storage_door_key"),
            "Inventory removed the same unique item twice");
}

bool TestObjectActionLuaBindings()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    if (!lua.Initialize()
        || !Expect(lua.ExecuteFile(
                "tests/object_action_api.lua",
                "action_test"),
            "Lua object-action API script failed"))
    {
        return false;
    }

    ObjectActionRequest request;
    if (!Expect(PopObjectAction(lua, request),
            "SpawnObject did not queue an action")
        || !Expect(request.type == ObjectActionType::Spawn,
            "First object action was not spawn")
        || !Expect(
            request.value == "assets/scenes/boatscene/objects/key/storage_door_key.obj.yaml",
            "SpawnObject queued the wrong definition path")
        || !Expect(!request.idOverride.has_value()
                && !request.xOverride.has_value()
                && !request.yOverride.has_value(),
            "SpawnObject without overrides queued unexpected values")
        || !Expect(PopObjectAction(lua, request),
            "SpawnObject with overrides did not queue an action")
        || !Expect(request.type == ObjectActionType::Spawn
                && request.value
                    == "assets/scenes/boatscene/objects/flames/flame.obj.yaml",
            "Overridden SpawnObject queued the wrong definition")
        || !Expect(request.idOverride == "overrideObj"
                && request.xOverride == 12.5F
                && !request.yOverride.has_value(),
            "SpawnObject did not preserve its optional overrides")
        || !Expect(PopObjectAction(lua, request),
            "DestroyObject did not queue an action")
        || !Expect(request.type == ObjectActionType::Destroy,
            "Second object action was not destroy")
        || !Expect(request.value == "action_test",
            "DestroyObject queued the wrong object ID")
        || !Expect(HasNoSceneActions(lua),
            "Invalid object actions reached the queue"))
    {
        return false;
    }

    if (!Expect(lua.ExecuteFile(
            "tests/object_action_api.lua",
            "clear_test"),
        "Lua object-action clear setup failed"))
    {
        return false;
    }
    lua.ClearSceneActions();
    return Expect(HasNoSceneActions(lua),
        "ClearSceneActions left a scene request queued");
}

bool TestGroundRegionLuaBindings()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    if (!lua.Initialize()
        || !Expect(lua.ExecuteFile(
                "tests/ground_region_api.lua",
                "ground_test"),
            "Lua ground-region API script failed"))
    {
        return false;
    }

    GroundRegionRequest request;
    if (!Expect(PopGroundRegionRequest(lua, request),
            "First SetGroundBehavior did not queue a request")
        || !Expect(request.regionId == "storageDoor",
            "SetGroundBehavior queued the wrong region ID")
        || !Expect(std::get_if<GroundBehavior>(&request.change) != nullptr
                && *std::get_if<GroundBehavior>(&request.change)
                    == GroundBehavior::Walkable,
            "SetGroundBehavior queued the wrong behavior")
        || !Expect(PopGroundRegionRequest(lua, request),
            "SetGroundParams did not queue a request"))
    {
        return false;
    }

    const auto* parameters = std::get_if<GroundParameterMap>(&request.change);
    if (!Expect(request.regionId == "storageDoor" && parameters != nullptr,
            "SetGroundParams queued the wrong request type")
        || !Expect(parameters->size() == 4,
            "SetGroundParams queued the wrong parameter count"))
    {
        return false;
    }

    const auto destination = parameters->find("destination");
    const auto transitionSeconds = parameters->find("transitionSeconds");
    const auto fade = parameters->find("fade");
    if (!Expect(destination != parameters->end()
                && std::get_if<std::string>(&destination->second) != nullptr
                && *std::get_if<std::string>(&destination->second)
                    == "storageRoom",
            "SetGroundParams did not preserve its string value")
        || !Expect(transitionSeconds != parameters->end()
                && std::get_if<double>(&transitionSeconds->second) != nullptr
                && *std::get_if<double>(&transitionSeconds->second) == 0.25,
            "SetGroundParams did not preserve its numeric value")
        || !Expect(fade != parameters->end()
                && std::get_if<bool>(&fade->second) != nullptr
                && *std::get_if<bool>(&fade->second),
            "SetGroundParams did not preserve its Boolean value")
        || !Expect(PopGroundRegionRequest(lua, request),
            "Second SetGroundBehavior did not queue a request")
        || !Expect(std::get_if<GroundBehavior>(&request.change) != nullptr
                && *std::get_if<GroundBehavior>(&request.change)
                    == GroundBehavior::Warp,
            "Ground-region requests did not preserve Lua call order")
        || !Expect(HasNoSceneActions(lua),
            "Invalid ground-region calls reached the queue"))
    {
        return false;
    }

    if (!Expect(lua.ExecuteFile(
            "tests/ground_region_api.lua",
            "clear_test"),
        "Lua ground-region clear setup failed"))
    {
        return false;
    }
    lua.ClearSceneActions();
    return Expect(HasNoSceneActions(lua),
        "ClearSceneActions left a ground-region request queued");
}

bool TestSceneInitLuaContext()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    if (!lua.Initialize()
        || !Expect(lua.ExecuteFile(
                "tests/scene_object_context.lua",
                "scene_object_context"),
            "Object-context setup script failed")
        || !Expect(lua.ExecuteSceneFile(
                "tests/scene_init.lua",
                "testScene"),
            "Scene init script failed")
        || !Expect(state.GetTag("scene_init_ran"),
            "Scene init script did not update persistent state"))
    {
        return false;
    }

    SceneActionRequest action;
    if (!Expect(lua.PopSceneAction(action),
            "Scene init did not queue its first action"))
    {
        return false;
    }
    const auto* behavior = std::get_if<GroundRegionRequest>(&action);
    if (!Expect(behavior != nullptr
                && std::get_if<GroundBehavior>(&behavior->change) != nullptr,
            "Scene init reordered its first ground action")
        || !Expect(lua.PopSceneAction(action),
            "Scene init did not queue its object action")
        || !Expect(std::get_if<ObjectActionRequest>(&action) != nullptr,
            "Scene init reordered its object action")
        || !Expect(lua.PopSceneAction(action),
            "Scene init did not queue its final ground action"))
    {
        return false;
    }

    const auto* parameters = std::get_if<GroundRegionRequest>(&action);
    return Expect(parameters != nullptr
            && std::get_if<GroundParameterMap>(&parameters->change) != nullptr,
        "Scene init reordered its parameter action")
        && Expect(HasNoSceneActions(lua),
            "Scene init queued an unexpected extra action");
}

bool TestBoatSceneReconciliation()
{
    constexpr const char* InitScript =
        "assets/scenes/boatscene/init.lua";

    {
        GlobalState state;
        Inventory inventory;
        MenuWindow menu;
        LuaSystem lua(state, inventory, menu);
        SceneActionRequest request;
        if (!Expect(lua.Initialize()
                && lua.ExecuteSceneFile(InitScript, "boatScene"),
                "Fresh boat-scene init failed")
            || !Expect(lua.PopSceneAction(request)
                    && std::holds_alternative<StartCutsceneRequest>(request)
                    && std::get<StartCutsceneRequest>(request).scriptPath
                        == "assets/scenes/boatscene/opening.lua",
                "Fresh boat scene did not request its opening cutscene")
            || !Expect(HasNoSceneActions(lua),
                "Fresh boat scene queued unexpected extra actions"))
        {
            return false;
        }
    }

    {
        GlobalState state;
        state.SetTag("mission_storageLantern", true);
        Inventory inventory;
        MenuWindow menu;
        LuaSystem lua(state, inventory, menu);
        ObjectActionRequest request;
        if (!Expect(lua.Initialize()
                && lua.ExecuteSceneFile(InitScript, "boatScene"),
                "Active-mission boat-scene init failed")
            || !Expect(PopObjectAction(lua, request),
                "Active mission did not reconstruct the storage key")
            || !Expect(request.type == ObjectActionType::Spawn
                    && request.value
                        == "assets/scenes/boatscene/objects/key/storage_door_key.obj.yaml",
                "Active mission reconstructed the wrong object")
            || !Expect(HasNoSceneActions(lua),
                "Active mission queued unexpected reconstruction work"))
        {
            return false;
        }
    }

    {
        GlobalState state;
        state.SetTag("storageLantern_has_lantern", true);
        Inventory inventory;
        MenuWindow menu;
        LuaSystem lua(state, inventory, menu);
        ObjectActionRequest request;
        AudioRequest audioRequest;
        if (!Expect(lua.Initialize()
                && lua.ExecuteSceneFile(InitScript, "boatScene"),
                "Lantern-collected boat-scene init failed")
            || !Expect(lua.PopAudioRequest(audioRequest)
                    && audioRequest.type == AudioRequestType::PlaySound
                    && audioRequest.assetId == "boatFire",
                "Lantern-collected scene did not request the fire sound")
            || !Expect(!lua.PopAudioRequest(audioRequest),
                "Lantern-collected scene queued an unexpected extra sound")
            || !Expect(PopObjectAction(lua, request),
                "Lantern-collected scene did not remove the water")
            || !Expect(request.type == ObjectActionType::Destroy
                    && request.value == "waterObj",
                "Lantern-collected scene removed the wrong object")
            || !Expect(PopObjectAction(lua, request),
                "Lantern-collected scene did not spawn the lava")
            || !Expect(request.type == ObjectActionType::Spawn
                    && request.value
                        == "assets/scenes/boatscene/objects/water/lava.obj.yaml",
                "Lantern-collected scene spawned the wrong surface")
            || !Expect(!state.GetTag("storageLantern_flames_active"),
                "Missing flames tag did not remain false")
            || !Expect(HasNoSceneActions(lua),
                "Lantern-collected scene queued unexpected reconstruction work"))
        {
            return false;
        }
    }

    {
        GlobalState state;
        state.SetTag("storageLantern_has_lantern", true);
        state.SetTag("storageLantern_fairies_released", true);
        Inventory inventory;
        MenuWindow menu;
        LuaSystem lua(state, inventory, menu);
        if (!Expect(lua.Initialize()
                && lua.ExecuteSceneFile(InitScript, "boatScene"),
                "Released-fairies boat-scene init failed")
            || !Expect(HasNoSceneActions(lua),
                "Released fairies did not preserve the authored water"))
        {
            return false;
        }
    }

    {
        GlobalState state;
        state.SetTag("storageLantern_flames_active", true);
        Inventory inventory;
        MenuWindow menu;
        LuaSystem lua(state, inventory, menu);
        ObjectActionRequest request;
        struct ExpectedFlame
        {
            const char* id;
            float x;
            float y;
        };
        constexpr ExpectedFlame Flames[] = {
            {"flame1Obj", 570.0F, 30.0F},
            {"flame2Obj", 775.0F, 310.0F},
            {"flame3Obj", 500.0F, 625.0F},
            {"flame4Obj", 745.0F, 690.0F}};
        if (!Expect(lua.Initialize()
                && lua.ExecuteSceneFile(InitScript, "boatScene"),
                "Active-flames boat-scene init failed"))
        {
            return false;
        }
        for (const ExpectedFlame& expected : Flames)
        {
            if (!Expect(PopObjectAction(lua, request),
                    "Active flames did not reconstruct every flame")
                || !Expect(request.type == ObjectActionType::Spawn
                        && request.value
                            == "assets/scenes/boatscene/objects/flames/flame.obj.yaml",
                    "Active flames reconstructed the wrong template")
                || !Expect(request.idOverride == expected.id
                        && request.xOverride == expected.x
                        && request.yOverride == expected.y,
                    "Active flames reconstructed the wrong overrides"))
            {
                return false;
            }
        }
        if (!Expect(HasNoSceneActions(lua),
                "Active flames queued unexpected reconstruction work"))
        {
            return false;
        }
    }

    {
        GlobalState state;
        state.SetTag("mission_storageLantern", true);
        state.SetTag("storageLantern_has_doorKey", true);
        Inventory inventory;
        MenuWindow menu;
        LuaSystem lua(state, inventory, menu);
        if (!Expect(lua.Initialize()
                && lua.ExecuteSceneFile(InitScript, "boatScene"),
                "Collected-key boat-scene init failed")
            || !Expect(HasNoSceneActions(lua),
                "Collected storage key was incorrectly respawned"))
        {
            return false;
        }
    }

    {
        GlobalState state;
        state.SetTag("mission_storageLantern", true);
        state.SetTag("storageLantern_door_open", true);
        Inventory inventory;
        MenuWindow menu;
        LuaSystem lua(state, inventory, menu);
        GroundRegionRequest request;
        if (!Expect(lua.Initialize()
                && lua.ExecuteSceneFile(InitScript, "boatScene"),
                "Open-door boat-scene init failed")
            || !Expect(PopGroundRegionRequest(lua, request),
                "Open door did not restore its ground behavior"))
        {
            return false;
        }

        const auto* behavior = std::get_if<GroundBehavior>(&request.change);
        if (!Expect(request.regionId == "storageDoor"
                    && behavior != nullptr
                    && *behavior == GroundBehavior::Warp,
                "Open door restored the wrong ground behavior"))
        {
            return false;
        }

        ObjectActionRequest objectRequest;
        if (!Expect(PopObjectAction(lua, objectRequest),
                "Open door did not remove its authored object")
            || !Expect(objectRequest.type == ObjectActionType::Destroy
                    && objectRequest.value == "storageDoorObj",
                "Open door removed the wrong authored object")
            || !Expect(HasNoSceneActions(lua),
                "Open door queued unexpected reconstruction work"))
        {
            return false;
        }
    }

    return true;
}

bool TestStorageDoorKeyPickup()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    if (!lua.Initialize()
        || !RunDialogScript(
            lua,
            menu,
            "assets/scenes/boatscene/objects/key/pickup.lua",
            "storageDoorKeyObj",
            1,
            "keyPickup"))
    {
        return false;
    }

    ObjectActionRequest request;
    return Expect(inventory.HasItem("storage_door_key"),
            "Key pickup did not add the inventory item")
        && Expect(state.GetTag("storageLantern_has_doorKey"),
            "Key pickup did not set its persistent tag")
        && Expect(PopObjectAction(lua, request),
            "Key pickup did not request object destruction")
        && Expect(request.type == ObjectActionType::Destroy,
            "Key pickup queued the wrong object action")
        && Expect(request.value == "storageDoorKeyObj",
            "Key pickup requested destruction of the wrong object")
        && Expect(HasNoSceneActions(lua),
            "Key pickup queued an unexpected extra object action");
}

bool TestGuidingLanternPickup()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    if (!lua.Initialize()
        || !RunDialogScript(
            lua,
            menu,
            "assets/scenes/storageroom/objects/lantern/pickup.lua",
            "guidingLanternObj",
            1,
            "lanternPickup"))
    {
        return false;
    }

    ObjectActionRequest request;
    if (!Expect(inventory.HasItem("guiding_lantern"),
            "Lantern pickup did not add the inventory item")
        || !Expect(state.GetTag("storageLantern_has_lantern"),
            "Lantern pickup did not set its persistent tag")
        || !Expect(state.GetTag("storageLantern_flames_active"),
            "Lantern pickup did not activate the separate flames tag")
        || !Expect(PopObjectAction(lua, request),
            "Lantern pickup did not request object destruction")
        || !Expect(request.type == ObjectActionType::Destroy,
            "Lantern pickup queued the wrong object action")
        || !Expect(request.value == "guidingLanternObj",
            "Lantern pickup requested destruction of the wrong object")
        || !Expect(HasNoSceneActions(lua),
            "Lantern pickup queued an unexpected extra object action"))
    {
        return false;
    }

    GlobalState restoredState;
    restoredState.SetTag("storageLantern_has_lantern", true);
    Inventory restoredInventory;
    MenuWindow restoredMenu;
    LuaSystem restoredLua(restoredState, restoredInventory, restoredMenu);
    if (!Expect(restoredLua.Initialize()
            && restoredLua.ExecuteSceneFile(
                "assets/scenes/storageroom/init.lua",
                "storageRoom"),
            "Collected-lantern storage-scene init failed")
        || !Expect(PopObjectAction(restoredLua, request),
            "Collected lantern was not removed during scene reconstruction"))
    {
        return false;
    }

    return Expect(request.type == ObjectActionType::Destroy
            && request.value == "guidingLanternObj",
            "Storage init removed the wrong reconstructed object")
        && Expect(HasNoSceneActions(restoredLua),
            "Collected lantern queued unexpected reconstruction work");
}

bool TestGuidingLanternUseContext()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    if (!lua.Initialize())
    {
        return false;
    }

    LuaCoroutineHandle use = InvalidLuaCoroutineHandle;
    if (!Expect(lua.StartItemCoroutine(
            "assets/items/guiding_lantern.lua",
            "guiding_lantern",
            "storageRoom",
            {},
            use),
            "Lantern use did not start outside the release region")
        || !Expect(
            menu.GetCharacterCount()
                == std::string_view("Nothing happened...").size(),
            "Lantern use chose the wrong outside-region dialog")
        || !AdvanceDialogPage(menu)
        || !lua.CompleteDialog())
    {
        return false;
    }

    if (!Expect(lua.StartItemCoroutine(
            "assets/items/guiding_lantern.lua",
            "guiding_lantern",
            "boatScene",
            "lanternRelease",
            use),
            "Lantern use did not start in the release region")
        || !Expect(
            menu.GetCharacterCount()
                == std::string_view("Fairies released").size(),
            "Lantern use chose the wrong release-region dialog")
        || !Expect(lua.IsCoroutineWaitingFor(use, "dialog"),
            "Lantern use did not wait for its release dialog")
        || !Expect(!state.GetTag("storageLantern_fairies_released"),
            "Lantern use changed state before its dialog closed")
        || !Expect(HasNoSceneActions(lua),
            "Lantern use changed the water before its dialog closed")
        || !AdvanceDialogPage(menu)
        || !lua.CompleteDialog()
        || !Expect(!menu.IsModalActive(),
            "Lantern release dialog stayed open during the fade")
        || !Expect(state.GetTag("storageLantern_fairies_released"),
            "Closing the dialog did not persist the released-fairies state")
        || !Expect(state.GetTag("weather=fairies"),
            "Closing the dialog did not select fairy weather")
        || !Expect(lua.TakeDialogExitSoundOverride()
                == std::optional<std::string>("lanternRelease"),
            "Lantern release did not replace the dialog exit sound")
        || !Expect(lua.IsCoroutineWaitingFor(use, "objectFade"),
            "Closing the dialog did not begin waiting for the lava fade")
        || !Expect(!lua.IsPlayerInputEnabled(),
            "Lantern release left player controls enabled during the lava fade"))
    {
        return false;
    }


    ObjectActionRequest objectRequest;
    FadeObjectRequest fadeRequest;
    SceneActionRequest weatherAction;
    if (!Expect(PopObjectAction(lua, objectRequest),
            "Lantern use did not restore water beneath the lava")
        || !Expect(objectRequest.type == ObjectActionType::Spawn
                && objectRequest.value
                    == "assets/scenes/boatscene/objects/water/water.obj.yaml",
            "Lantern use restored the wrong surface")
        || !Expect(PopFadeObjectRequest(lua, fadeRequest),
            "Lantern use did not request a lava fade")
        || !Expect(fadeRequest.objectId == "lavaObj"
                && fadeRequest.durationSeconds == 2.0F
                && fadeRequest.coroutineHandle == use,
            "Lantern use requested the wrong fade")
        || !Expect(HasNoSceneActions(lua),
            "FadeOutObject destroyed the lava before its fade completed")
        || !Expect(lua.ResumeCoroutine(use, "objectFade"),
            "Completed lava fade did not resume the item script")
        || !Expect(PopObjectAction(lua, objectRequest),
            "Resumed lantern script did not destroy the lava")
        || !Expect(objectRequest.type == ObjectActionType::Destroy
                && objectRequest.value == "lavaObj",
            "Resumed lantern script destroyed the wrong object")
        || !Expect(lua.IsCoroutineWaitingFor(use, "weatherFade"),
            "Lantern item did not wait for the weather fade")
        || !Expect(!lua.IsPlayerInputEnabled(),
            "Lantern item restored controls before the fairy fade finished")
        || !Expect(lua.PopSceneAction(weatherAction)
                && std::holds_alternative<FadeWeatherRequest>(weatherAction),
            "Lantern item did not request a scene-owned weather fade")
        || !Expect(std::get<FadeWeatherRequest>(weatherAction).durationSeconds == 0.5F
                && std::get<FadeWeatherRequest>(weatherAction).coroutineHandle == use,
            "Lantern item requested the wrong weather fade")
        || !Expect(state.GetTag("weather=fairies"),
            "Weather selection changed before the scene handled its fade")
        || !Expect(HasNoSceneActions(lua),
            "Lantern item queued extra scene actions during its weather fade")
        || !Expect(lua.ResumeCoroutine(use, "objectFade")
                && lua.IsCoroutineWaitingFor(use, "weatherFade"),
            "An unrelated fade resumed the weather waiter")
        || !Expect(!lua.IsPlayerInputEnabled(),
            "Unrelated fade restored player controls")
        || !Expect(lua.ResumeCoroutine(use, "weatherFade"),
            "Fairy fade completion did not resume the lantern script")
        || !Expect(lua.IsPlayerInputEnabled(),
            "Lantern item did not restore controls after the fairy fade")
        || !Expect(!lua.IsCoroutineActive(use),
            "Lantern item script stayed active after the fairy fade"))
    {
        return false;
    }

    return Expect(!menu.IsModalActive(),
        "Lantern item dialog stayed open after its script finished");
}

bool TestStorageDoorInteraction()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    AudioRequest audioRequest;
    if (!lua.Initialize()
        || !RunDialogScript(
            lua,
            menu,
            "assets/scenes/boatscene/objects/door/interact.lua",
            "storageDoorObj",
            1)
        || !Expect(!state.GetTag("storageLantern_door_open"),
            "Locked door changed persistent state without its key")
        || !Expect(HasNoSceneActions(lua),
            "Locked door queued scene mutations without its key")
        || !Expect(lua.PopAudioRequest(audioRequest)
                && audioRequest.type == AudioRequestType::PlaySound
                && audioRequest.assetId == "storageDoorLocked",
            "Locked door did not request its sound")
        || !Expect(!lua.PopAudioRequest(audioRequest),
            "Locked door queued an unexpected extra sound")
        || !Expect(inventory.AddItem("storage_door_key"),
            "Door interaction test could not add its key")
        || !RunDialogScript(
            lua,
            menu,
            "assets/scenes/boatscene/objects/door/interact.lua",
            "storageDoorObj",
            1,
            "storageDoorOpen"))
    {
        return false;
    }

    GroundRegionRequest groundRequest;
    if (!Expect(!inventory.HasItem("storage_door_key"),
            "Unlocked door did not consume its key")
        || !Expect(state.GetTag("storageLantern_door_open"),
            "Unlocked door did not persist its open state")
        || !Expect(!lua.PopAudioRequest(audioRequest),
            "Unlocked door queued an unexpected extra sound")
        || !Expect(PopGroundRegionRequest(lua, groundRequest),
            "Unlocked door did not change its ground behavior"))
    {
        return false;
    }

    const auto* behavior = std::get_if<GroundBehavior>(&groundRequest.change);
    if (!Expect(groundRequest.regionId == "storageDoor"
                && behavior != nullptr
                && *behavior == GroundBehavior::Warp,
            "Unlocked door requested the wrong ground behavior"))
    {
        return false;
    }

    ObjectActionRequest objectRequest;
    return Expect(PopObjectAction(lua, objectRequest),
            "Unlocked door did not request its removal")
        && Expect(objectRequest.type == ObjectActionType::Destroy
                && objectRequest.value == "storageDoorObj",
            "Unlocked door requested removal of the wrong object")
        && Expect(HasNoSceneActions(lua),
            "Unlocked door queued unexpected scene mutations");
}

bool TestStartBattleLuaBinding()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    if (!lua.Initialize()
        || !lua.ExecuteSceneFile("tests/start_battle.lua", "storageRoom"))
    {
        return false;
    }

    BattleRequest request;
    return Expect(lua.PopBattleRequest(request),
            "StartBattle did not queue a battle request")
        && Expect(request.enemyIds.size() == 2,
            "StartBattle queued the wrong enemy count")
        && Expect(request.enemyIds[0] == "goblinSword"
                && request.enemyIds[1] == "goblinClub",
            "StartBattle did not preserve enemy order")
        && Expect(!lua.PopBattleRequest(request),
            "Battle request was not consumed");
}

bool TestStorageBattleTriggerScript()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    if (!lua.Initialize()
        || !lua.ExecuteSceneFile(
            "assets/scenes/storageroom/start_battle.lua",
            "storageRoom"))
    {
        return false;
    }

    GroundRegionRequest groundRequest;
    BattleRequest battleRequest;
    if (!Expect(state.GetTag("storageBattle_triggered"),
            "Storage battle script did not persist its one-shot tag")
        || !Expect(PopGroundRegionRequest(lua, groundRequest),
            "Storage battle script did not disable its ground trigger")
        || !Expect(groundRequest.regionId == "storageBattle",
            "Storage battle script changed the wrong ground region")
        || !Expect(lua.PopBattleRequest(battleRequest),
            "Storage battle script did not start a battle")
        || !Expect(battleRequest.enemyIds.size() == 2,
            "Storage battle script requested the wrong enemy count"))
    {
        return false;
    }

    const auto* behavior = std::get_if<GroundBehavior>(&groundRequest.change);
    if (!Expect(behavior != nullptr
                && *behavior == GroundBehavior::Walkable,
            "Storage battle script did not make its region walkable"))
    {
        return false;
    }

    if (!lua.ExecuteSceneFile(
            "assets/scenes/storageroom/init.lua",
            "storageRoom")
        || !Expect(PopGroundRegionRequest(lua, groundRequest),
            "Storage init did not reconstruct the disabled battle trigger"))
    {
        return false;
    }
    behavior = std::get_if<GroundBehavior>(&groundRequest.change);
    return Expect(groundRequest.regionId == "storageBattle"
            && behavior != nullptr
            && *behavior == GroundBehavior::Walkable,
        "Storage init reconstructed the wrong battle-region behavior");
}

bool TestGoblinBattleTactics()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    if (!lua.Initialize())
    {
        return false;
    }

    BattleDecisionContext context;
    context.actor = {
        "goblinSword", "Goblin A", 1, 30, 30, 0, 0, 8, 2,
        {"swordStrike"}};
    context.player = {
        "battlePlayerObj", "Cecil", 1, 142, 142, 36, 36, 12, 5,
        {"strike"}};
    context.enemies.push_back(context.actor);

    BattleActionIntent intent;
    if (!Expect(
            lua.ChooseBattleAction(
                "assets/battle/characters/goblin_sword/"
                "goblin_sword.battle.lua",
                context,
                intent),
            "Sword-goblin tactics did not return an action")
        && Expect(
            intent.moveId == "swordStrike"
                && intent.targetId == "battlePlayerObj",
            "Sword-goblin tactics returned the wrong action"))
    {
        return false;
    }

    context.actor = {
        "goblinClub", "Goblin B", 1, 36, 36, 0, 0, 9, 3,
        {"clubStrike"}};
    context.enemies = {context.actor};
    intent = {};
    return Expect(
            lua.ChooseBattleAction(
                "assets/battle/characters/goblin_club/"
                "goblin_club.battle.lua",
                context,
                intent),
            "Club-goblin tactics did not return an action")
        && Expect(
            intent.moveId == "clubStrike"
                && intent.targetId == "battlePlayerObj",
            "Club-goblin tactics returned the wrong action");
}

bool TestAudioLuaBindings()
{
    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    if (!lua.Initialize()
        || !lua.ExecuteFile("tests/audio_requests.lua", "audioTest"))
    {
        return false;
    }

    AudioRequest request;
    if (!Expect(lua.PopAudioRequest(request),
            "PlaySound did not queue an audio request")
        || !Expect(request.type == AudioRequestType::PlaySound
                && request.assetId == "doorOpen"
                && !request.loop,
            "PlaySound queued the wrong request")
        || !Expect(lua.PopAudioRequest(request),
            "PlayMusic did not queue an audio request")
        || !Expect(request.type == AudioRequestType::PlayMusic
                && request.assetId == "shipTheme"
                && request.loop,
            "PlayMusic did not default to looping")
        || !Expect(lua.PopAudioRequest(request),
            "Non-looping PlayMusic did not queue an audio request")
        || !Expect(request.type == AudioRequestType::PlayMusic
                && request.assetId == "battleIntro"
                && !request.loop,
            "PlayMusic did not preserve its loop argument")
        || !Expect(lua.PopAudioRequest(request),
            "StopMusic did not queue an audio request")
        || !Expect(request.type == AudioRequestType::StopMusic
                && request.assetId.empty(),
            "StopMusic queued the wrong request")
        || !Expect(!lua.PopAudioRequest(request),
            "Audio request queue contained an unexpected request"))
    {
        return false;
    }

    return true;
}

bool TestCrewDialogProgression(
    const std::string& crew,
    const std::string& initialScript,
    const std::string& missionScript)
{
    const std::string prefix =
        "assets/scenes/boatscene/characters/" + crew + "/";
    RuleTable rules;
    if (!rules.Load(prefix + "dialog.rules.yaml"))
    {
        return false;
    }

    GlobalState state;
    if (!ExpectScript(rules, state, prefix + initialScript))
    {
        return false;
    }

    state.SetTag("mission_storageLantern", true);
    if (!ExpectScript(rules, state, prefix + missionScript))
    {
        return false;
    }

    state.SetTag("storageLantern_has_doorKey", true);
    if (!ExpectScript(rules, state, prefix + "dialog_has_key.lua"))
    {
        return false;
    }

    state.SetTag("storageLantern_door_open", true);
    if (!ExpectScript(rules, state, prefix + "dialog_door_open.lua"))
    {
        return false;
    }

    state.SetTag("storageBattle_triggered", true);
    if (!ExpectScript(rules, state, prefix + "dialog_after_battle.lua"))
    {
        return false;
    }

    state.SetTag("storageLantern_has_lantern", true);
    if (!ExpectScript(rules, state, prefix + "dialog_has_lantern.lua"))
    {
        return false;
    }

    state.SetTag("storageLantern_fairies_released", true);
    if (!ExpectScript(rules, state, prefix + "dialog_fairies.lua"))
    {
        return false;
    }
    if (crew == "crew3")
    {
        state.SetTag("shipFinale_complete", true);
        return ExpectScript(rules, state, prefix + "dialog_complete.lua");
    }
    return true;
}
}

int main()
{
    if (!TestSceneCutsceneCoroutine()
        || !TestCutsceneArguments()
        || !TestWeatherFadeLuaBinding()
        || !TestPlayerInputControl()
        || !TestAudioLuaBindings()
        || !TestGoblinBattleTactics()
        || !TestStorageBattleTriggerScript()
        || !TestStartBattleLuaBinding()
        || !TestStorageDoorInteraction()
        || !TestGuidingLanternUseContext()
        || !TestGuidingLanternPickup()
        || !TestStorageDoorKeyPickup()
        || !TestBoatSceneReconciliation()
        || !TestSceneInitLuaContext()
        || !TestGroundRegionLuaBindings()
        || !TestObjectActionLuaBindings()
        || !TestInventoryLuaBindings()
        || !TestDutiesCoroutine()
        || !TestConsecutiveDialogPages()
        || !TestCrewDialogProgression(
            "crew1",
            "dialog_start_mission.lua",
            "dialog_find_key.lua")
        || !TestCrewDialogProgression(
            "crew2",
            "dialog_idle.lua",
            "dialog_stowaways.lua")
        || !TestCrewDialogProgression(
            "crew3",
            "dialog_idle.lua",
            "dialog_rain.lua"))
    {
        return 1;
    }

    RuleTable dutiesRules;
    GlobalState dutiesState;
    if (!dutiesRules.Load("assets/scenes/boatscene/characters/crew1/duties.rules.yaml")
        || !ExpectScript(
            dutiesRules,
            dutiesState,
            "assets/scenes/boatscene/characters/crew1/duties.lua"))
    {
        return 1;
    }

    RuleTable rules;
    if (!rules.Load("assets/scenes/boatscene/characters/crew1/dialog.rules.yaml"))
    {
        return 1;
    }

    GlobalState state;
    Inventory inventory;
    MenuWindow menu;
    LuaSystem lua(state, inventory, menu);
    if (!lua.Initialize())
    {
        return 1;
    }

    if (!ExpectScript(
            rules,
            state,
            "assets/scenes/boatscene/characters/crew1/dialog_start_mission.lua"))
    {
        return 1;
    }
    if (!RunDialogScript(
            lua,
            menu,
            "assets/scenes/boatscene/characters/crew1/dialog_start_mission.lua",
            "crew1",
            3)
        || !state.GetTag("mission_storageLantern"))
    {
        return 1;
    }

    ObjectActionRequest missionAction;
    if (!Expect(PopObjectAction(lua, missionAction),
            "Mission start did not request the key spawn")
        || !Expect(missionAction.type == ObjectActionType::Spawn,
            "Mission start queued the wrong object action")
        || !Expect(
            missionAction.value
                == "assets/scenes/boatscene/objects/key/storage_door_key.obj.yaml",
            "Mission start requested the wrong object definition"))
    {
        return 1;
    }

    if (!ExpectScript(
            rules,
            state,
            "assets/scenes/boatscene/characters/crew1/dialog_find_key.lua")
        || !RunDialogScript(
            lua,
            menu,
            "assets/scenes/boatscene/characters/crew1/dialog_find_key.lua",
            "crew1",
            1)
        || !Expect(!state.GetTag("storageLantern_has_doorKey"),
            "Find-key dialogue changed key ownership")
        || !Expect(HasNoSceneActions(lua),
            "Repeat crew dialogue requested another key"))
    {
        return 1;
    }

    state.SetTag("storageLantern_has_doorKey", true);
    if (!ExpectScript(
            rules,
            state,
            "assets/scenes/boatscene/characters/crew1/dialog_has_key.lua")
        || !RunDialogScript(
            lua,
            menu,
            "assets/scenes/boatscene/characters/crew1/dialog_has_key.lua",
            "crew1",
            1))
    {
        return 1;
    }

    GlobalState keyOnly;
    keyOnly.SetTag("storageLantern_has_doorKey", true);
    if (!ExpectScript(
            rules,
            keyOnly,
            "assets/scenes/boatscene/characters/crew1/dialog_start_mission.lua"))
    {
        return 1;
    }

    return ExpectScript(
        rules,
        state,
        "assets/scenes/boatscene/characters/crew1/dialog_has_key.lua") ? 0 : 1;
}
