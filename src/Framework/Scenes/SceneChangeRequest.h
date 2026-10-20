#pragma once

#include <string>
#include <unordered_map>
#include <variant>

using SceneParameterValue = std::variant<std::string, double, bool>;
using SceneParameterMap =
    std::unordered_map<std::string, SceneParameterValue>;

struct SceneChangeRequest
{
    // Optional generic provenance for diagnostics, such as the system or
    // activity that requested the transition. The framework does not assign
    // gameplay meaning to this value.
    std::string source;
    std::string destination;
    SceneParameterMap parameters;
};
