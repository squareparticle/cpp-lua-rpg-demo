#include "../src/Game/Scenes/WorldScene.h"
#include "../src/Game/Scenes/EndingScene.h"
#include "../src/Game/Scenes/SceneBuilder.h"
#include "../src/Game/Objects/InteractableObject.h"
#include "../src/Game/State/GlobalState.h"
#include "../src/Game/State/Inventory.h"
#include "../src/Game/UI/DialogueWindow.h"
#include "../src/Game/Input/InputState.h"
#include "../src/Framework/Resources/ResourceManager.h"
#include "../src/Platform/Graphics.h"

#include <d2d1.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <cmath>
#include <filesystem>
#include <iostream>

using Microsoft::WRL::ComPtr;

namespace
{
bool Expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << message << '\n';
    }
    return condition;
}

class TestWorld final : public WorldScene
{
public:
    TestWorld(ResourceManager& resources, GlobalState& state, LuaSystem& lua)
        : WorldScene(resources, "assets/scenes/boatscene/map.yaml",
            "walkableArea", state, lua)
    {
    }

    bool OnGameObjectCreated(BaseGameObject& object, ObjectLayer& owner) override
    {
        if (object.GetId() == "crew3")
        {
            mage = dynamic_cast<InteractableObject*>(&object);
        }
        if (object.GetId() == "crew1")
        {
            firstCrew = dynamic_cast<InteractableObject*>(&object);
        }
        return WorldScene::OnGameObjectCreated(object, owner);
    }

    InteractableObject* mage = nullptr;
    InteractableObject* firstCrew = nullptr;
};

bool AdvanceDialogue(DialogueWindow& dialogue, LuaSystem& lua)
{
    dialogue.Update(1.0F, {});
    InputState confirm;
    confirm.primaryActionPressed = true;
    dialogue.Update(0.0F, confirm);
    if (dialogue.GetState() == DialogueWindowState::Waiting)
    {
        dialogue.Update(0.0F, confirm);
    }
    return dialogue.ConsumeAdvanceRequest() && lua.CompleteDialog();
}

bool RunOpening(ID2D1RenderTarget* target)
{
    ResourceManager resources;
    GlobalState state;
    Inventory inventory;
    DialogueWindow dialogue;
    LuaSystem lua(state, inventory, dialogue);
    if (!lua.Initialize())
    {
        return false;
    }
    TestWorld world(resources, state, lua);
    if (!Expect(SceneBuilder::BuildScene(world.GetMapPath(), target,
            resources, world) && world.Init() && world.firstCrew != nullptr,
            "Could not initialize the opening scene"))
    {
        world.Shutdown();
        return false;
    }
    float crewX = 0.0F;
    float crewY = 0.0F;
    world.firstCrew->GetWorldPosition(crewX, crewY);
    bool success = Expect(crewX == 705.0F && crewY == 145.0F
            && !lua.IsPlayerInputEnabled(),
            "Opening was not staged at the stairs before the first frame");
    const auto playerStart = world.GetPlayerPosition();
    InputState held;
    held.leftHeld = true;
    held.primaryActionPressed = true;
    int pages = 0;
    bool reachedDialogue = false;
    for (int frame = 0; frame < 300 && !lua.IsPlayerInputEnabled(); ++frame)
    {
        world.Update(0.1F, held);
        if (dialogue.IsModalActive())
        {
            world.firstCrew->GetWorldPosition(crewX, crewY);
            success = Expect(crewX == 720.0F && crewY == 400.0F,
                "Crew began opening dialogue before reaching the watch station")
                && success;
            reachedDialogue = true;
            ++pages;
            success = AdvanceDialogue(dialogue, lua) && success;
        }
        if (pages < 3)
        {
            success = Expect(!lua.IsPlayerInputEnabled(),
                "Opening restored control before its final dialogue closed") && success;
        }
    }
    const auto playerFinish = world.GetPlayerPosition();
    success = Expect(reachedDialogue && pages == 3
            && state.GetTag("mission_storageLantern")
            && lua.IsPlayerInputEnabled(),
            "Opening did not complete the dialogue and hand over control") && success;
    success = Expect(playerStart && playerFinish
            && playerStart->x == playerFinish->x
            && playerStart->y == playerFinish->y,
            "Player moved during the opening") && success;
    world.Update(1.0F, {});
    world.firstCrew->GetWorldPosition(crewX, crewY);
    success = Expect(crewX == 720.0F && crewY == 400.0F
            && !dialogue.IsModalActive(),
            "Crew did not remain at the watch station after opening") && success;
    world.Shutdown();
    lua.ClearSceneActions();
    resources.ClearScene();

    // The same persistent mission state is used on scene return and Save/Load.
    TestWorld returned(resources, state, lua);
    if (!SceneBuilder::BuildScene(returned.GetMapPath(), target, resources, returned)
        || !returned.Init() || returned.firstCrew == nullptr)
    {
        returned.Shutdown();
        return false;
    }
    returned.Update(2.0F, {});
    returned.firstCrew->GetWorldPosition(crewX, crewY);
    success = Expect(crewX == 720.0F && crewY == 400.0F
            && lua.IsPlayerInputEnabled() && !dialogue.IsModalActive(),
            "Returning to the boat replayed the opening") && success;
    returned.Shutdown();
    return success;
}

