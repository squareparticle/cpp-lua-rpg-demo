#include "SceneBuilder.h"

#include "../../Framework/Objects/BaseGameObject.h"
#include "../../Framework/Scenes/BaseScene.h"
#include "../World/GroundRegionMap.h"
#include "../../Platform/DebugLog.h"
#include "../Objects/GameObjectLoad.h"
#include "../Battle/BattleMoveLoad.h"
#include "../../Framework/Layer/ImageLayer.h"
#include "../../Framework/Layer/ObjectLayer.h"
#include "WorldScene.h"
#include "BattleArenaScene.h"
#include "../../Framework/Resources/ResourceManager.h"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
using ObjectTemplates = std::unordered_map<std::string, YAML::Node>;

bool ConfigureSceneMetadata(const YAML::Node& root, BaseScene& scene)
{
    const YAML::Node definition = root["scene"];
    if (!definition)
    {
        return true;
    }
    if (!definition.IsMap()
        || !definition["id"].IsScalar()
        || (definition["initScript"]
            && !definition["initScript"].IsScalar())
        || (definition["music"] && !definition["music"].IsScalar()))
    {
        WriteDebugMessage("Scene metadata is malformed.");
        return false;
    }

    const std::string id = definition["id"].as<std::string>();
    const std::string initScript = definition["initScript"]
        ? definition["initScript"].as<std::string>()
        : std::string{};
    const std::string musicId = definition["music"]
        ? definition["music"].as<std::string>()
        : std::string{};
    if (id.empty()
        || (definition["initScript"] && initScript.empty())
        || (definition["music"] && musicId.empty())
        || !scene.SetSceneMetadata(id, initScript, musicId))
    {
        WriteDebugMessage("Scene metadata was rejected: " + id);
        return false;
    }
    return true;
}

bool ConfigurePlayerSpawnPoints(const YAML::Node& root, BaseScene& scene)
{
    const YAML::Node spawnPoints = root["playerSpawnPoints"];
    if (!spawnPoints)
    {
        return true;
    }
    auto* worldScene = dynamic_cast<WorldScene*>(&scene);
    if (worldScene == nullptr || !spawnPoints.IsMap())
    {
        WriteDebugMessage(
            "playerSpawnPoints require a WorldScene and a map value.");
        return false;
    }

    for (const auto& spawnPointEntry : spawnPoints)
    {
        if (!spawnPointEntry.first.IsScalar()
            || !spawnPointEntry.second.IsMap())
        {
            return false;
        }

        const std::string id = spawnPointEntry.first.as<std::string>();
        const YAML::Node definition = spawnPointEntry.second;
        if (!definition["x"].IsScalar()
            || !definition["y"].IsScalar())
        {
            return false;
        }

        const float x = definition["x"].as<float>();
        const float y = definition["y"].as<float>();
        if (!std::isfinite(x)
            || !std::isfinite(y)
            || !worldScene->AddPlayerSpawnPoint(id, x, y))
        {
            WriteDebugMessage("World scene rejected player spawn point: " + id);
            return false;
        }
    }
    return true;
}

bool ConfigureWaypoints(const YAML::Node& root, BaseScene& scene)
{
    const YAML::Node points = root["waypoints"];
    if (!points)
    {
        return true;
    }
    auto* world = dynamic_cast<WorldScene*>(&scene);
    if (world == nullptr || !points.IsMap())
    {
        return false;
    }
    for (const auto& entry : points)
    {
        const auto point = entry.second;
        if (!entry.first.IsScalar() || !point.IsMap()
            || !point["x"].IsScalar() || !point["y"].IsScalar()
            || !world->AddWaypoint(entry.first.as<std::string>(),
                point["x"].as<float>(), point["y"].as<float>()))
        {
            return false;
        }
    }
    return true;
}

YAML::Node MergeObjectProperties(
    const YAML::Node& defaults,
    const YAML::Node& overrides,
    const std::string& metadataKey)
{
    YAML::Node result(YAML::NodeType::Map);

    // This function is used twice while resolving an object, producing the
    // intentional priority chain: .obj.yaml defaults < resource entry < layer
    // placement. Later maps replace whole properties such as interactionPoint.
    for (const auto& property : defaults)
    {
        result[property.first.as<std::string>()] = property.second;
    }

    for (const auto& property : overrides)
    {
        const std::string key = property.first.as<std::string>();
        if (key != metadataKey)
        {
            result[key] = property.second;
        }
    }

    return result;
}

