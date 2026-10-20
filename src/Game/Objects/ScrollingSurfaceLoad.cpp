#include "GameObjectLoad.h"

#include "ScrollingSurface.h"

#include <yaml-cpp/yaml.h>

#include <memory>

std::unique_ptr<BaseGameObject> GameObjectLoad::ScrollingSurface(
    const YAML::Node& node)
{
    const YAML::Node animation = node["animation"];
    const YAML::Node x = node["x"];
    const YAML::Node y = node["y"];
    const YAML::Node speedX = node["speedX"];
    const YAML::Node speedY = node["speedY"];
    const YAML::Node alternateSprite = node["alternateSprite"];
    const YAML::Node crossfadeSeconds = node["crossfadeSeconds"];
    const YAML::Node crossfadeStrength = node["crossfadeStrength"];

    return std::make_unique<::ScrollingSurface>(
        node["id"].as<std::string>(),
        node["sprite"].as<std::string>(),
        animation && animation.IsScalar()
            ? animation.as<std::string>()
            : "idle",
        x && x.IsScalar() ? x.as<float>() : 0.0F,
        y && y.IsScalar() ? y.as<float>() : 0.0F,
        speedX && speedX.IsScalar() ? speedX.as<float>() : 0.0F,
        speedY && speedY.IsScalar() ? speedY.as<float>() : 0.0F,
        alternateSprite && alternateSprite.IsScalar()
            ? alternateSprite.as<std::string>()
            : "",
        crossfadeSeconds && crossfadeSeconds.IsScalar()
            ? crossfadeSeconds.as<float>()
            : 0.0F,
        crossfadeStrength && crossfadeStrength.IsScalar()
            ? crossfadeStrength.as<float>()
            : 1.0F);
}
