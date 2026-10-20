#include "SpriteAnimator.h"

#include "../../Platform/Graphics.h"
#include "../../Platform/SpriteAsset.h"

bool SpriteAnimator::Init(const SpriteAsset& asset, const std::string& animationName)
{
    asset_ = &asset;
    return PlayAnimation(animationName);
}

bool SpriteAnimator::PlayAnimation(
    const std::string& animationName,
    bool loop)
{
    if (asset_ == nullptr)
    {
        return false;
    }

    const SpriteAnimation* animation = asset_->GetAnimation(animationName);
    if (animation == nullptr)
    {
        return false;
    }

    animation_ = animation;
    frameIndex_ = 0;
    frameElapsedSeconds_ = 0.0F;
    loop_ = loop;
    animationFinished_ = false;
    return true;
}

void SpriteAnimator::Update(float deltaTime)
{
    if (animation_ == nullptr
        || animation_->cells.empty()
        || animationFinished_)
    {
        return;
    }

    const float frameDuration = 1.0F / animation_->framesPerSecond;
    frameElapsedSeconds_ += deltaTime;
    const auto elapsedFrames = static_cast<std::size_t>(
        frameElapsedSeconds_ / frameDuration);
    if (elapsedFrames == 0)
    {
        return;
    }

    // One-shot battle animations hold their final frame and report completion;
    // ordinary world animations keep the original looping behavior.
    if (!loop_ && frameIndex_ + elapsedFrames >= animation_->cells.size())
    {
        frameIndex_ = animation_->cells.size() - 1;
        frameElapsedSeconds_ = 0.0F;
        animationFinished_ = true;
        return;
    }

    frameIndex_ = loop_
        ? (frameIndex_ + elapsedFrames) % animation_->cells.size()
        : frameIndex_ + elapsedFrames;
    frameElapsedSeconds_ -= frameDuration * static_cast<float>(elapsedFrames);
}

std::size_t SpriteAnimator::GetFrameIndex() const noexcept
{
    return frameIndex_;
}

bool SpriteAnimator::IsAnimationFinished() const noexcept
{
    return animationFinished_;
}

void SpriteAnimator::Render(
    const Graphics& graphics,
    float x,
    float y,
    float scale,
    float opacity) const
{
    if (asset_ == nullptr || animation_ == nullptr || animation_->cells.empty())
    {
        return;
    }

    graphics.DrawImageFrame(
        asset_->GetImage(),
        asset_->GetRows(),
        asset_->GetColumns(),
        animation_->cells[frameIndex_],
        x,
        y,
        scale,
        opacity);
}

void SpriteAnimator::RenderTiled(
    const Graphics& graphics,
    float offsetX,
    float offsetY,
    float opacity) const
{
    if (asset_ == nullptr || animation_ == nullptr || animation_->cells.empty())
    {
        return;
    }

    graphics.DrawImageFrameTiled(
        asset_->GetImage(),
        asset_->GetRows(),
        asset_->GetColumns(),
        animation_->cells[frameIndex_],
        offsetX,
        offsetY,
        opacity);
}
