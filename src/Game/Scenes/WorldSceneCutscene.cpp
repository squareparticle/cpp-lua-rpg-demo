#include "WorldScene.h"

#include "../../Framework/Rendering/Camera.h"
#include "../../Platform/DebugLog.h"
#include "../../Platform/Graphics.h"
#include "../Objects/Player.h"

#include <d2d1.h>

#include <algorithm>
#include <cmath>
#include <utility>

bool WorldScene::AddWaypoint(std::string id, float x, float y)
{
    return !id.empty() && std::isfinite(x) && std::isfinite(y)
        && waypoints_.emplace(std::move(id), PlayerSpawnPosition{x, y}).second;
}

void WorldScene::CancelCutscene(std::uint64_t handle)
{
    const auto cancelledHandle = handle;
    luaSystem_.StopCoroutine(handle);
    std::erase_if(activeObjectMoves_, [cancelledHandle](const auto& move)
        { return move.coroutineHandle == cancelledHandle; });
    std::erase_if(activeWaits_, [cancelledHandle](const auto& wait)
        { return wait.coroutineHandle == cancelledHandle; });
    if (cameraMoveCoroutine_ == cancelledHandle)
    {
        cameraMoveCoroutine_ = InvalidLuaCoroutineHandle;
    }
    if (sceneFadeCoroutine_ == cancelledHandle)
    {
        sceneFadeCoroutine_ = InvalidLuaCoroutineHandle;
    }
    luaSystem_.SetPlayerInputEnabled(true);
    scriptedPlayerAnimation_ = false;
    scriptedCamera_ = false;
    sceneFadeOpacity_ = 0.0F;
    WriteDebugMessage("Cutscene action failed; player control restored.");
}

std::optional<bool> WorldScene::ProcessCutsceneAction(
    const SceneActionRequest& request)
{
    if (const auto* cutscene = std::get_if<StartCutsceneRequest>(&request))
    {
        LuaCoroutineHandle handle = InvalidLuaCoroutineHandle;
        if (!luaSystem_.StartSceneCoroutine(cutscene->scriptPath, GetSceneId(), handle))
        {
            WriteDebugMessage("Could not start cutscene: " + cutscene->scriptPath);
            luaSystem_.SetPlayerInputEnabled(true);
            return false;
        }
        if (handle != InvalidLuaCoroutineHandle)
        {
            cutsceneCoroutines_.push_back(handle);
        }
        return true;
    }

    if (const auto* move = std::get_if<MoveObjectRequest>(&request))
    {
        if (!luaSystem_.IsCoroutineWaitingFor(move->coroutineHandle, "objectMove"))
        {
            return true;
        }
        const auto point = waypoints_.find(move->waypoint);
        BaseGameObject* object = FindObject(move->objectId);
        float x = 0.0F;
        float y = 0.0F;
        if (point == waypoints_.end() || object == nullptr
            || !object->GetWorldPosition(x, y)
            || !std::isfinite(move->speed) || move->speed <= 0.0F
            || std::ranges::any_of(activeObjectMoves_, [&](const auto& active)
                { return active.objectId == move->objectId; }))
        {
            WriteDebugMessage("Could not move object: " + move->objectId
                + " to waypoint: " + move->waypoint);
            CancelCutscene(move->coroutineHandle);
            return false;
        }
        activeObjectMoves_.push_back({move->objectId, point->second,
            move->speed, move->coroutineHandle});
        return true;
    }

    if (const auto* position = std::get_if<SetObjectPositionRequest>(&request))
    {
        BaseGameObject* object = FindObject(position->objectId);
        const auto point = waypoints_.find(position->waypoint);
        if (object == nullptr || point == waypoints_.end()
            || std::ranges::any_of(activeObjectMoves_, [&](const auto& move)
                { return move.objectId == position->objectId; })
            || !object->SetWorldPosition(point->second.x, point->second.y))
        {
            WriteDebugMessage("Could not place object: " + position->objectId
                + " at waypoint: " + position->waypoint);
            return false;
        }
        return true;
    }

    if (const auto* animation = std::get_if<PlayAnimationRequest>(&request))
    {
        BaseGameObject* object = FindObject(animation->objectId);
        if (object == nullptr || !object->PlaySpriteAnimation(animation->animation))
        {
            WriteDebugMessage("Could not play animation: " + animation->objectId
                + "/" + animation->animation);
            return false;
        }
        if (object == player_)
        {
            scriptedPlayerAnimation_ = true;
        }
        return true;
    }

    if (const auto* follow = std::get_if<FollowObjectRequest>(&request))
    {
        BaseGameObject* object = FindObject(follow->objectId);
        float x = 0.0F;
        float y = 0.0F;
        if (object == nullptr || !object->GetWorldPosition(x, y)
            || cameraMoveCoroutine_ != InvalidLuaCoroutineHandle)
        {
            WriteDebugMessage("Could not follow object: " + follow->objectId);
            return false;
        }
        scriptedCamera_ = true;
        scriptedCameraTarget_ = follow->objectId;
        scriptedCameraOffsetX_ = follow->offsetX;
        scriptedCameraOffsetY_ = follow->offsetY;
        UpdateCamera();
        return true;
    }

    if (const auto* camera = std::get_if<MoveCameraRequest>(&request))
    {
        if (!luaSystem_.IsCoroutineWaitingFor(camera->coroutineHandle, "cameraMove"))
        {
            return true;
        }
        const auto point = waypoints_.find(camera->waypoint);
        if (point == waypoints_.end()
            || cameraMoveCoroutine_ != InvalidLuaCoroutineHandle
            || !GetCurrentCamera()->PanTo(
                point->second.x, point->second.y, camera->durationSeconds))
        {
            CancelCutscene(camera->coroutineHandle);
            return false;
        }
        scriptedCamera_ = true;
        scriptedCameraTarget_.clear();
        cameraMoveCoroutine_ = camera->coroutineHandle;
        return true;
    }

    if (const auto* wait = std::get_if<WaitRequest>(&request))
    {
        if (luaSystem_.IsCoroutineWaitingFor(wait->coroutineHandle, "wait"))
        {
            activeWaits_.push_back({wait->durationSeconds, wait->coroutineHandle});
        }
        return true;
    }

    if (const auto* fade = std::get_if<FadeSceneRequest>(&request))
    {
        if (!luaSystem_.IsCoroutineWaitingFor(fade->coroutineHandle, "sceneFade"))
        {
            return true;
        }
        if (sceneFadeCoroutine_ != InvalidLuaCoroutineHandle)
        {
            CancelCutscene(fade->coroutineHandle);
            return false;
        }
        sceneFadeDuration_ = fade->durationSeconds;
        sceneFadeElapsed_ = 0.0F;
        sceneFadeOpacity_ = 0.0F;
        sceneFadeCoroutine_ = fade->coroutineHandle;
        return true;
    }

    if (const auto* change = std::get_if<SceneChangeRequest>(&request))
    {
        pendingSceneChange_ = *change;
        return true;
    }
    return std::nullopt;
}