bool LoadImages(
    const YAML::Node& root,
    ID2D1RenderTarget* renderTarget,
    ResourceManager& resourceManager)
{
    const YAML::Node images = root["resources"]["images"];
    if (!images || !images.IsSequence())
    {
        return false;
    }

    for (const YAML::Node& image : images)
    {
        if (!image["id"].IsScalar() || !image["path"].IsScalar())
        {
            return false;
        }

        if (!resourceManager.LoadImage(
                renderTarget,
                image["id"].as<std::string>(),
                image["path"].as<std::string>(),
                ResourceScope::Scene))
        {
            return false;
        }
    }

    return true;
}

bool LoadSprites(
    const YAML::Node& root,
    ID2D1RenderTarget* renderTarget,
    ResourceManager& resourceManager)
{
    const YAML::Node sprites = root["resources"]["sprites"];
    if (!sprites)
    {
        return true;
    }

    if (!sprites.IsSequence())
    {
        return false;
    }

    for (const YAML::Node& sprite : sprites)
    {
        if (!sprite["id"].IsScalar() || !sprite["path"].IsScalar())
        {
            return false;
        }

        if (!resourceManager.LoadSprite(
                renderTarget,
                sprite["id"].as<std::string>(),
                sprite["path"].as<std::string>(),
                ResourceScope::Scene))
        {
            return false;
        }
    }

    return true;
}

bool LoadCollisionMasks(
    const YAML::Node& root,
    ResourceManager& resourceManager)
{
    const YAML::Node collisionMasks = root["resources"]["collisionMasks"];
    if (!collisionMasks)
    {
        return true;
    }

    if (!collisionMasks.IsSequence())
    {
        return false;
    }

    for (const YAML::Node& collisionMask : collisionMasks)
    {
        if (!collisionMask["id"].IsScalar()
            || !collisionMask["path"].IsScalar())
        {
            return false;
        }

        if (!resourceManager.LoadCollisionMask(
                collisionMask["id"].as<std::string>(),
                collisionMask["path"].as<std::string>(),
                ResourceScope::Scene))
        {
            return false;
        }
    }

    return true;
}

bool LoadSounds(
    const YAML::Node& root,
    ResourceManager& resourceManager)
{
    const YAML::Node sounds = root["resources"]["sounds"];
    if (!sounds)
    {
        return true;
    }

    if (!sounds.IsSequence())
    {
        return false;
    }

    for (const YAML::Node& sound : sounds)
    {
        if (!sound["id"].IsScalar() || !sound["path"].IsScalar())
        {
            return false;
        }

        if (!resourceManager.LoadSound(
                sound["id"].as<std::string>(),
                sound["path"].as<std::string>(),
                ResourceScope::Scene))
        {
            return false;
        }
    }

    return true;
}

std::optional<std::uint32_t> ParseGroundColor(std::string_view text)
{
    if (text.size() != 6)
    {
        return std::nullopt;
    }

    std::uint32_t color = 0;
    const auto result = std::from_chars(
        text.data(),
        text.data() + text.size(),
        color,
        16);
    if (result.ec != std::errc{}
        || result.ptr != text.data() + text.size())
    {
        return std::nullopt;
    }
    return color;
}

std::optional<GroundParameterValue> ParseGroundParameterValue(
    const YAML::Node& node)
{
    if (!node.IsScalar())
    {
        return std::nullopt;
    }

    const std::string text = node.as<std::string>();
    const std::string tag = node.Tag();
    const bool explicitlyString = tag == "!"
        || tag == "tag:yaml.org,2002:str";
    if (!explicitlyString)
    {
        std::string lowercase = text;
        std::ranges::transform(
            lowercase,
            lowercase.begin(),
            [](unsigned char character)
            {
                return static_cast<char>(std::tolower(character));
            });
        if (lowercase == "true")
        {
            return GroundParameterValue{true};
        }
        if (lowercase == "false")
        {
            return GroundParameterValue{false};
        }

        double number = 0.0;
        const auto result = std::from_chars(
            text.data(),
            text.data() + text.size(),
            number,
            std::chars_format::general);
        if (result.ec == std::errc{}
            && result.ptr == text.data() + text.size()
            && std::isfinite(number))
        {
            return GroundParameterValue{number};
        }
    }

    return GroundParameterValue{text};
}

bool ReadGroundParameters(
    const YAML::Node& node,
    GroundParameterMap& parameters)
{
    if (!node)
    {
        return true;
    }
    if (!node.IsMap())
    {
        return false;
    }

    for (const auto& parameterEntry : node)
    {
        if (!parameterEntry.first.IsScalar())
        {
            return false;
        }

        const std::string name = parameterEntry.first.as<std::string>();
        const std::optional<GroundParameterValue> value =
            ParseGroundParameterValue(parameterEntry.second);
        if (name.empty()
            || !value.has_value()
            || !parameters.emplace(name, *value).second)
        {
            return false;
        }
    }
    return true;
}

