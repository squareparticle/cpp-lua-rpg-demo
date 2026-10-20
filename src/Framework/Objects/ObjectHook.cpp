#include "ObjectHook.h"

std::optional<ObjectHook> ParseObjectHook(std::string_view name) noexcept
{
    if (name == "INIT")
    {
        return ObjectHook::Init;
    }
    if (name == "DUTIES")
    {
        return ObjectHook::Duties;
    }
    if (name == "INTERACT")
    {
        return ObjectHook::Interact;
    }
    if (name == "DESTROY")
    {
        return ObjectHook::Destroy;
    }
    return std::nullopt;
}

std::string_view ToString(ObjectHook hook) noexcept
{
    switch (hook)
    {
    case ObjectHook::Init:
        return "INIT";
    case ObjectHook::Duties:
        return "DUTIES";
    case ObjectHook::Interact:
        return "INTERACT";
    case ObjectHook::Destroy:
        return "DESTROY";
    default:
        return "UNKNOWN";
    }
}