void WorldScene::UpdateCutsceneActions(float deltaTime)
{
    const float elapsed = std::max(deltaTime, 0.0F);
    for (std::size_t index = 0; index < activeObjectMoves_.size();)
    {
        auto& move = activeObjectMoves_[index];
        BaseGameObject* object = FindObject(move.objectId);
        float x = 0.0F;
        float y = 0.0F;
        if (!luaSystem_.IsCoroutineActive(move.coroutineHandle)
            || object == nullptr || !object->GetWorldPosition(x, y))
        {
            auto handle = move.coroutineHandle;
            activeObjectMoves_.erase(activeObjectMoves_.begin() + index);
            CancelCutscene(handle);
            continue;
        }
        const float dx = move.destination.x - x;
        const float dy = move.destination.y - y;
        const float distance = std::hypot(dx, dy);
        const float step = move.speed * elapsed;
        const bool arrived = distance <= step || distance <= 0.001F;
        const float fraction = arrived ? 1.0F : step / distance;
        if (!object->SetWorldPosition(x + dx * fraction, y + dy * fraction))
        {
            auto handle = move.coroutineHandle;
            activeObjectMoves_.erase(activeObjectMoves_.begin() + index);
            CancelCutscene(handle);
            continue;
        }
        if (!arrived)
        {
            ++index;
            continue;
        }
        auto handle = move.coroutineHandle;
        activeObjectMoves_.erase(activeObjectMoves_.begin() + index);
        luaSystem_.ResumeCoroutine(handle, "objectMove");
    }

    GetCurrentCamera()->Update(elapsed);
    if (cameraMoveCoroutine_ != InvalidLuaCoroutineHandle
        && GetCurrentCamera()->IsPanComplete())
    {
        auto handle = cameraMoveCoroutine_;
        cameraMoveCoroutine_ = InvalidLuaCoroutineHandle;
        luaSystem_.ResumeCoroutine(handle, "cameraMove");
    }

    for (std::size_t index = 0; index < activeWaits_.size();)
    {
        auto& wait = activeWaits_[index];
        wait.remainingSeconds -= elapsed;
        if (wait.remainingSeconds > 0.0F)
        {
            ++index;
            continue;
        }
        auto handle = wait.coroutineHandle;
        activeWaits_.erase(activeWaits_.begin() + index);
        luaSystem_.ResumeCoroutine(handle, "wait");
    }

    if (sceneFadeCoroutine_ != InvalidLuaCoroutineHandle)
    {
        sceneFadeElapsed_ += elapsed;
        sceneFadeOpacity_ = std::clamp(
            sceneFadeElapsed_ / sceneFadeDuration_, 0.0F, 1.0F);
        if (sceneFadeOpacity_ >= 1.0F)
        {
            auto handle = sceneFadeCoroutine_;
            sceneFadeCoroutine_ = InvalidLuaCoroutineHandle;
            luaSystem_.ResumeCoroutine(handle, "sceneFade");
        }
    }
}

void WorldScene::RenderTransition(const Graphics& graphics) const
{
    if (sceneFadeOpacity_ <= 0.0F || graphics.GetRenderTarget() == nullptr)
    {
        return;
    }
    const auto size = graphics.GetRenderTarget()->GetSize();
    graphics.FillRectangle({0.0F, 0.0F, size.width, size.height},
        {0x000000, sceneFadeOpacity_});
}
