#pragma once

#include <string>
#include <unordered_set>
#include <vector>

class Inventory
{
public:
    bool AddItem(std::string itemId);
    bool RemoveItem(const std::string& itemId);
    bool HasItem(const std::string& itemId) const noexcept;
    std::vector<std::string> GetItemIds() const;

private:
    // This vertical slice needs unique persistent items, not quantities or
    // item definitions. A richer item model can wait for a real gameplay need.
    std::unordered_set<std::string> itemIds_;
};
