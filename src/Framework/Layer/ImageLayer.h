#pragma once

#include "BaseLayer.h"
#include "../Rendering/SpriteAnimator.h"

#include <string>
#include <vector>

class ImageAsset;

class ImageLayer final : public BaseLayer
{
public:
    void AddImage(std::string imageId, float x, float y);
    void AddSprite(
        std::string spriteId,
        std::string animationName,
        float x,
        float y,
        float scale);

    bool Init(ResourceManager& resources) override;
    void Update(float deltaTime) override;
    void Render(const Graphics& graphics, const Camera& camera) override;

private:
    struct ImagePlacement
    {
        std::string imageId;
        float x = 0.0F;
        float y = 0.0F;
        const ImageAsset* image = nullptr;
    };

    struct SpritePlacement
    {
        std::string spriteId;
        std::string animationName;
        float x = 0.0F;
        float y = 0.0F;
        float scale = 1.0F;
        SpriteAnimator spriteAnimator;
    };

    std::vector<ImagePlacement> images_;
    std::vector<SpritePlacement> sprites_;
};
