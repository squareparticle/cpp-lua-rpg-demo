#pragma once

#include "../Collision/CollisionMask.h"
#include "../../Platform/ImageAsset.h"
#include "../../Platform/SoundAsset.h"
#include "../../Platform/SpriteAsset.h"

#include <memory>
#include <string>
#include <unordered_map>

struct ID2D1RenderTarget;

enum class ResourceScope
{
    Game,
    Scene
};

class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    bool LoadImage(
        ID2D1RenderTarget* renderTarget,
        const std::string& id,
        const std::string& filename,
        ResourceScope scope);
    bool LoadSprite(
        ID2D1RenderTarget* renderTarget,
        const std::string& id,
        const std::string& filename,
        ResourceScope scope);
    bool LoadCollisionMask(
        const std::string& id,
        const std::string& filename,
        ResourceScope scope);
    bool LoadSound(
        const std::string& id,
        const std::string& filename,
        ResourceScope scope);
    const ImageAsset* GetImage(const std::string& id) const noexcept;
    const SpriteAsset* GetSprite(const std::string& id) const noexcept;
    const CollisionMask* GetCollisionMask(
        const std::string& id) const noexcept;
    const SoundAsset* GetSound(const std::string& id) const noexcept;
    bool ReloadAssets(ID2D1RenderTarget* renderTarget);
    void ReleaseAssets() noexcept;
    void ClearScene();
    void Clear();

private:
    struct ImageResource
    {
        std::wstring filename;
        std::unique_ptr<ImageAsset> asset;
    };

    using ImageMap = std::unordered_map<std::string, ImageResource>;
    using SpriteMap = std::unordered_map<std::string, std::unique_ptr<SpriteAsset>>;
    using CollisionMaskMap =
        std::unordered_map<std::string, std::unique_ptr<CollisionMask>>;
    using SoundMap =
        std::unordered_map<std::string, std::unique_ptr<SoundAsset>>;

    ImageMap gameImages_;
    ImageMap sceneImages_;
    SpriteMap gameSprites_;
    SpriteMap sceneSprites_;
    CollisionMaskMap gameCollisionMasks_;
    CollisionMaskMap sceneCollisionMasks_;
    SoundMap gameSounds_;
    SoundMap sceneSounds_;
};
