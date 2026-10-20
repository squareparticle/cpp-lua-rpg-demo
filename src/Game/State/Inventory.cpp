#include "Inventory.h"

#include <algorithm>
#include <utility>

bool Inventory::AddItem(std::string itemId)
{
    if (itemId.empty())
    {
        return false;
    }

    return itemIds_.insert(std::move(itemId)).second;
}

bool Inventory::RemoveItem(const std::string& itemId)
{
    return itemIds_.erase(itemId) != 0;
}

bool Inventory::HasItem(const std::string& itemId) const noexcept
{
    return itemIds_.contains(itemId);
}

std::vector<std::string> Inventory::GetItemIds() const
{
    std::vector<std::string> ids(itemIds_.begin(), itemIds_.end());
    std::ranges::sort(ids);
    return ids;
}
