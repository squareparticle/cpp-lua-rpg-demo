#include "ResourceManager.h"

#include <filesystem>
#include <memory>
#include <utility>

ResourceManager::ResourceManager() = default;
ResourceManager::~ResourceManager() = default;

bool ResourceManager::LoadImage(
    ID2D1RenderTarget* renderTarget,
    const std::string& id,
    const std::string& filename,
    ResourceScope scope)
{
    if (id.empty() || filename.empty())
    {
        return false;
    }

    ImageMap& images = scope == ResourceScope::Game ? gameImages_ : sceneImages_;
    if (images.contains(id))
    {
        return true;
    }

    const std::wstring wideFilename = std::filesystem::path(filename).wstring();
    auto image = std::make_unique<ImageAsset>();
    if (!image->Load(renderTarget, wideFilename))
    {
        return false;
    }

    images.emplace(
        id,
        ImageResource{wideFilename, std::move(image)});
    return true;
}

bool ResourceManager::LoadSprite(
    ID2D1RenderTarget* renderTarget,
    const std::string& id,
    const std::string& filename,
    ResourceScope scope)
{
    if (id.empty() || filename.empty())
    {
        return false;
    }

    SpriteMap& sprites = scope == ResourceScope::Game ? gameSprites_ : sceneSprites_;
    if (sprites.contains(id))
    {
        return true;
    }

    auto sprite = std::make_unique<SpriteAsset>();
    if (!sprite->Load(renderTarget, filename))
    {
        return false;
    }

    sprites.emplace(id, std::move(sprite));
    return true;
}

bool ResourceManager::LoadCollisionMask(
    const std::string& id,
    const std::string& filename,
    ResourceScope scope)
{
    if (id.empty() || filename.empty())
    {
        return false;
    }

    CollisionMaskMap& masks = scope == ResourceScope::Game
        ? gameCollisionMasks_
        : sceneCollisionMasks_;
    if (masks.contains(id))
    {
        return true;
    }

    auto mask = std::make_unique<CollisionMask>();
    if (!mask->Load(std::filesystem::path(filename).wstring()))
    {
        return false;
    }

    masks.emplace(id, std::move(mask));
    return true;
}

bool ResourceManager::LoadSound(
    const std::string& id,
    const std::string& filename,
    ResourceScope scope)
{
    if (id.empty() || filename.empty())
    {
        return false;
    }

    SoundMap& sounds = scope == ResourceScope::Game
        ? gameSounds_
        : sceneSounds_;
    if (sounds.contains(id))
    {
        return true;
    }

    auto sound = std::make_unique<SoundAsset>();
    if (!sound->Load(std::filesystem::path(filename).wstring()))
    {
        return false;
    }

    sounds.emplace(id, std::move(sound));
    return true;
}

const ImageAsset* ResourceManager::GetImage(const std::string& id) const noexcept
{
    if (const auto sceneImage = sceneImages_.find(id);
        sceneImage != sceneImages_.end())
    {
        return sceneImage->second.asset.get();
    }

    if (const auto gameImage = gameImages_.find(id);
        gameImage != gameImages_.end())
    {
        return gameImage->second.asset.get();
    }

    return nullptr;
}

const SpriteAsset* ResourceManager::GetSprite(const std::string& id) const noexcept
{
    if (const auto sceneSprite = sceneSprites_.find(id);
        sceneSprite != sceneSprites_.end())
    {
        return sceneSprite->second.get();
    }

    if (const auto gameSprite = gameSprites_.find(id);
        gameSprite != gameSprites_.end())
    {
        return gameSprite->second.get();
    }

    return nullptr;
}

const CollisionMask* ResourceManager::GetCollisionMask(
    const std::string& id) const noexcept
{
    if (const auto sceneMask = sceneCollisionMasks_.find(id);
        sceneMask != sceneCollisionMasks_.end())
    {
        return sceneMask->second.get();
    }

    if (const auto gameMask = gameCollisionMasks_.find(id);
        gameMask != gameCollisionMasks_.end())
    {
        return gameMask->second.get();
    }

    return nullptr;
}

const SoundAsset* ResourceManager::GetSound(
    const std::string& id) const noexcept
{
    if (const auto sceneSound = sceneSounds_.find(id);
        sceneSound != sceneSounds_.end())
    {
        return sceneSound->second.get();
    }

    if (const auto gameSound = gameSounds_.find(id);
        gameSound != gameSounds_.end())
    {
        return gameSound->second.get();
    }

    return nullptr;
}

bool ResourceManager::ReloadAssets(ID2D1RenderTarget* renderTarget)
{
    const auto reloadImages = [renderTarget](ImageMap& images)
    {
        for (auto& [id, resource] : images)
        {
            static_cast<void>(id);
            if (!resource.asset->Load(renderTarget, resource.filename))
            {
                return false;
            }
        }

        return true;
    };

    const auto reloadSprites = [renderTarget](SpriteMap& sprites)
    {
        for (auto& [id, sprite] : sprites)
        {
            static_cast<void>(id);
            if (!sprite->ReloadImage(renderTarget))
            {
                return false;
            }
        }

        return true;
    };

    return reloadImages(gameImages_)
        && reloadImages(sceneImages_)
        && reloadSprites(gameSprites_)
        && reloadSprites(sceneSprites_);
}

void ResourceManager::ReleaseAssets() noexcept
{
    const auto releaseImages = [](ImageMap& images)
    {
        for (auto& [id, resource] : images)
        {
            static_cast<void>(id);
            resource.asset->Reset();
        }
    };

    const auto releaseSprites = [](SpriteMap& sprites)
    {
        for (auto& [id, sprite] : sprites)
        {
            static_cast<void>(id);
            sprite->ReleaseImage();
        }
    };

    releaseImages(gameImages_);
    releaseImages(sceneImages_);
    releaseSprites(gameSprites_);
    releaseSprites(sceneSprites_);
}

void ResourceManager::ClearScene()
{
    sceneImages_.clear();
    sceneSprites_.clear();
    sceneCollisionMasks_.clear();
    sceneSounds_.clear();
}

void ResourceManager::Clear()
{
    sceneImages_.clear();
    gameImages_.clear();
    sceneSprites_.clear();
    gameSprites_.clear();
    sceneCollisionMasks_.clear();
    gameCollisionMasks_.clear();
    sceneSounds_.clear();
    gameSounds_.clear();
}
