#include "WorldScene.h"

#include "../../Framework/Rendering/Camera.h"
#include "../../Framework/Collision/CollisionMask.h"
#include "../../Platform/DebugLog.h"
#include "../../Framework/Resources/ResourceManager.h"
#include "SceneBuilder.h"
#include "../Objects/InteractableObject.h"
#include "../Objects/Player.h"
#include "../State/GlobalState.h"
#include "../Battle/BattleRequest.h"
#include "../Input/InputState.h"
#include "../World/GroundRegionSceneChange.h"
#include "../../Framework/Layer/ObjectLayer.h"
#include "../Scripting/LuaSystem.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <utility>

namespace
{
constexpr float InitialNearestInteractionDistanceSquared =
    std::numeric_limits<float>::max();
}

WorldScene::WorldScene(
    ResourceManager& resources,
    std::string mapPath,
    std::string collisionMaskId,
    GlobalState& globalState,
    LuaSystem& luaSystem,
    std::string requestedPlayerSpawnPointId,
    std::optional<PlayerSpawnPosition> requestedPlayerPosition)
    : BaseScene(resources, std::move(mapPath)),
      collisionMaskId_(std::move(collisionMaskId)),
      requestedPlayerSpawnPointId_(
          std::move(requestedPlayerSpawnPointId)),
      requestedPlayerPosition_(requestedPlayerPosition),
      globalState_(globalState),
      luaSystem_(luaSystem)
{
}

bool WorldScene::SetCameraTarget(std::string_view objectId)
{
    if (objectId.empty())
    {
        return false;
    }
    cameraTargetId_ = objectId;
    return true;
}

bool WorldScene::AddPlayerSpawnPoint(std::string id, float x, float y)
{
    return !id.empty()
        && playerSpawnPoints_.emplace(
            std::move(id),
            PlayerSpawnPoint{x, y}).second;
}

bool WorldScene::AddGroundRegion(
    std::string id,
    std::uint32_t color,
    GroundBehavior behavior,
    GroundParameterMap parameters,
    std::string hook)
{
    return groundRegions_.AddRegion(
        std::move(id),
        color,
        behavior,
        std::move(parameters),
        std::move(hook));
}

bool WorldScene::OnGameObjectCreated(
    BaseGameObject& object,
    ObjectLayer& owner)
{
    if (object.GetId().empty() || objectOwners_.contains(object.GetId()))
    {
        WriteDebugMessage(
            "World object IDs must be unique: " + object.GetId());
        return false;
    }

    auto* player = dynamic_cast<Player*>(&object);
    if (player != nullptr)
    {
        if (player_ != nullptr)
        {
            return false;
        }
        player_ = player;
        if (object.GetId() == cameraTargetId_)
        {
            cameraTarget_ = player;
        }
    }
    else if (auto* interactable = dynamic_cast<InteractableObject*>(&object);
        interactable != nullptr)
    {
        interactable->SetRuntime(globalState_, luaSystem_);
        interactables_.push_back(interactable);
    }

    objectOwners_.emplace(object.GetId(), &owner);
    return true;
}

bool WorldScene::Init()
{
    collisionMask_ = resources_.GetCollisionMask(collisionMaskId_);
    if (player_ == nullptr
        || collisionMask_ == nullptr
        || (!cameraTargetId_.empty() && cameraTarget_ == nullptr)
        || !BaseScene::Init())
    {
        return false;
    }

    if (requestedPlayerPosition_.has_value())
    {
        player_->SetPosition(
            requestedPlayerPosition_->x,
            requestedPlayerPosition_->y);
    }
    else if (!requestedPlayerSpawnPointId_.empty())
    {
        const auto spawnPoint = playerSpawnPoints_.find(
            requestedPlayerSpawnPointId_);
        if (spawnPoint == playerSpawnPoints_.end())
        {
            WriteDebugMessage(
                "World scene has no requested player spawn point: "
                + requestedPlayerSpawnPointId_);
            return false;
        }
        player_->SetPosition(
            spawnPoint->second.x,
            spawnPoint->second.y);
    }

    const std::string& initScript = GetSceneInitScript();
    if ((!initScript.empty()
            && !luaSystem_.ExecuteSceneFile(initScript, GetSceneId()))
        || !ProcessSceneActions())
    {
        return false;
    }

    UpdateCamera();
    weather_.Update(0.0F, globalState_);
    return true;
}

