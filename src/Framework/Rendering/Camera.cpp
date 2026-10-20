#include "Camera.h"

#include <algorithm>
#include <cmath>

bool Camera::Init()
{
    x_ = hasWindow_ ? windowX_ : 0.0F;
    y_ = hasWindow_ ? windowY_ : 0.0F;
    return true;
}

bool Camera::SetWindow(
    float x,
    float y,
    float width,
    float height,
    float viewportWidth,
    float viewportHeight) noexcept
{
    if (width <= 0.0F
        || height <= 0.0F
        || viewportWidth <= 0.0F
        || viewportHeight <= 0.0F)
    {
        return false;
    }

    windowX_ = x;
    windowY_ = y;
    windowWidth_ = width;
    windowHeight_ = height;
    viewportWidth_ = viewportWidth;
    viewportHeight_ = viewportHeight;
    hasWindow_ = true;
    return true;
}

void Camera::Follow(float targetX, float targetY) noexcept
{
    if (!hasWindow_)
    {
        return;
    }

    const float maximumX = windowX_
        + std::max(windowWidth_ - viewportWidth_, 0.0F);
    const float maximumY = windowY_
        + std::max(windowHeight_ - viewportHeight_, 0.0F);

    // Center the target whenever possible, then clamp the camera's viewport
    // inside its world window. At an edge the player keeps moving on screen
    // while the camera remains pinned to the boundary.
    x_ = std::clamp(
        targetX - viewportWidth_ * 0.5F,
        windowX_,
        maximumX);
    y_ = std::clamp(
        targetY - viewportHeight_ * 0.5F,
        windowY_,
        maximumY);
}

void Camera::Focus(float targetX, float targetY) noexcept
{
    x_ = targetX - viewportWidth_ * 0.5F;
    y_ = targetY - viewportHeight_ * 0.5F;
}

bool Camera::PanTo(float targetX, float targetY, float durationSeconds) noexcept
{
    if (!hasWindow_ || !std::isfinite(targetX) || !std::isfinite(targetY)
        || !std::isfinite(durationSeconds) || durationSeconds <= 0.0F)
    {
        return false;
    }
    panStartX_ = x_;
    panStartY_ = y_;
    panEndX_ = targetX - viewportWidth_ * 0.5F;
    panEndY_ = targetY - viewportHeight_ * 0.5F;
    panElapsed_ = 0.0F;
    panDuration_ = durationSeconds;
    return true;
}

void Camera::Update(float deltaTime) noexcept
{
    if (panDuration_ <= 0.0F || IsPanComplete())
    {
        return;
    }
    panElapsed_ += std::max(deltaTime, 0.0F);
    const float t = std::clamp(panElapsed_ / panDuration_, 0.0F, 1.0F);
    const float blend = t * t * (3.0F - 2.0F * t);
    x_ = panStartX_ + (panEndX_ - panStartX_) * blend;
    y_ = panStartY_ + (panEndY_ - panStartY_) * blend;
}

bool Camera::IsPanComplete() const noexcept
{
    return panDuration_ > 0.0F && panElapsed_ >= panDuration_;
}

int Camera::GetX() const noexcept
{
    return static_cast<int>(std::floor(x_));
}

int Camera::GetY() const noexcept
{
    return static_cast<int>(std::floor(y_));
}