bool ConfigureGroundRegions(const YAML::Node& root, BaseScene& scene)
{
    const YAML::Node regions = root["groundRegions"];
    if (!regions)
    {
        return true;
    }
    if (!regions.IsMap())
    {
        WriteDebugMessage("groundRegions must be a map.");
        return false;
    }

    // Ground colors drive walking-scene mechanics. Requiring WorldScene here
    // keeps menu-like scenes such as BattleArenaScene free of a meaningless
    // AddGroundRegion hook on their common BaseScene base.
    auto* worldScene = dynamic_cast<WorldScene*>(&scene);
    if (worldScene == nullptr)
    {
        WriteDebugMessage("groundRegions require a WorldScene.");
        return false;
    }

    for (const auto& regionEntry : regions)
    {
        if (!regionEntry.first.IsScalar() || !regionEntry.second.IsMap())
        {
            return false;
        }

        const std::string id = regionEntry.first.as<std::string>();
        const YAML::Node definition = regionEntry.second;
        const YAML::Node colorNode = definition["color"];
        const YAML::Node behaviorNode = definition["behavior"];
        const YAML::Node parametersNode = definition["parameters"];
        const YAML::Node hookNode = definition["hook"];
        if (id.empty()
            || !colorNode.IsScalar()
            || !behaviorNode.IsScalar()
            || (hookNode && !hookNode.IsScalar()))
        {
            return false;
        }

        const std::optional<std::uint32_t> color = ParseGroundColor(
            colorNode.as<std::string>());
        const std::optional<GroundBehavior> behavior = ParseGroundBehavior(
            behaviorNode.as<std::string>());
        if (!color.has_value()
            || *color == CollisionMask::BlockedColor
            || *color == CollisionMask::WalkableColor
            || !behavior.has_value())
        {
            WriteDebugMessage("Invalid ground region: " + id);
            return false;
        }

        GroundParameterMap parameters;
        if (!ReadGroundParameters(parametersNode, parameters))
        {
            WriteDebugMessage("Invalid ground-region parameters: " + id);
            return false;
        }
        if (!worldScene->AddGroundRegion(
                id,
                *color,
                *behavior,
                std::move(parameters),
                hookNode ? hookNode.as<std::string>() : std::string{}))
        {
            WriteDebugMessage("Scene rejected ground region: " + id);
            return false;
        }
    }
    return true;
}

bool ConfigureBattleMoves(const YAML::Node& root, BaseScene& scene)
{
    const YAML::Node moves = root["resources"]["battleMoves"];
    if (!moves)
    {
        return true;
    }

    auto* battleScene = dynamic_cast<BattleArenaScene*>(&scene);
    if (battleScene == nullptr || !moves.IsSequence())
    {
        WriteDebugMessage(
            "resources.battleMoves require a BattleArenaScene sequence.");
        return false;
    }

    for (const YAML::Node& moveNode : moves)
    {
        if (!moveNode.IsMap()
            || !moveNode["id"].IsScalar()
            || !moveNode["path"].IsScalar())
        {
            return false;
        }

        BattleMove move;
        const std::string id = moveNode["id"].as<std::string>();
        if (!LoadBattleMove(
                id,
                moveNode["path"].as<std::string>(),
                move)
            || !battleScene->AddBattleMove(std::move(move)))
        {
            WriteDebugMessage("Could not load battle move: " + id);
            return false;
        }
    }
    return true;
}

bool LoadObjects(
    const YAML::Node& root,
    ObjectTemplates& objectTemplates)
{
    const YAML::Node objects = root["resources"]["objects"];
    if (!objects)
    {
        return true;
    }

    if (!objects.IsSequence())
    {
        return false;
    }

    for (const YAML::Node& object : objects)
    {
        if (!object.IsMap()
            || !object["id"].IsScalar()
            || !object["path"].IsScalar())
        {
            return false;
        }

        const std::string id = object["id"].as<std::string>();
        if (id.empty() || objectTemplates.contains(id))
        {
            return false;
        }

        const YAML::Node objectRoot = YAML::LoadFile(
            object["path"].as<std::string>());
        const YAML::Node init = objectRoot["init"];
        if (!init || !init.IsMap())
        {
            return false;
        }

        YAML::Node objectTemplate = MergeObjectProperties(init, object, "path");
        objectTemplate["id"] = id;
        objectTemplates.emplace(id, std::move(objectTemplate));
    }

    return true;
}

