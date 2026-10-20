#include "BaseGameObject.h"

#include "../Rendering/Camera.h"
#include "../../Platform/Graphics.h"
#include "../Resources/ResourceManager.h"

#include <utility>

BaseGameObject::BaseGameObject(std::string id)
    : id_(std::move(id))
{
}

const std::string& BaseGameObject::GetId() const noexcept
{
    return id_;
}

bool BaseGameObject::Init(ResourceManager& resources)
{
    static_cast<void>(resources);
    return true;
}

void BaseGameObject::Update(float deltaTime)
{
    static_cast<void>(deltaTime);
}

void BaseGameObject::Render(const Graphics& graphics, const Camera& camera)
{
    static_cast<void>(graphics);
    static_cast<void>(camera);
}

float BaseGameObject::GetRenderSortKey() const noexcept
{
    return 0.0F;
}

bool BaseGameObject::GetWorldPosition(float& x, float& y) const noexcept
{
    static_cast<void>(x);
    static_cast<void>(y);
    return false;
}

bool BaseGameObject::SetWorldPosition(float x, float y) noexcept
{
    static_cast<void>(x);
    static_cast<void>(y);
    return false;
}

bool BaseGameObject::PlaySpriteAnimation(const std::string& animationName)
{
    return spriteAnimator_.PlayAnimation(animationName);
}

bool BaseGameObject::BeginFadeOut(float durationSeconds) noexcept
{
    static_cast<void>(durationSeconds);
    return false;
}

bool BaseGameObject::IsFadeOutComplete() const noexcept
{
    return false;
}

void BaseGameObject::Shutdown()
{
}
