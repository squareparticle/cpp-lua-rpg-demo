#pragma once

#include <memory>
#include <optional>
#include <string>

struct ID2D1RenderTarget;
class BaseScene;
class BaseGameObject;
class ResourceManager;
namespace YAML
{
class Node;
}

class SceneBuilder
{
public:
    static bool BuildScene(
        const std::string& filename,
        ID2D1RenderTarget* renderTarget,
        ResourceManager& resourceManager,
        BaseScene& scene);

    static std::unique_ptr<BaseGameObject> CreateGameObject(
        const YAML::Node& node);
    static std::unique_ptr<BaseGameObject> LoadGameObjectDefinition(
        const std::string& filename,
        std::string& targetLayerId,
        std::optional<std::string> idOverride = std::nullopt,
        std::optional<float> xOverride = std::nullopt,
        std::optional<float> yOverride = std::nullopt);
};