bool ResolveObjectSprite(
    YAML::Node& object,
    ID2D1RenderTarget* renderTarget,
    ResourceManager& resourceManager)
{
    const YAML::Node sprite = object["sprite"];
    if (!sprite.IsScalar())
    {
        return false;
    }

    const std::string spriteReference = sprite.as<std::string>();
    if (resourceManager.GetSprite(spriteReference) != nullptr)
    {
        return true;
    }

    return resourceManager.LoadSprite(
        renderTarget,
        spriteReference,
        spriteReference,
        ResourceScope::Scene);
}

bool ConfigureCamera(
    const YAML::Node& root,
    ID2D1RenderTarget* renderTarget,
    BaseScene& scene)
{
    const YAML::Node camera = root["camera"];
    if (!camera)
    {
        return true;
    }

    const YAML::Node window = camera["window"];
    if (!camera.IsMap()
        || !window.IsMap()
        || !window["x"].IsScalar()
        || !window["y"].IsScalar()
        || !window["width"].IsScalar()
        || !window["height"].IsScalar())
    {
        WriteDebugMessage("Camera requires a valid window map.");
        return false;
    }

    const D2D1_SIZE_F viewport = renderTarget->GetSize();
    if (!scene.SetCameraWindow(
            window["x"].as<float>(),
            window["y"].as<float>(),
            window["width"].as<float>(),
            window["height"].as<float>(),
            viewport.width,
            viewport.height))
    {
        WriteDebugMessage("Camera window and viewport sizes must be positive.");
        return false;
    }

    const YAML::Node target = camera["target"];
    if (target && (!target.IsScalar()
        || !scene.SetCameraTarget(target.as<std::string>())))
    {
        WriteDebugMessage("Scene rejected the configured camera target.");
        return false;
    }

    return true;
}

bool AddLayers(
    const YAML::Node& root,
    ID2D1RenderTarget* renderTarget,
    ResourceManager& resourceManager,
    const ObjectTemplates& objectTemplates,
    BaseScene& scene)
{
    const YAML::Node layers = root["layers"];
    if (!layers || !layers.IsMap())
    {
        return false;
    }

    for (const auto& layerEntry : layers)
    {
        if (!layerEntry.first.IsScalar())
        {
            return false;
        }
        const std::string layerId = layerEntry.first.as<std::string>();
        const YAML::Node layerNode = layerEntry.second;
        if (!layerNode["type"].IsScalar())
        {
            return false;
        }

        const std::string type = layerNode["type"].as<std::string>();
        if (type == "Object")
        {
            const YAML::Node objects = layerNode["objects"];
            if (!objects || !objects.IsSequence())
            {
                return false;
            }

            bool sortByY = false;
            const YAML::Node sort = layerNode["sort"];
            if (sort)
            {
                if (!sort.IsScalar() || sort.as<std::string>() != "y")
                {
                    WriteDebugMessage("Object layer sort must be 'y'.");
                    return false;
                }
                sortByY = true;
            }

            auto layer = std::make_unique<ObjectLayer>(sortByY);
            for (const YAML::Node& objectNode : objects)
            {
                if (!objectNode.IsMap() || !objectNode["obj"].IsScalar())
                {
                    return false;
                }

                const std::string objectId = objectNode["obj"].as<std::string>();
                const auto objectTemplate = objectTemplates.find(objectId);
                if (objectTemplate == objectTemplates.end())
                {
                    return false;
                }

                YAML::Node properties = MergeObjectProperties(
                    objectTemplate->second,
                    objectNode,
                    "obj");
                if (!ResolveObjectSprite(
                        properties,
                        renderTarget,
                        resourceManager))
                {
                    return false;
                }

                auto object = SceneBuilder::CreateGameObject(properties);
                if (object == nullptr)
                {
                    return false;
                }

                if (!scene.OnGameObjectCreated(*object, *layer)
                    || !layer->AddObject(std::move(object)))
                {
                    return false;
                }
            }

            if (!scene.AddLayer(layerId, std::move(layer)))
            {
                return false;
            }
            continue;
        }

        if (type != "Image")
        {
            return false;
        }

        const YAML::Node items = layerNode["items"];
        if (!items || !items.IsSequence())
        {
            return false;
        }
        auto layer = std::make_unique<ImageLayer>();
        for (const YAML::Node& item : items)
        {
            if (!item["x"].IsScalar() || !item["y"].IsScalar())
            {
                return false;
            }

            const YAML::Node image = item["image"];
            const YAML::Node sprite = item["sprite"];
            const YAML::Node animation = item["animation"];
            const YAML::Node scaleNode = item["scale"];

            if (image && image.IsScalar())
            {
                layer->AddImage(
                    image.as<std::string>(),
                    item["x"].as<float>(),
                    item["y"].as<float>());
            }
            else if (sprite
                && sprite.IsScalar()
                && animation
                && animation.IsScalar())
            {
                const float scale = scaleNode && scaleNode.IsScalar()
                    ? scaleNode.as<float>()
                    : 1.0F;
                if (scale <= 0.0F)
                {
                    return false;
                }

                layer->AddSprite(
                    sprite.as<std::string>(),
                    animation.as<std::string>(),
                    item["x"].as<float>(),
                    item["y"].as<float>(),
                    scale);
            }
            else
            {
                return false;
            }
        }

        if (!scene.AddLayer(layerId, std::move(layer)))
        {
            return false;
        }
    }

    return true;
}
}

