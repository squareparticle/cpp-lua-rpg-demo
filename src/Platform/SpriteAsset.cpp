#include "SpriteAsset.h"

#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <utility>

bool SpriteAsset::Load(
    ID2D1RenderTarget* renderTarget,
    const std::string& filename)
{
    try
    {
        const YAML::Node root = YAML::LoadFile(filename);
        const YAML::Node sheet = root["sheet"];
        const YAML::Node animations = root["animations"];

        if (!root["image"].IsScalar()
            || !sheet["rows"].IsScalar()
            || !sheet["columns"].IsScalar()
            || !animations.IsMap())
        {
            return false;
        }

        const auto rows = sheet["rows"].as<std::uint32_t>();
        const auto columns = sheet["columns"].as<std::uint32_t>();
        if (rows == 0 || columns == 0)
        {
            return false;
        }

        std::unordered_map<std::string, SpriteAnimation> parsedAnimations;
        for (const auto& entry : animations)
        {
            const std::string name = entry.first.as<std::string>();
            const YAML::Node animationNode = entry.second;
            const YAML::Node cellsNode = animationNode["cells"];

            if (name.empty()
                || !animationNode["fps"].IsScalar()
                || !cellsNode.IsSequence())
            {
                return false;
            }

            SpriteAnimation animation;
            animation.framesPerSecond = animationNode["fps"].as<float>();
            if (animation.framesPerSecond <= 0.0F || cellsNode.size() == 0)
            {
                return false;
            }

            for (const YAML::Node& cellNode : cellsNode)
            {
                const auto cell = cellNode.as<std::uint32_t>();
                if (cell >= rows * columns)
                {
                    return false;
                }

                animation.cells.push_back(cell);
            }

            parsedAnimations.emplace(name, std::move(animation));
        }

        const std::wstring imageFilename =
            std::filesystem::path(root["image"].as<std::string>()).wstring();
        if (!image_.Load(renderTarget, imageFilename))
        {
            return false;
        }

        imageFilename_ = imageFilename;
        rows_ = rows;
        columns_ = columns;
        animations_ = std::move(parsedAnimations);
        return true;
    }
    catch (const YAML::Exception&)
    {
        return false;
    }
}

bool SpriteAsset::ReloadImage(ID2D1RenderTarget* renderTarget)
{
    return image_.Load(renderTarget, imageFilename_);
}

void SpriteAsset::ReleaseImage() noexcept
{
    image_.Reset();
}

const ImageAsset& SpriteAsset::GetImage() const noexcept
{
    return image_;
}

std::uint32_t SpriteAsset::GetRows() const noexcept
{
    return rows_;
}

std::uint32_t SpriteAsset::GetColumns() const noexcept
{
    return columns_;
}

const SpriteAnimation* SpriteAsset::GetAnimation(
    const std::string& name) const noexcept
{
    const auto animation = animations_.find(name);
    return animation == animations_.end() ? nullptr : &animation->second;
}