bool WorldScene::AddReturnContext(BattleRequest& request) const
{
    if (player_ == nullptr || GetSceneId().empty())
    {
        return false;
    }

    request.returnSceneId = GetSceneId();
    request.returnParameters = {
        {"playerX", static_cast<double>(player_->GetX())},
        {"playerY", static_cast<double>(player_->GetY())}};
    return true;
}

std::optional<PlayerSpawnPosition>
WorldScene::GetPlayerPosition() const noexcept
{
    if (player_ == nullptr)
    {
        return std::nullopt;
    }
    return PlayerSpawnPosition{player_->GetX(), player_->GetY()};
}

std::string WorldScene::GetPlayerGroundRegionId() const
{
    if (player_ == nullptr || collisionMask_ == nullptr)
    {
        return {};
    }

    const GroundRegion* region = player_->FindGroundRegionUnderfoot(
        *collisionMask_,
        groundRegions_);
    return region != nullptr ? region->id : std::string{};
}

void WorldScene::Update(float deltaTime, const InputState& input)
{
    if (luaSystem_.IsPlayerInputEnabled())
    {
        scriptedPlayerAnimation_ = false;
    }
    if (!scriptedPlayerAnimation_)
    {
        player_->ApplyInput(
            deltaTime,
            luaSystem_.IsPlayerInputEnabled() ? input : InputState{},
            *collisionMask_,
            groundRegions_);
    }
    BaseScene::Update(deltaTime, input);
    UpdateCutsceneActions(deltaTime);
    UpdateCamera();
    weather_.Update(deltaTime, globalState_);

    InteractableObject* selected = nullptr;
    float nearestDistance = InitialNearestInteractionDistanceSquared;
    const float playerX = player_->GetInteractionX();
    const float playerY = player_->GetInteractionY();

    for (InteractableObject* interactable : interactables_)
    {
        interactable->SetInteractionPromptVisible(false);
        const float distance = interactable->GetInteractionDistanceSquared(
            playerX,
            playerY);

        // Several interaction areas may overlap. Only the nearest valid object
        // owns the prompt and receives this frame's edge-triggered E press.
        if (interactable->HasInteractionHook()
            && interactable->IsInInteractionRange(playerX, playerY)
            && distance < nearestDistance)
        {
            selected = interactable;
            nearestDistance = distance;
        }
    }

    if (selected != nullptr && luaSystem_.IsPlayerInputEnabled())
    {
        selected->SetInteractionPromptVisible(true);
        if (input.primaryActionPressed)
        {
            selected->Interact();
        }
    }

    // Lua may have requested scene mutation from INIT, INTERACT, or a resumed
    // coroutine. Apply it only after every object and interaction traversal
    // for this frame has completed.
    CompleteObjectFades();
    CompleteWeatherFade();
    ProcessSceneActions();
    UpdateGroundBehavior();
    std::erase_if(cutsceneCoroutines_, [this](LuaCoroutineHandle handle)
        { return !luaSystem_.IsCoroutineActive(handle); });
}

void WorldScene::Shutdown()
{
    for (auto& handle : cutsceneCoroutines_)
    {
        luaSystem_.StopCoroutine(handle);
    }
    cutsceneCoroutines_.clear();
    for (auto& move : activeObjectMoves_)
    {
        luaSystem_.StopCoroutine(move.coroutineHandle);
    }
    activeObjectMoves_.clear();
    for (auto& wait : activeWaits_)
    {
        luaSystem_.StopCoroutine(wait.coroutineHandle);
    }
    activeWaits_.clear();
    luaSystem_.StopCoroutine(cameraMoveCoroutine_);
    luaSystem_.StopCoroutine(sceneFadeCoroutine_);
    luaSystem_.StopCoroutine(weatherFadeCoroutine_);
    for (ActiveObjectFade& fade : activeObjectFades_)
    {
        LuaCoroutineHandle handle = fade.coroutineHandle;
        luaSystem_.StopCoroutine(handle);
    }
    activeObjectFades_.clear();
    BaseScene::Shutdown();
}

std::optional<SceneChangeRequest>
WorldScene::TakeSceneChangeRequest()
{
    std::optional<SceneChangeRequest> request =
        std::move(pendingSceneChange_);
    pendingSceneChange_.reset();
    return request;
}