std::unique_ptr<BaseGameObject> SceneBuilder::CreateGameObject(
    const YAML::Node& node)
{
    const YAML::Node id = node["id"];
    const YAML::Node type = node["type"];
    const YAML::Node sprite = node["sprite"];
    if (!id.IsScalar() || !type.IsScalar() || !sprite.IsScalar())
    {
        return nullptr;
    }

    const std::string objectType = type.as<std::string>();
    if (objectType == "spriteObject")
    {
        return GameObjectLoad::SpriteObject(node);
    }
    if (objectType == "scrollingSurface")
    {
        return GameObjectLoad::ScrollingSurface(node);
    }
    if (objectType == "player")
    {
        return GameObjectLoad::Player(node);
    }
    if (objectType == "enemy")
    {
        return GameObjectLoad::BattleEnemy(node);
    }
    if (objectType == "playerBattle")
    {
        return GameObjectLoad::PlayerBattle(node);
    }
    if (objectType == "interactable")
    {
        return GameObjectLoad::InteractableObject(node);
    }

    WriteDebugMessage("Unknown game-object type: " + objectType);
    return nullptr;
}

std::unique_ptr<BaseGameObject> SceneBuilder::LoadGameObjectDefinition(
    const std::string& filename,
    std::string& targetLayerId,
    std::optional<std::string> idOverride,
    std::optional<float> xOverride,
    std::optional<float> yOverride)
{
    targetLayerId.clear();
    try
    {
        const YAML::Node root = YAML::LoadFile(filename);
        const YAML::Node layer = root["layer"];
        const YAML::Node init = root["init"];
        if (!layer.IsScalar() || !init.IsMap())
        {
            WriteDebugMessage(
                "Spawned object requires layer and init entries: "
                + filename);
            return nullptr;
        }

        targetLayerId = layer.as<std::string>();
        if (targetLayerId.empty())
        {
            return nullptr;
        }
        YAML::Node properties = YAML::Clone(init);
        if (idOverride.has_value())
        {
            properties["id"] = std::move(*idOverride);
        }
        if (xOverride.has_value())
        {
            properties["x"] = *xOverride;
        }
        if (yOverride.has_value())
        {
            properties["y"] = *yOverride;
        }
        return CreateGameObject(properties);
    }
    catch (const YAML::Exception& error)
    {
        WriteDebugMessage(
            "Could not load spawned object '" + filename + "': "
            + error.what());
        return nullptr;
    }
}

bool SceneBuilder::BuildScene(
    const std::string& filename,
    ID2D1RenderTarget* renderTarget,
    ResourceManager& resourceManager,
    BaseScene& scene)
{
    try
    {
        const YAML::Node root = YAML::LoadFile(filename);
        ObjectTemplates objectTemplates;
        return ConfigureSceneMetadata(root, scene)
            && ConfigurePlayerSpawnPoints(root, scene)
            && ConfigureWaypoints(root, scene)
            && LoadImages(root, renderTarget, resourceManager)
            && LoadSprites(root, renderTarget, resourceManager)
            && LoadCollisionMasks(root, resourceManager)
            && LoadSounds(root, resourceManager)
            && ConfigureGroundRegions(root, scene)
            && ConfigureBattleMoves(root, scene)
            && LoadObjects(root, objectTemplates)
            && ConfigureCamera(root, renderTarget, scene)
            && AddLayers(
                root,
                renderTarget,
                resourceManager,
                objectTemplates,
                scene);
    }
    catch (const YAML::Exception&)
    {
        return false;
    }
}
