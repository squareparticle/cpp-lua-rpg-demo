#include "ItemDefinitionLoad.h"

#include <yaml-cpp/yaml.h>

#include <utility>

bool LoadItemDefinition(
    const std::string& id,
    const std::string& filename,
    ItemDefinition& item)
{
    try
    {
        const YAML::Node root = YAML::LoadFile(filename);
        if (id.empty()
            || !root.IsMap()
            || !root["name"].IsScalar()
            || !root["description"].IsScalar()
            || !root["value"].IsScalar())
        {
            return false;
        }

        std::string useScript;
        const YAML::Node useScriptNode = root["useScript"];
        if (useScriptNode)
        {
            if (!useScriptNode.IsScalar())
            {
                return false;
            }
            useScript = useScriptNode.as<std::string>();
            if (useScript.empty())
            {
                return false;
            }
        }

        ItemDefinition parsed{
            id,
            root["name"].as<std::string>(),
            root["description"].as<std::string>(),
            root["value"].as<int>(),
            std::move(useScript)};
        if (parsed.name.empty()
            || parsed.description.empty()
            || parsed.value < 0)
        {
            return false;
        }

        item = std::move(parsed);
        return true;
    }
    catch (const YAML::Exception&)
    {
        return false;
    }
}
