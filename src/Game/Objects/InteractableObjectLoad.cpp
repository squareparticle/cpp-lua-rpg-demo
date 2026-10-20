#include "GameObjectLoad.h"

#include "InteractableObject.h"
#include "TriggerRegion.h"
#include "../../Platform/DebugLog.h"
#include "../../Framework/Objects/ObjectHook.h"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace
{
bool ReadHooks(
    const YAML::Node& hooksNode,
    std::vector<ObjectHookBinding>& hooks)
{
    if (!hooksNode)
    {
        return true;
    }
    if (!hooksNode.IsMap())
    {
        WriteDebugMessage("Object hooks must be a map.");
        return false;
    }

    for (const auto& hookEntry : hooksNode)
    {
        if (!hookEntry.first.IsScalar() || !hookEntry.second.IsMap())
        {
            WriteDebugMessage("Malformed object hook.");
            return false;
        }

        const std::string hookName = hookEntry.first.as<std::string>();
        const std::optional<ObjectHook> hook = ParseObjectHook(hookName);
        if (!hook.has_value())
        {
            WriteDebugMessage("Unknown object hook: " + hookName);
            return false;
        }

        if (std::ranges::any_of(
                hooks,
                [hook](const ObjectHookBinding& existing)
                {
                    return existing.hook == *hook;
                }))
        {
            WriteDebugMessage("Duplicate object hook: " + hookName);
            return false;
        }

        const YAML::Node script = hookEntry.second["script"];
        const YAML::Node ruleTable = hookEntry.second["ruleTable"];
        if ((script && ruleTable)
            || (!script && !ruleTable)
            || (script && !script.IsScalar())
            || (ruleTable && !ruleTable.IsScalar()))
        {
            WriteDebugMessage(
                "Hook " + hookName
                + " must define exactly one script or ruleTable.");
            return false;
        }

        ObjectHookBinding binding;
        binding.hook = *hook;
        if (script)
        {
            binding.scriptPath = script.as<std::string>();
        }
        else
        {
            binding.ruleTablePath = ruleTable.as<std::string>();
        }
        if (binding.scriptPath.empty() && binding.ruleTablePath.empty())
        {
            WriteDebugMessage("Hook path cannot be empty: " + hookName);
            return false;
        }
        hooks.push_back(std::move(binding));
    }

    return true;
}

bool ReadTriggers(
    const YAML::Node& triggersNode,
    std::vector<TriggerRegion>& triggers)
{
    if (!triggersNode)
    {
        return true;
    }
    if (!triggersNode.IsMap())
    {
        WriteDebugMessage("Object triggers must be a map.");
        return false;
    }

    for (const auto& triggerEntry : triggersNode)
    {
        if (!triggerEntry.first.IsScalar() || !triggerEntry.second.IsMap())
        {
            WriteDebugMessage("Malformed object trigger.");
            return false;
        }

        TriggerRegion trigger;
        trigger.name = triggerEntry.first.as<std::string>();
        const YAML::Node definition = triggerEntry.second;
        const YAML::Node shape = definition["shape"];
        const YAML::Node x = definition["x"];
        const YAML::Node y = definition["y"];
        if (trigger.name.empty()
            || !shape.IsScalar()
            || (x && !x.IsScalar())
            || (y && !y.IsScalar())
            || std::ranges::any_of(
                triggers,
                [&trigger](const TriggerRegion& existing)
                {
                    return existing.name == trigger.name;
                }))
        {
            WriteDebugMessage("Invalid or duplicate object trigger.");
            return false;
        }

        trigger.x = x ? x.as<float>() : 0.0F;
        trigger.y = y ? y.as<float>() : 0.0F;
        const std::string shapeName = shape.as<std::string>();
        if (shapeName == "circle")
        {
            const YAML::Node radius = definition["radius"];
            if (!radius.IsScalar())
            {
                WriteDebugMessage(
                    "Circle trigger requires a radius: " + trigger.name);
                return false;
            }
            trigger.shape = TriggerRegionShape::Circle;
            trigger.radius = radius.as<float>();
            if (!std::isfinite(trigger.radius) || trigger.radius <= 0.0F)
            {
                return false;
            }
        }
        else if (shapeName == "rect")
        {
            const YAML::Node width = definition["width"];
            const YAML::Node height = definition["height"];
            if (!width.IsScalar() || !height.IsScalar())
            {
                WriteDebugMessage(
                    "Rect trigger requires width and height: "
                    + trigger.name);
                return false;
            }
            trigger.shape = TriggerRegionShape::Rectangle;
            trigger.width = width.as<float>();
            trigger.height = height.as<float>();
            if (!std::isfinite(trigger.width)
                || !std::isfinite(trigger.height)
                || trigger.width <= 0.0F
                || trigger.height <= 0.0F)
            {
                return false;
            }
        }
        else
        {
            WriteDebugMessage(
                "Unknown object trigger shape: " + shapeName);
            return false;
        }

        if (!std::isfinite(trigger.x) || !std::isfinite(trigger.y))
        {
            return false;
        }
        triggers.push_back(std::move(trigger));
    }

    return true;
}
}

std::unique_ptr<BaseGameObject> GameObjectLoad::InteractableObject(
    const YAML::Node& node)
{
    const YAML::Node animation = node["animation"];
    const YAML::Node x = node["x"];
    const YAML::Node y = node["y"];
    const YAML::Node scale = node["scale"];
    const YAML::Node sortPointY = node["sortPointY"];
    const YAML::Node promptSprite = node["promptSprite"];
    const YAML::Node promptAnimation = node["promptAnimation"];
    const YAML::Node promptScale = node["promptScale"];

    if (!animation.IsScalar()
        || !x.IsScalar()
        || !y.IsScalar()
        || (sortPointY && !sortPointY.IsScalar())
        || !promptSprite.IsScalar()
        || !promptAnimation.IsScalar())
    {
        WriteDebugMessage(
            "Interactable object is missing required properties.");
        return nullptr;
    }

    const float objectScale = scale && scale.IsScalar()
        ? scale.as<float>()
        : 1.0F;
    const float objectPromptScale = promptScale && promptScale.IsScalar()
        ? promptScale.as<float>()
        : 1.0F;
    if (objectScale <= 0.0F || objectPromptScale <= 0.0F)
    {
        return nullptr;
    }

    std::vector<ObjectHookBinding> hooks;
    std::vector<TriggerRegion> triggers;
    if (!ReadHooks(node["hooks"], hooks)
        || !ReadTriggers(node["triggers"], triggers))
    {
        return nullptr;
    }

    return std::make_unique<::InteractableObject>(
        node["id"].as<std::string>(),
        node["sprite"].as<std::string>(),
        animation.as<std::string>(),
        x.as<float>(),
        y.as<float>(),
        objectScale,
        sortPointY ? sortPointY.as<float>() : 0.0F,
        promptSprite.as<std::string>(),
        promptAnimation.as<std::string>(),
        objectPromptScale,
        std::move(triggers),
        std::move(hooks));
}
