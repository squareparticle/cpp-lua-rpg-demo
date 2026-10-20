#include "ImageLayer.h"

#include "../Rendering/Camera.h"
#include "../../Platform/Graphics.h"
#include "../Resources/ResourceManager.h"

#include <utility>

void ImageLayer::AddImage(std::string imageId, float x, float y)
{
    images_.push_back(ImagePlacement{std::move(imageId), x, y, nullptr});
}

void ImageLayer::AddSprite(
    std::string spriteId,
    std::string animationName,
    float x,
    float y,
    float scale)
{
    sprites_.push_back(SpritePlacement{
        std::move(spriteId),
        std::move(animationName),
        x,
        y,
        scale,
        {}});
}

bool ImageLayer::Init(ResourceManager& resources)
{
    for (auto& placement : images_)
    {
        placement.image = resources.GetImage(placement.imageId);
        if (placement.image == nullptr)
        {
            return false;
        }
    }

    for (auto& placement : sprites_)
    {
        const SpriteAsset* asset = resources.GetSprite(placement.spriteId);
        if (asset == nullptr
            || !placement.spriteAnimator.Init(*asset, placement.animationName))
        {
            return false;
        }
    }

    return true;
}

void ImageLayer::Update(float deltaTime)
{
    for (auto& placement : sprites_)
    {
        placement.spriteAnimator.Update(deltaTime);
    }
}

void ImageLayer::Render(const Graphics& graphics, const Camera& camera)
{
    for (const auto& placement : images_)
    {
        if (placement.image != nullptr)
        {
            graphics.DrawImage(
                *placement.image,
                placement.x - static_cast<float>(camera.GetX()),
                placement.y - static_cast<float>(camera.GetY()));
        }
    }


    for (const auto& placement : sprites_)
    {
        placement.spriteAnimator.Render(
            graphics,
            placement.x - static_cast<float>(camera.GetX()),
            placement.y - static_cast<float>(camera.GetY()),
            placement.scale);
    }
}