bool RunOpeningCancellation(ID2D1RenderTarget* target)
{
    ResourceManager resources;
    GlobalState state;
    Inventory inventory;
    DialogueWindow dialogue;
    LuaSystem lua(state, inventory, dialogue);
    if (!lua.Initialize())
    {
        return false;
    }
    TestWorld world(resources, state, lua);
    if (!SceneBuilder::BuildScene(world.GetMapPath(), target, resources, world)
        || !world.Init())
    {
        world.Shutdown();
        return false;
    }
    for (int frame = 0; frame < 300 && !dialogue.IsModalActive(); ++frame)
    {
        world.Update(0.1F, {});
    }
    const bool reachedDialogue = dialogue.IsModalActive();
    // Arrival has consumed the move waiter. Only the scene-owned coroutine
    // list can now cancel this opening; it is not an NPC duties/interaction.
    world.Shutdown();
    lua.ClearSceneActions();
    return Expect(reachedDialogue && !dialogue.IsModalActive()
            && !state.GetTag("mission_storageLantern")
            && lua.IsPlayerInputEnabled(),
        "Scene shutdown left its opening dialogue/coroutine alive");
}

bool RunFinale(ID2D1RenderTarget* target)
{
    ResourceManager resources;
    GlobalState state;
    Inventory inventory;
    DialogueWindow dialogue;
    LuaSystem lua(state, inventory, dialogue);
    if (!resources.LoadImage(target, "weatherRain",
            "assets/world/weather/rain.png", ResourceScope::Game))
    {
        return false;
    }
    state.SetTag("mission_storageLantern", true);
    state.SetTag("storageLantern_door_open", true);
    state.SetTag("storageLantern_has_lantern", true);
    state.SetTag("storageLantern_fairies_released", true);
    state.SetTag("storageLantern_flames_active", true);
    if (!lua.Initialize())
    {
        return false;
    }
    TestWorld world(resources, state, lua);
    if (!Expect(SceneBuilder::BuildScene(world.GetMapPath(), target,
            resources, world) && world.Init() && world.mage != nullptr,
            "Could not initialize headless boat scene"))
    {
        world.Shutdown();
        return false;
    }
    Graphics graphics(target);
    if (!graphics.Initialize())
    {
        world.Shutdown();
        return false;
    }
    const auto start = world.GetPlayerPosition();
    LuaCoroutineHandle invalid = InvalidLuaCoroutineHandle;
    if (!lua.StartCoroutine("tests/cutscene_failure.lua", "crew3", invalid))
    {
        world.Shutdown();
        return false;
    }
    world.Update(0.0F, {});
    if (!Expect(!lua.IsCoroutineActive(invalid) && lua.IsPlayerInputEnabled()
            && !state.GetTag("test_failed_cutscene_continued"),
            "Invalid waypoint did not safely stop the cutscene"))
    {
        world.Shutdown();
        return false;
    }
    if (!world.mage->Interact())
    {
        world.Shutdown();
        return false;
    }

    InputState held;
    held.rightHeld = true;
    held.upHeld = true;
    held.primaryActionPressed = true;
    held.menuPressed = true;
    std::optional<SceneChangeRequest> ending;
    int pages = 0;
    int unlocks = 0;
    int rainCues = 0;
    int extinguishCues = 0;
    bool success = true;
    for (int frame = 0; frame < 1200 && !ending; ++frame)
    {
        success = Expect(!lua.IsPlayerInputEnabled(),
            "Controls were enabled during the finale") && success;
        // Use deliberately held controls to exercise the world input gate.
        world.Update(0.1F, held);
        if (dialogue.IsModalActive())
        {
            success = Expect(!state.GetTag("weather=rain"),
                "Rain started before the final dialogue closed") && success;
            ++pages;
            success = AdvanceDialogue(dialogue, lua) && success;
            if (!dialogue.IsModalActive())
            {
                const std::optional<std::string> exitSound =
                    lua.TakeDialogExitSoundOverride();
                if (exitSound && *exitSound == "rainMagic")
                {
                    ++rainCues;
                    success = Expect(pages == 3,
                        "Rain cue replaced the wrong dialog exit") && success;
                }
            }
        }
        AudioRequest audio;
        while (lua.PopAudioRequest(audio))
        {
            if (audio.type == AudioRequestType::PlaySound
                && audio.assetId == "chestUnlock")
            {
                ++unlocks;
            }
            if (audio.type == AudioRequestType::PlaySound
                && audio.assetId == "boatFire")
            {
                ++extinguishCues;
            }
        }
        if (frame % 20 == 0)
        {
            target->BeginDraw();
            target->Clear(D2D1::ColorF(D2D1::ColorF::Black));
            world.Render(graphics);
            world.RenderTransition(graphics);
            success = SUCCEEDED(target->EndDraw()) && success;
        }
        ending = world.TakeSceneChangeRequest();
    }

    const auto finish = world.GetPlayerPosition();
    float mageX = 0.0F;
    float mageY = 0.0F;
    world.mage->GetWorldPosition(mageX, mageY);
    success = Expect(ending && ending->destination == "ending",
            "Finale did not reach its ending scene") && success;
    success = Expect(start && finish && start->x == finish->x
            && start->y == finish->y, "Held controls moved the cutscene player")
        && success;
    success = Expect(std::abs(mageX - 470.0F) < 0.01F
            && std::abs(mageY - 45.0F) < 0.01F,
            "Mage did not arrive at the chest waypoint") && success;
    success = Expect(rainCues == 1 && extinguishCues == 4,
            "Finale did not play one rain cue and four fire-out cues") && success;
    success = Expect(pages == 3 && unlocks == 1,
            "Finale did not show opening, Got it, Here we go or unlock exactly once")
        && success;
    success = Expect(state.GetTag("weather=rain")
            && !state.GetTag("storageLantern_flames_active")
            && state.GetTag("shipFinale_complete"),
            "Finale did not finish rain/fire/quest state") && success;
    LuaCoroutineHandle pending = InvalidLuaCoroutineHandle;
    success = lua.StartCoroutine("tests/cutscene_pending.lua", "crew3", pending)
        && success;
    world.Update(0.0F, {});
    world.Shutdown();
    success = Expect(!lua.IsCoroutineActive(pending)
            && !state.GetTag("test_cancelled_cutscene_continued"),
            "Scene shutdown did not cancel its timed waiter") && success;
    lua.ClearSceneActions();
    resources.ClearScene();
    EndingScene end(resources, lua);
    success = Expect(SceneBuilder::BuildScene(end.GetMapPath(), target,
            resources, end) && end.Init(), "Ending scene could not load") && success;
    AudioRequest endingCue;
    success = Expect(lua.PopAudioRequest(endingCue)
            && endingCue.type == AudioRequestType::PlaySound
            && endingCue.assetId == "endingVictory"
            && !lua.PopAudioRequest(endingCue),
            "Ending scene did not request its victory cue exactly once") && success;
    end.Update(1.0F, {});
    target->BeginDraw();
    end.Render(graphics);
    success = SUCCEEDED(target->EndDraw()) && success;
    end.Shutdown();
    return success;
}
}

