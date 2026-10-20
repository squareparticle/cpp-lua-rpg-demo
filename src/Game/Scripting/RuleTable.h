#pragma once

#include <string>
#include <vector>

class GlobalState;

struct RuleCondition
{
    std::vector<std::string> all;
    std::vector<std::string> any;
    std::vector<std::string> none;
};

struct ScriptRule
{
    RuleCondition condition;
    std::string scriptPath;
    bool unconditional = false;
};

class RuleTable
{
public:
    bool Load(const std::string& filename);
    const ScriptRule* Resolve(const GlobalState& state) const noexcept;

private:
    std::string filename_;
    std::vector<ScriptRule> rules_;
};
