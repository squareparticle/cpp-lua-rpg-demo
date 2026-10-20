#pragma once

#include "../../Framework/Objects/BaseGameObject.h"

#include <string>

class SpriteObject final : public BaseGameObject
{
public:
    SpriteObject(
        std::string id,
        std::string spriteId,
        std::string animationName,
        float x,
        float y,
        float scale,
        float sortPointY);

    bool Init(ResourceManager& resources) override;
    void Update(float deltaTime) override;
    void Render(const Graphics& graphics, const Camera& camera) override;
    float GetRenderSortKey() const noexcept override;
    bool GetWorldPosition(float& x, float& y) const noexcept override;
    bool SetWorldPosition(float x, float y) noexcept override;
    bool BeginFadeOut(float durationSeconds) noexcept override;
    bool IsFadeOutComplete() const noexcept override;

private:
    std::string spriteId_;
    std::string animationName_;
    float x_ = 0.0F;
    float y_ = 0.0F;
    float scale_ = 1.0F;
    float sortPointY_ = 0.0F;
    float opacity_ = 1.0F;
    float fadeDurationSeconds_ = 0.0F;
    float fadeElapsedSeconds_ = 0.0F;
};
