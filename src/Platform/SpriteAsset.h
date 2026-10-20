#pragma once

#include "ImageAsset.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct ID2D1RenderTarget;

struct SpriteAnimation
{
    float framesPerSecond = 0.0F;
    std::vector<std::uint32_t> cells;
};

class SpriteAsset
{
public:
    bool Load(ID2D1RenderTarget* renderTarget, const std::string& filename);
    bool ReloadImage(ID2D1RenderTarget* renderTarget);
    void ReleaseImage() noexcept;

    const ImageAsset& GetImage() const noexcept;
    std::uint32_t GetRows() const noexcept;
    std::uint32_t GetColumns() const noexcept;
    const SpriteAnimation* GetAnimation(const std::string& name) const noexcept;

private:
    ImageAsset image_;
    std::wstring imageFilename_;
    std::uint32_t rows_ = 0;
    std::uint32_t columns_ = 0;
    std::unordered_map<std::string, SpriteAnimation> animations_;
};
