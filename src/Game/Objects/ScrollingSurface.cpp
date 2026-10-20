#include "ScrollingSurface.h"

#include "../../Framework/Rendering/Camera.h"
#include "../../Platform/Graphics.h"
#include "../../Framework/Resources/ResourceManager.h"

#include <algorithm>
#include <cmath>
#include <utility>

ScrollingSurface::ScrollingSurface(
    std::string id,
    std::string spriteId,
    std::string animationName,
    float x,
    float y,
    float speedX,
    float speedY,
    std::string alternateSpriteId,
    float crossfadeSeconds,
    float crossfadeStrength)
    : BaseGameObject(std::move(id)),
      spriteId_(std::move(spriteId)),
      animationName_(std::move(animationName)),
      x_(x),
      y_(y),
      speedX_(speedX),
      speedY_(speedY),
      alternateSpriteId_(std::move(alternateSpriteId)),
      crossfadeSeconds_(crossfadeSeconds),
      crossfadeStrength_(crossfadeStrength),
      crossfadeOpacity_(0.5F - 0.5F * crossfadeStrength)
{
}

bool ScrollingSurface::Init(ResourceManager& resources)
{
    const SpriteAsset* asset = resources.GetSprite(spriteId_);
    if (asset == nullptr || !spriteAnimator_.Init(*asset, animationName_))
    {
        return false;
    }

    if (alternateSpriteId_.empty())
    {
        return true;
    }

    const SpriteAsset* alternateAsset = resources.GetSprite(alternateSpriteId_);
    return std::isfinite(crossfadeSeconds_)
        && crossfadeSeconds_ > 0.0F
        && std::isfinite(crossfadeStrength_)
        && crossfadeStrength_ >= 0.0F
        && crossfadeStrength_ <= 1.0F
        && alternateAsset != nullptr
        && alternateSpriteAnimator_.Init(*alternateAsset, animationName_);
}

void ScrollingSurface::Update(float deltaTime)
{
    scrollX_ += speedX_ * deltaTime;
    scrollY_ += speedY_ * deltaTime;
    spriteAnimator_.Update(deltaTime);

    if (!alternateSpriteId_.empty())
    {
        const float elapsed = std::max(0.0F, deltaTime);
        crossfadeElapsedSeconds_ = std::fmod(
            crossfadeElapsedSeconds_ + elapsed,
            crossfadeSeconds_ * 2.0F);
        alternateSpriteAnimator_.Update(deltaTime);

        const float phase = crossfadeElapsedSeconds_ / crossfadeSeconds_;
        const float blend = phase <= 1.0F ? phase : 2.0F - phase;
        const float easedBlend = blend * blend * (3.0F - 2.0F * blend);

        // Strength controls the swing around an even blend. For example,
        // 0.25 keeps the two images between 37.5% and 62.5% contribution.
        crossfadeOpacity_ = 0.5F
            + (easedBlend - 0.5F) * crossfadeStrength_;
    }

    if (fadingOut_)
    {
        fadeElapsedSeconds_ += std::max(0.0F, deltaTime);
        opacity_ = 1.0F - std::clamp(
            fadeElapsedSeconds_ / fadeDurationSeconds_,
            0.0F,
            1.0F);
    }
}

void ScrollingSurface::Render(
    const Graphics& graphics,
    const Camera& camera)
{
    spriteAnimator_.RenderTiled(
        graphics,
        x_ + scrollX_ - static_cast<float>(camera.GetX()),
        y_ + scrollY_ - static_cast<float>(camera.GetY()),
        opacity_);

    if (!alternateSpriteId_.empty())
    {
        alternateSpriteAnimator_.RenderTiled(
            graphics,
            x_ + scrollX_ - static_cast<float>(camera.GetX()),
            y_ + scrollY_ - static_cast<float>(camera.GetY()),
            opacity_ * crossfadeOpacity_);
    }
}

bool ScrollingSurface::BeginFadeOut(float durationSeconds) noexcept
{
    if (fadingOut_
        || !std::isfinite(durationSeconds)
        || durationSeconds <= 0.0F)
    {
        return false;
    }

    fadeDurationSeconds_ = durationSeconds;
    fadeElapsedSeconds_ = 0.0F;
    opacity_ = 1.0F;
    fadingOut_ = true;
    return true;
}

bool ScrollingSurface::IsFadeOutComplete() const noexcept
{
    return fadingOut_ && fadeElapsedSeconds_ >= fadeDurationSeconds_;
}
