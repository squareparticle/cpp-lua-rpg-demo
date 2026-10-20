#pragma once

#include "../../Framework/Objects/BaseGameObject.h"

#include <string>

class ScrollingSurface final : public BaseGameObject
{
public:
    ScrollingSurface(
        std::string id,
        std::string spriteId,
        std::string animationName,
        float x,
        float y,
        float speedX,
        float speedY,
        std::string alternateSpriteId,
        float crossfadeSeconds,
        float crossfadeStrength);

    bool Init(ResourceManager& resources) override;
    void Update(float deltaTime) override;
    void Render(const Graphics& graphics, const Camera& camera) override;
    bool BeginFadeOut(float durationSeconds) noexcept override;
    bool IsFadeOutComplete() const noexcept override;

private:
    std::string spriteId_;
    std::string animationName_;
    float x_ = 0.0F;
    float y_ = 0.0F;
    float scrollX_ = 0.0F;
    float scrollY_ = 0.0F;
    float speedX_ = 0.0F;
    float speedY_ = 0.0F;
    SpriteAnimator alternateSpriteAnimator_;
    std::string alternateSpriteId_;
    float crossfadeSeconds_ = 0.0F;
    float crossfadeStrength_ = 1.0F;
    float crossfadeElapsedSeconds_ = 0.0F;
    float crossfadeOpacity_ = 0.0F;
    float opacity_ = 1.0F;
    float fadeDurationSeconds_ = 0.0F;
    float fadeElapsedSeconds_ = 0.0F;
    bool fadingOut_ = false;
};
