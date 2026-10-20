#pragma once

class Camera
{
public:
    bool Init();
    bool SetWindow(
        float x,
        float y,
        float width,
        float height,
        float viewportWidth,
        float viewportHeight) noexcept;
    void Follow(float targetX, float targetY) noexcept;
    void Focus(float targetX, float targetY) noexcept;
    bool PanTo(float targetX, float targetY, float durationSeconds) noexcept;
    void Update(float deltaTime) noexcept;
    bool IsPanComplete() const noexcept;

    int GetX() const noexcept;
    int GetY() const noexcept;

private:
    float x_ = 0.0F;
    float y_ = 0.0F;
    float windowX_ = 0.0F;
    float windowY_ = 0.0F;
    float windowWidth_ = 0.0F;
    float windowHeight_ = 0.0F;
    float viewportWidth_ = 0.0F;
    float viewportHeight_ = 0.0F;
    bool hasWindow_ = false;
    float panStartX_ = 0.0F;
    float panStartY_ = 0.0F;
    float panEndX_ = 0.0F;
    float panEndY_ = 0.0F;
    float panDuration_ = 0.0F;
    float panElapsed_ = 0.0F;
};
