#include "RuleTable.h"

#include "../../Platform/DebugLog.h"
#include "../State/GlobalState.h"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <array>
#include <string>

namespace
{
bool ReadTagList(const YAML::Node& node, std::vector<std::string>& tags)
{
    if (!node.IsSequence() || node.size() == 0)
    {
        return false;
    }

    for (const YAML::Node& tag : node)
    {
        if (!tag.IsScalar())
        {
            return false;
        }

        std::string name = tag.as<std::string>();
        if (name.empty())
        {
            return false;
        }
        tags.push_back(std::move(name));
    }
    return true;
}

bool Matches(const RuleCondition& condition, const GlobalState& state)
{
    const bool allMatch = std::ranges::all_of(
        condition.all,
        [&state](const std::string& tag) { return state.GetTag(tag); });
    const bool anyMatch = condition.any.empty() || std::ranges::any_of(
        condition.any,
        [&state](const std::string& tag) { return state.GetTag(tag); });
    const bool noneMatch = std::ranges::none_of(
        condition.none,
        [&state](const std::string& tag) { return state.GetTag(tag); });
    return allMatch && anyMatch && noneMatch;
}
}

bool RuleTable::Load(const std::string& filename)
{
    filename_ = filename;
    rules_.clear();

    try
    {
        const YAML::Node root = YAML::LoadFile(filename);
        const YAML::Node rules = root["rules"];
        if (!rules || !rules.IsSequence())
        {
            WriteDebugMessage("Rule table must contain a rules sequence: " + filename);
            return false;
        }

        for (const YAML::Node& ruleNode : rules)
        {
            if (!ruleNode.IsMap()
                || !ruleNode["script"].IsScalar())
            {
                WriteDebugMessage("Malformed rule in: " + filename);
                return false;
            }

            ScriptRule rule;
            rule.scriptPath = ruleNode["script"].as<std::string>();
            if (rule.scriptPath.empty())
            {
                WriteDebugMessage("Rule has an empty script path in: " + filename);
                return false;
            }

            const YAML::Node when = ruleNode["when"];
            rule.unconditional = !when;
            if (when)
            {
                if (!when.IsMap())
                {
                    WriteDebugMessage("Rule when must be a map in: " + filename);
                    return false;
                }

                for (const auto& conditionEntry : when)
                {
                    const std::string name = conditionEntry.first.as<std::string>();
                    std::vector<std::string>* destination = nullptr;
                    if (name == "all")
                    {
                        destination = &rule.condition.all;
                    }
                    else if (name == "any")
                    {
                        destination = &rule.condition.any;
                    }
                    else if (name == "none")
                    {
                        destination = &rule.condition.none;
                    }
                    else
                    {
                        WriteDebugMessage("Unknown rule condition '" + name + "' in: " + filename);
                        return false;
                    }

                    if (!ReadTagList(conditionEntry.second, *destination))
                    {
                        WriteDebugMessage("Rule condition must contain tag strings in: " + filename);
                        return false;
                    }
                }
            }

            rules_.push_back(std::move(rule));
        }
    }
    catch (const YAML::Exception& error)
    {
        WriteDebugMessage("Could not load rule table '" + filename + "': " + error.what());
        return false;
    }

    return true;
}

const ScriptRule* RuleTable::Resolve(const GlobalState& state) const noexcept
{
    // File order is priority: specific mission states belong above the
    // unconditional fallback, and the first matching rule wins.
    for (const ScriptRule& rule : rules_)
    {
        if (rule.unconditional || Matches(rule.condition, state))
        {
            return &rule;
        }
    }
    return nullptr;
}
