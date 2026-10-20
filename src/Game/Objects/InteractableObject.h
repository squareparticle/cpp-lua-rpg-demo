#pragma once

#include "../../Framework/Objects/BaseGameObject.h"
#include "../Scripting/LuaSystem.h"
#include "../../Framework/Objects/ObjectHook.h"
#include "../Scripting/RuleTable.h"
#include "TriggerRegion.h"

#include <string>
#include <string_view>
#include <vector>

class GlobalState;

class InteractableObject final : public BaseGameObject
{
public:
    InteractableObject(
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
        std::vector<ObjectHookBinding> hooks);

    void SetRuntime(GlobalState& globalState, LuaSystem& luaSystem) noexcept;
    bool IsInInteractionRange(float x, float y) const noexcept;
    float GetInteractionDistanceSquared(float x, float y) const noexcept;
    bool HasInteractionHook() const noexcept;
    void SetInteractionPromptVisible(bool visible) noexcept;
    bool Interact();
    bool FireDutiesTrigger(std::string_view trigger);

    bool Init(ResourceManager& resources) override;
    void Update(float deltaTime) override;
    void Render(const Graphics& graphics, const Camera& camera) override;
    void RenderOverlay(const Graphics& graphics, const Camera& camera);
    float GetRenderSortKey() const noexcept override;
    bool GetWorldPosition(float& x, float& y) const noexcept override;
    bool SetWorldPosition(float x, float y) noexcept override;
    void Shutdown() override;

private:
    struct RuntimeHook
    {
        ObjectHookBinding binding;
        RuleTable ruleTable;
    };

    RuntimeHook* FindHook(ObjectHook hook) noexcept;
    const RuntimeHook* FindHook(ObjectHook hook) const noexcept;
    const TriggerRegion* FindTrigger(std::string_view name) const noexcept;
    bool ResolveHookScript(ObjectHook hook, std::string& scriptPath);
    bool FireHook(ObjectHook hook);
    bool StartDuties();

    std::string spriteId_;
    std::string animationName_;
    float x_ = 0.0F;
    float y_ = 0.0F;
    float scale_ = 1.0F;
    float sortPointY_ = 0.0F;
    std::string promptSpriteId_;
    std::string promptAnimationName_;
    float promptScale_ = 1.0F;
    SpriteAnimator promptSpriteAnimator_;
    std::vector<TriggerRegion> triggers_;
    std::vector<RuntimeHook> hooks_;
    GlobalState* globalState_ = nullptr;
    LuaSystem* luaSystem_ = nullptr;
    LuaCoroutineHandle dutiesCoroutine_ = InvalidLuaCoroutineHandle;
    LuaCoroutineHandle interactionCoroutine_ = InvalidLuaCoroutineHandle;
    bool promptVisible_ = false;
    bool dutiesStarted_ = false;
    bool shutdown_ = false;
};
