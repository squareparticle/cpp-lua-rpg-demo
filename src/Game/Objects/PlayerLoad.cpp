#include "GameObjectLoad.h"

#include "Player.h"
#include "../../Platform/DebugLog.h"

#include <yaml-cpp/yaml.h>

#include <memory>
#include <utility>
#include <vector>

std::unique_ptr<BaseGameObject> GameObjectLoad::Player(
    const YAML::Node& node)
{
    const YAML::Node animation = node["animation"];
    const YAML::Node x = node["x"];
    const YAML::Node y = node["y"];
    const YAML::Node speed = node["speed"];
    const YAML::Node scale = node["scale"];
    const YAML::Node sortPointY = node["sortPointY"];
    const YAML::Node cameraPoint = node["cameraPoint"];
    const YAML::Node interactionPoint = node["interactionPoint"];
    const YAML::Node collisionProbesNode = node["collisionProbes"];

    if (!sortPointY.IsScalar()
        || !cameraPoint.IsMap()
        || !cameraPoint["x"].IsScalar()
        || !cameraPoint["y"].IsScalar()
        || !interactionPoint.IsMap()
        || !interactionPoint["x"].IsScalar()
        || !interactionPoint["y"].IsScalar())
    {
        WriteDebugMessage(
            "Player is missing a valid sortPointY, cameraPoint, "
            "or interactionPoint.");
        return nullptr;
    }

    const float playerSpeed = speed && speed.IsScalar()
        ? speed.as<float>()
        : 0.0F;
    const float playerScale = scale && scale.IsScalar()
        ? scale.as<float>()
        : 1.0F;
    if (playerSpeed < 0.0F || playerScale <= 0.0F)
    {
        return nullptr;
    }

    std::vector<CollisionProbe> collisionProbes;
    if (collisionProbesNode)
    {
        if (!collisionProbesNode.IsSequence())
        {
            return nullptr;
        }

        for (const YAML::Node& probe : collisionProbesNode)
        {
            if (!probe.IsMap()
                || !probe["x"].IsScalar()
                || !probe["y"].IsScalar())
            {
                return nullptr;
            }

            collisionProbes.push_back(CollisionProbe{
                probe["x"].as<float>(),
                probe["y"].as<float>()});
        }
    }

    return std::make_unique<::Player>(
        node["id"].as<std::string>(),
        node["sprite"].as<std::string>(),
        animation && animation.IsScalar()
            ? animation.as<std::string>()
            : "idleDown",
        x && x.IsScalar() ? x.as<float>() : 0.0F,
        y && y.IsScalar() ? y.as<float>() : 0.0F,
        playerSpeed,
        playerScale,
        sortPointY.as<float>(),
        cameraPoint["x"].as<float>(),
        cameraPoint["y"].as<float>(),
        interactionPoint["x"].as<float>(),
        interactionPoint["y"].as<float>(),
        std::move(collisionProbes));
}