bool WorldScene::ProcessSceneActions()
{
    bool allSucceeded = true;
    SceneActionRequest request;
    while (luaSystem_.PopSceneAction(request))
    {
        if (const auto succeeded = ProcessCutsceneAction(request))
        {
            allSucceeded = *succeeded && allSucceeded;
            continue;
        }
        if (auto* groundRequest =
                std::get_if<GroundRegionRequest>(&request))
        {
            bool succeeded = false;
            if (const auto* behavior =
                    std::get_if<GroundBehavior>(&groundRequest->change))
            {
                succeeded = groundRegions_.SetBehavior(
                    groundRequest->regionId,
                    *behavior);
            }
            else if (auto* parameters =
                    std::get_if<GroundParameterMap>(&groundRequest->change))
            {
                succeeded = groundRegions_.SetParameters(
                    groundRequest->regionId,
                    std::move(*parameters));
            }

            if (!succeeded)
            {
                allSucceeded = false;
                WriteDebugMessage(
                    "Could not update ground region: "
                    + groundRequest->regionId);
            }
            continue;
        }

        if (auto* fadeRequest = std::get_if<FadeWeatherRequest>(&request))
        {
            if (weatherFadeCoroutine_ != InvalidLuaCoroutineHandle
                || !weather_.BeginFadeOut(
                    fadeRequest->durationSeconds, globalState_))
            {
                allSucceeded = false;
                CancelCutscene(fadeRequest->coroutineHandle);
                WriteDebugMessage("Could not fade weather.");
            }
            else
            {
                weatherFadeCoroutine_ = fadeRequest->coroutineHandle;
            }
            continue;
        }

        if (auto* fadeRequest = std::get_if<FadeObjectRequest>(&request))
        {
            if (!StartObjectFade(
                    fadeRequest->objectId,
                    fadeRequest->durationSeconds,
                    fadeRequest->coroutineHandle))
            {
                allSucceeded = false;
                CancelCutscene(fadeRequest->coroutineHandle);
                WriteDebugMessage(
                    "Could not fade object: " + fadeRequest->objectId);
            }
            continue;
        }

        auto* objectRequest = std::get_if<ObjectActionRequest>(&request);
        if (objectRequest == nullptr)
        {
            allSucceeded = false;
            continue;
        }

        const bool succeeded = objectRequest->type == ObjectActionType::Spawn
            ? SpawnObject(*objectRequest)
            : DestroyObject(objectRequest->value);
        if (!succeeded)
        {
            allSucceeded = false;
            const char* action = objectRequest->type == ObjectActionType::Spawn
                ? "spawn"
                : "destroy";
            WriteDebugMessage(
                std::string("Could not ") + action + " object: "
                + objectRequest->value);
        }
    }

    return allSucceeded;
}

bool WorldScene::StartObjectFade(
    const std::string& objectId,
    float durationSeconds,
    std::uint64_t coroutineHandle)
{
    if (coroutineHandle == InvalidLuaCoroutineHandle
        || std::ranges::any_of(
            activeObjectFades_,
            [&objectId](const ActiveObjectFade& fade)
            {
                return fade.objectId == objectId;
            }))
    {
        return false;
    }

    BaseGameObject* object = FindObject(objectId);
    if (object == nullptr || !object->BeginFadeOut(durationSeconds))
    {
        return false;
    }

    activeObjectFades_.push_back(
        ActiveObjectFade{objectId, coroutineHandle});
    return true;
}

void WorldScene::CompleteObjectFades()
{
    std::size_t index = 0;
    while (index < activeObjectFades_.size())
    {
        ActiveObjectFade& fade = activeObjectFades_[index];
        BaseGameObject* object = FindObject(fade.objectId);
        if (object != nullptr && !object->IsFadeOutComplete())
        {
            ++index;
            continue;
        }

        LuaCoroutineHandle handle = fade.coroutineHandle;
        activeObjectFades_.erase(activeObjectFades_.begin() + index);
        if (object == nullptr)
        {
            luaSystem_.StopCoroutine(handle);
            continue;
        }

        luaSystem_.ResumeCoroutine(handle, "objectFade");
    }
}

void WorldScene::CompleteWeatherFade()
{
    if (weatherFadeCoroutine_ == InvalidLuaCoroutineHandle
        || !weather_.IsFadeOutComplete())
    {
        return;
    }

    LuaCoroutineHandle handle = weatherFadeCoroutine_;
    weatherFadeCoroutine_ = InvalidLuaCoroutineHandle;
    luaSystem_.ResumeCoroutine(handle, "weatherFade");
}

BaseGameObject* WorldScene::FindObject(std::string_view objectId) noexcept
{
    const auto owner = objectOwners_.find(std::string(objectId));
    return owner != objectOwners_.end()
        ? owner->second->FindObject(objectId)
        : nullptr;
}

