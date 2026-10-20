#include "InteractableObject.h"

#include "../../Framework/Rendering/Camera.h"
#include "../../Platform/Graphics.h"
#include "../../Platform/DebugLog.h"
#include "../../Framework/Resources/ResourceManager.h"
#include "../State/GlobalState.h"
#include "../Scripting/LuaSystem.h"

#include <algorithm>
#include <limits>
#include <utility>

namespace
{
constexpr float NoInteractionDistanceSquared =
    std::numeric_limits<float>::max();
}

InteractableObject::InteractableObject(
    std::string id,
    std::string spriteId,
    std::string animationName,
    float x,
    float y,
    float scale,
    float sortPointY,
    std::string promptSpriteId,
    std::string promptAnimationName,
    float promptScale,
    std::vector<TriggerRegion> triggers,
    std::vector<ObjectHookBinding> hooks)
    : BaseGameObject(std::move(id)),
      spriteId_(std::move(spriteId)),
      animationName_(std::move(animationName)),
      x_(x),
      y_(y),
      scale_(scale),
      sortPointY_(sortPointY),
      promptSpriteId_(std::move(promptSpriteId)),
      promptAnimationName_(std::move(promptAnimationName)),
      promptScale_(promptScale),
      triggers_(std::move(triggers))
{
    hooks_.reserve(hooks.size());
    for (ObjectHookBinding& hook : hooks)
    {
        hooks_.push_back(RuntimeHook{std::move(hook), {}});
    }
}

void InteractableObject::SetRuntime(
    GlobalState& globalState,
    LuaSystem& luaSystem) noexcept
{
    globalState_ = &globalState;
    luaSystem_ = &luaSystem;
}

bool InteractableObject::IsInInteractionRange(float x, float y) const noexcept
{
    const TriggerRegion* interaction = FindTrigger("interaction");
    return interaction != nullptr
        && interaction->Contains(x_, y_, scale_, x, y);
}

float InteractableObject::GetInteractionDistanceSquared(
    float x,
    float y) const noexcept
{
    const TriggerRegion* interaction = FindTrigger("interaction");
    return interaction != nullptr
        ? interaction->GetDistanceSquared(x_, y_, scale_, x, y)
        : NoInteractionDistanceSquared;
}

bool InteractableObject::HasInteractionHook() const noexcept
{
    return FindHook(ObjectHook::Interact) != nullptr
        && FindTrigger("interaction") != nullptr;
}

void InteractableObject::SetInteractionPromptVisible(bool visible) noexcept
{
    promptVisible_ = visible;
}

bool InteractableObject::Interact()
{
    if (luaSystem_ == nullptr)
    {
        return false;
    }
    if (luaSystem_->IsCoroutineActive(interactionCoroutine_))
    {
        return true;
    }

    std::string scriptPath;
    if (!ResolveHookScript(ObjectHook::Interact, scriptPath))
    {
        return false;
    }
    if (scriptPath.empty())
    {
        return true;
    }

    // INTERACT is a coroutine because Say suspends this exact script until the
    // modal dialog completes. INIT and DESTROY remain immediate hooks.
    return luaSystem_->StartCoroutine(
        scriptPath,
        GetId(),
        interactionCoroutine_);
}

bool InteractableObject::FireDutiesTrigger(std::string_view trigger)
{
    if (luaSystem_ == nullptr
        || dutiesCoroutine_ == InvalidLuaCoroutineHandle)
    {
        return false;
    }

    return luaSystem_->ResumeCoroutine(dutiesCoroutine_, trigger);
}

bool InteractableObject::Init(ResourceManager& resources)
{
    if (globalState_ == nullptr || luaSystem_ == nullptr)
    {
        WriteDebugMessage("Interactable object has no runtime: " + GetId());
        return false;
    }
    if (FindHook(ObjectHook::Interact) != nullptr
        && FindTrigger("interaction") == nullptr)
    {
        WriteDebugMessage(
            "Interactable object has INTERACT but no interaction trigger: "
            + GetId());
        return false;
    }

    const SpriteAsset* asset = resources.GetSprite(spriteId_);
    const SpriteAsset* promptAsset = resources.GetSprite(promptSpriteId_);
    if (asset == nullptr
        || promptAsset == nullptr
        || !spriteAnimator_.Init(*asset, animationName_)
        || !promptSpriteAnimator_.Init(*promptAsset, promptAnimationName_))
    {
        WriteDebugMessage("Interactable object could not initialize sprites: " + GetId());
        return false;
    }

    for (RuntimeHook& hook : hooks_)
    {
        if (!hook.binding.ruleTablePath.empty()
            && !hook.ruleTable.Load(hook.binding.ruleTablePath))
        {
            return false;
        }
    }

    return FireHook(ObjectHook::Init);
}

