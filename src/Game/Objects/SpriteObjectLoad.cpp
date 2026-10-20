#include "GameObjectLoad.h"

#include "SpriteObject.h"

#include <yaml-cpp/yaml.h>

#include <cmath>
#include <memory>

std::unique_ptr<BaseGameObject> GameObjectLoad::SpriteObject(
    const YAML::Node& node)
{
    const YAML::Node animation = node["animation"];
    const YAML::Node x = node["x"];
    const YAML::Node y = node["y"];
    const YAML::Node scale = node["scale"];
    const YAML::Node sortPointY = node["sortPointY"];

    if (!animation.IsScalar()
        || !x.IsScalar()
        || !y.IsScalar()
        || (scale && !scale.IsScalar())
        || (sortPointY && !sortPointY.IsScalar()))
    {
        return nullptr;
    }

    const float objectX = x.as<float>();
    const float objectY = y.as<float>();
    const float objectScale = scale ? scale.as<float>() : 1.0F;
    const float objectSortPointY = sortPointY
        ? sortPointY.as<float>()
        : 0.0F;
    if (!std::isfinite(objectX)
        || !std::isfinite(objectY)
        || !std::isfinite(objectScale)
        || !std::isfinite(objectSortPointY)
        || objectScale <= 0.0F)
    {
        return nullptr;
    }

    return std::make_unique<::SpriteObject>(
        node["id"].as<std::string>(),
        node["sprite"].as<std::string>(),
        animation.as<std::string>(),
        objectX,
        objectY,
        objectScale,
        objectSortPointY);
}