int main()
{
    // The public checkout deliberately excludes some local artwork/audio.
    if (!std::filesystem::exists("assets/scenes/boatscene/background.png")
        || !std::filesystem::exists("assets/sounds/unlocked.wav"))
    {
        std::cout << "Local boat art/audio unavailable; headless finale test skipped.\n";
        return 77;
    }
    if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
    {
        return 1;
    }
    bool success = false;
    {
        ComPtr<IWICImagingFactory> imaging;
        ComPtr<IWICBitmap> bitmap;
        ComPtr<ID2D1Factory> factory;
        ComPtr<ID2D1RenderTarget> target;
        if (SUCCEEDED(CoCreateInstance(CLSID_WICImagingFactory, nullptr,
                CLSCTX_INPROC_SERVER, IID_PPV_ARGS(imaging.GetAddressOf())))
            && SUCCEEDED(imaging->CreateBitmap(1280, 720,
                GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad,
                bitmap.GetAddressOf()))
            && SUCCEEDED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
                factory.GetAddressOf()))
            && SUCCEEDED(factory->CreateWicBitmapRenderTarget(bitmap.Get(),
                D2D1::RenderTargetProperties(), target.GetAddressOf())))
        {
            success = RunOpening(target.Get())
                && RunOpeningCancellation(target.Get()) && RunFinale(target.Get());
        }
    }
    CoUninitialize();
    return success ? 0 : 1;
}
