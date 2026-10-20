#pragma once

#include <cstddef>
#include <string>

class Graphics;
class SpriteAsset;
struct SpriteAnimation;

class SpriteAnimator
{
public:
    bool Init(const SpriteAsset& asset, const std::string& animationName);
    bool PlayAnimation(
        const std::string& animationName,
        bool loop = true);
    void Update(float deltaTime);
    std::size_t GetFrameIndex() const noexcept;
    bool IsAnimationFinished() const noexcept;
    void Render(
        const Graphics& graphics,
        float x,
        float y,
        float scale = 1.0F,
        float opacity = 1.0F) const;
    void RenderTiled(
        const Graphics& graphics,
        float offsetX,
        float offsetY,
        float opacity = 1.0F) const;

private:
    const SpriteAsset* asset_ = nullptr;
    const SpriteAnimation* animation_ = nullptr;
    std::size_t frameIndex_ = 0;
    float frameElapsedSeconds_ = 0.0F;
    bool loop_ = true;
    bool animationFinished_ = false;
};
