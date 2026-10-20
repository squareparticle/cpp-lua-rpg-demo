#include "GlobalState.h"

#include <utility>

void GlobalState::SetTag(std::string name, bool value)
{
    tags_[std::move(name)] = value;
}

bool GlobalState::HasTag(const std::string& name) const noexcept
{
    return tags_.contains(name);
}

bool GlobalState::GetTag(const std::string& name) const noexcept
{
    const auto tag = tags_.find(name);
    return tag != tags_.end() && tag->second;
}

const std::unordered_map<std::string, bool>& GlobalState::GetTags() const noexcept
{
    return tags_;
}
