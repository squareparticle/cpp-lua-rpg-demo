#pragma once

#include <string>
#include <unordered_map>

class GlobalState
{
public:
    void SetTag(std::string name, bool value);
    bool HasTag(const std::string& name) const noexcept;
    bool GetTag(const std::string& name) const noexcept;
    const std::unordered_map<std::string, bool>& GetTags() const noexcept;

private:
    std::unordered_map<std::string, bool> tags_;
};
