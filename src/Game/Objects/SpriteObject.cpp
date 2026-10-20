#include "SpriteObject.h"

#include "../../Framework/Rendering/Camera.h"
#include "../../Platform/Graphics.h"
#include "../../Framework/Resources/ResourceManager.h"

#include <utility>
#include <algorithm>
#include <cmath>

SpriteObject::SpriteObject(
    std::string id,
    std::string spriteId,
    std::string animationName,
    float x,
    float y,
    float scale,
    float sortPointY)
    : BaseGameObject(std::move(id)),
      spriteId_(std::move(spriteId)),
      animationName_(std::move(animationName)),
      x_(x),
      y_(y),
      scale_(scale),
      sortPointY_(sortPointY)
{
}

bool SpriteObject::Init(ResourceManager& resources)
{
    const SpriteAsset* asset = resources.GetSprite(spriteId_);
    return asset != nullptr && spriteAnimator_.Init(*asset, animationName_);
}

void SpriteObject::Update(float deltaTime)
{
    spriteAnimator_.Update(deltaTime);
    if (fadeDurationSeconds_ > 0.0F)
    {
        fadeElapsedSeconds_ += std::max(deltaTime, 0.0F);
        opacity_ = 1.0F - std::clamp(
            fadeElapsedSeconds_ / fadeDurationSeconds_, 0.0F, 1.0F);
    }
}

void SpriteObject::Render(
    const Graphics& graphics,
    const Camera& camera)
{
    spriteAnimator_.Render(
        graphics,
        x_ - static_cast<float>(camera.GetX()),
        y_ - static_cast<float>(camera.GetY()),
        scale_,
        opacity_);
}

float SpriteObject::GetRenderSortKey() const noexcept
{
    return y_ + sortPointY_ * scale_;
}

bool SpriteObject::GetWorldPosition(float& x, float& y) const noexcept
{
    x = x_;
    y = y_;
    return true;
}

bool SpriteObject::SetWorldPosition(float x, float y) noexcept
{
    x_ = x;
    y_ = y;
    return true;
}

bool SpriteObject::BeginFadeOut(float durationSeconds) noexcept
{
    if (fadeDurationSeconds_ > 0.0F || !std::isfinite(durationSeconds)
        || durationSeconds <= 0.0F)
    {
        return false;
    }
    fadeDurationSeconds_ = durationSeconds;
    fadeElapsedSeconds_ = 0.0F;
    return true;
}

bool SpriteObject::IsFadeOutComplete() const noexcept
{
    return fadeDurationSeconds_ > 0.0F
        && fadeElapsedSeconds_ >= fadeDurationSeconds_;
}