void InteractableObject::Update(float deltaTime)
{
    spriteAnimator_.Update(deltaTime);
    promptSpriteAnimator_.Update(deltaTime);

    if (!dutiesStarted_)
    {
        // Update starts DUTIES once, but the script remains alive as a yielded
        // coroutine. C++ resumes it only for meaningful object-local triggers;
        // Lua is never polled or called once per frame.
        dutiesStarted_ = true;
        StartDuties();
    }
}

void InteractableObject::Render(
    const Graphics& graphics,
    const Camera& camera)
{
    const float screenX = x_ - static_cast<float>(camera.GetX());
    const float screenY = y_ - static_cast<float>(camera.GetY());
    spriteAnimator_.Render(graphics, screenX, screenY, scale_);
}

void InteractableObject::RenderOverlay(
    const Graphics& graphics,
    const Camera& camera)
{
    if (promptVisible_)
    {
        const float screenX = x_ - static_cast<float>(camera.GetX());
        const float screenY = y_ - static_cast<float>(camera.GetY());
        promptSpriteAnimator_.Render(
            graphics,
            screenX + 25.0F,
            screenY - 45.0F,
            promptScale_);
    }
}

float InteractableObject::GetRenderSortKey() const noexcept
{
    return y_ + sortPointY_ * scale_;
}

bool InteractableObject::GetWorldPosition(float& x, float& y) const noexcept
{
    x = x_;
    y = y_;
    return true;
}

bool InteractableObject::SetWorldPosition(float x, float y) noexcept
{
    x_ = x;
    y_ = y;
    return true;
}

void InteractableObject::Shutdown()
{
    if (!shutdown_)
    {
        // DESTROY cannot run from the destructor: GameController must fire it
        // explicitly while the shared Lua state still exists.
        shutdown_ = true;
        if (luaSystem_ != nullptr)
        {
            luaSystem_->StopCoroutine(dutiesCoroutine_);
            luaSystem_->StopCoroutine(interactionCoroutine_);
        }
        FireHook(ObjectHook::Destroy);
    }
}

InteractableObject::RuntimeHook* InteractableObject::FindHook(
    ObjectHook hook) noexcept
{
    const auto result = std::ranges::find_if(
        hooks_,
        [hook](const RuntimeHook& candidate)
        {
            return candidate.binding.hook == hook;
        });
    return result != hooks_.end() ? &*result : nullptr;
}

const InteractableObject::RuntimeHook* InteractableObject::FindHook(
    ObjectHook hook) const noexcept
{
    const auto result = std::ranges::find_if(
        hooks_,
        [hook](const RuntimeHook& candidate)
        {
            return candidate.binding.hook == hook;
        });
    return result != hooks_.end() ? &*result : nullptr;
}

const TriggerRegion* InteractableObject::FindTrigger(
    std::string_view name) const noexcept
{
    const auto result = std::ranges::find_if(
        triggers_,
        [name](const TriggerRegion& trigger)
        {
            return trigger.name == name;
        });
    return result != triggers_.end() ? &*result : nullptr;
}

bool InteractableObject::FireHook(ObjectHook hook)
{
    std::string scriptPath;
    if (!ResolveHookScript(hook, scriptPath))
    {
        return false;
    }
    if (scriptPath.empty())
    {
        return true;
    }

    return luaSystem_->ExecuteFile(scriptPath, GetId());
}

bool InteractableObject::StartDuties()
{
    std::string scriptPath;
    if (!ResolveHookScript(ObjectHook::Duties, scriptPath))
    {
        return false;
    }
    if (scriptPath.empty())
    {
        return true;
    }

    return luaSystem_->StartCoroutine(
        scriptPath,
        GetId(),
        dutiesCoroutine_);
}

bool InteractableObject::ResolveHookScript(
    ObjectHook hook,
    std::string& scriptPath)
{
    scriptPath.clear();
    RuntimeHook* runtimeHook = FindHook(hook);
    if (runtimeHook == nullptr)
    {
        return true;
    }

    scriptPath = runtimeHook->binding.scriptPath;
    if (!runtimeHook->binding.ruleTablePath.empty())
    {
        const ScriptRule* rule = runtimeHook->ruleTable.Resolve(*globalState_);
        if (rule == nullptr)
        {
            WriteDebugMessage(
                "No matching " + std::string(ToString(hook))
                + " rule for object '" + GetId() + "' in: "
                + runtimeHook->binding.ruleTablePath);
            return true;
        }
        scriptPath = rule->scriptPath;
    }

    return true;
}
