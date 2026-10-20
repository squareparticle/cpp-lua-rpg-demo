#include "InventoryMenu.h"

#include "../Input/InputState.h"

#include <utility>

void InventoryMenu::SetEntries(std::vector<InventoryMenuEntry> entries)
{
    entries_ = std::move(entries);
    selectedIndex_ = 0;
    useRequest_.reset();
}

void InventoryMenu::Update(const InputState& input)
{
    navigationFeedbackPending_ = false;
    selectionFeedbackPending_ = false;
    if (input.navigateUpPressed && selectedIndex_ > 0)
    {
        --selectedIndex_;
        navigationFeedbackPending_ = true;
    }
    if (input.navigateDownPressed && selectedIndex_ + 1 < entries_.size())
    {
        ++selectedIndex_;
        navigationFeedbackPending_ = true;
    }
    if (input.confirmPressed
        && !entries_.empty()
        && entries_[selectedIndex_].usable)
    {
        useRequest_ = entries_[selectedIndex_].id;
        selectionFeedbackPending_ = true;
    }
}

bool InventoryMenu::ConsumeNavigationFeedback() noexcept
{
    const bool pending = navigationFeedbackPending_;
    navigationFeedbackPending_ = false;
    return pending;
}

bool InventoryMenu::ConsumeSelectionFeedback() noexcept
{
    const bool pending = selectionFeedbackPending_;
    selectionFeedbackPending_ = false;
    return pending;
}

std::optional<std::string> InventoryMenu::ConsumeUseRequest()
{
    std::optional<std::string> request = std::move(useRequest_);
    useRequest_.reset();
    return request;
}

std::size_t InventoryMenu::GetSelectedIndex() const noexcept
{
    return selectedIndex_;
}
