#pragma once

#include <optional>
#include <string>
#include <string_view>

enum class ObjectHook
{
    Init,
    Duties,
    Interact,
    Destroy
};

struct ObjectHookBinding
{
    ObjectHook hook = ObjectHook::Init;
    std::string scriptPath;
    std::string ruleTablePath;
};

std::optional<ObjectHook> ParseObjectHook(std::string_view name) noexcept;
std::string_view ToString(ObjectHook hook) noexcept;