bool WorldScene::SpawnObject(const ObjectActionRequest& request)
{
    std::string targetLayerId;
    std::unique_ptr<BaseGameObject> object =
        SceneBuilder::LoadGameObjectDefinition(
            request.value,
            targetLayerId,
            request.idOverride,
            request.xOverride,
            request.yOverride);
    if (object == nullptr || objectOwners_.contains(object->GetId()))
    {
        return false;
    }

    auto* owner = dynamic_cast<ObjectLayer*>(FindLayer(targetLayerId));
    if (owner == nullptr || !OnGameObjectCreated(*object, *owner))
    {
        return false;
    }

    if (!object->Init(resources_))
    {
        UnregisterGameObject(*object);
        return false;
    }

    if (!owner->AddObject(std::move(object)))
    {
        return false;
    }
    return true;
}

bool WorldScene::DestroyObject(std::string_view objectId)
{
    const auto ownerEntry = objectOwners_.find(std::string(objectId));
    if (ownerEntry == objectOwners_.end())
    {
        return false;
    }

    ObjectLayer* owner = ownerEntry->second;
    BaseGameObject* object = owner->FindObject(objectId);
    if (object == nullptr || object == player_)
    {
        // WorldScene requires its one player for movement and camera state.
        // The object-lifetime API is currently for authored world objects.
        return false;
    }

    for (std::size_t index = 0; index < activeObjectFades_.size();)
    {
        if (activeObjectFades_[index].objectId != objectId)
        {
            ++index;
            continue;
        }

        LuaCoroutineHandle handle =
            activeObjectFades_[index].coroutineHandle;
        activeObjectFades_.erase(activeObjectFades_.begin() + index);
        luaSystem_.StopCoroutine(handle);
    }

    UnregisterGameObject(*object);
    return owner->RemoveObject(objectId);
}

void WorldScene::UnregisterGameObject(BaseGameObject& object) noexcept
{
    if (auto* interactable = dynamic_cast<InteractableObject*>(&object);
        interactable != nullptr)
    {
        std::erase(interactables_, interactable);
    }
    objectOwners_.erase(object.GetId());
}

void WorldScene::UpdateCamera()
{
    if (scriptedCamera_)
    {
        if (BaseGameObject* target = FindObject(scriptedCameraTarget_))
        {
            float x = 0.0F;
            float y = 0.0F;
            if (target->GetWorldPosition(x, y))
            {
                GetCurrentCamera()->Focus(
                    x + scriptedCameraOffsetX_, y + scriptedCameraOffsetY_);
            }
        }
        return;
    }
    if (cameraTarget_ != nullptr)
    {
        GetCurrentCamera()->Follow(
            cameraTarget_->GetCameraX(),
            cameraTarget_->GetCameraY());
    }
}

void WorldScene::UpdateGroundBehavior()
{
    const GroundRegion* region = player_->FindGroundRegionUnderfoot(
        *collisionMask_,
        groundRegions_);
    const std::string regionId = region != nullptr
        ? region->id
        : std::string{};

    // Ground behavior fires on entry, not once per frame while a player stands
    // on the same colored pixels. Leaving or entering another region rearms it.
    if (regionId == activeGroundRegionId_)
    {
        return;
    }
    activeGroundRegionId_ = regionId;

    if (region != nullptr && region->behavior == GroundBehavior::Warp)
    {
        pendingSceneChange_ = BuildSceneChangeRequest(*region);
    }
    else if (region != nullptr && region->behavior == GroundBehavior::Trigger)
    {
        // C++ owns detecting the one-shot region entry. The hook gives that
        // authored place its scene-specific Lua meaning.
        if (!luaSystem_.ExecuteSceneFile(region->hook, GetSceneId()))
        {
            WriteDebugMessage("Ground trigger hook failed: " + region->id);
            return;
        }
        ProcessSceneActions();
    }
}

void WorldScene::Render(const Graphics& graphics)
{
    BaseScene::Render(graphics);

    // Weather is screen-space, like the legacy effect, but remains below
    // interaction prompts and the game-level dialogue/menu layer.
    weather_.Render(graphics, resources_);

    const Camera* camera = GetCurrentCamera();
    if (camera == nullptr)
    {
        return;
    }

    // Interaction prompts belong to the world scene, but not to its depth
    // sorting. Draw them after every map layer so actors and foreground art
    // cannot cover the selected object's E prompt.
    for (InteractableObject* interactable : interactables_)
    {
        interactable->RenderOverlay(graphics, *camera);
    }
}
