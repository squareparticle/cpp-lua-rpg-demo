#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

class Graphics;
class ResourceManager;
struct InputState;

struct InventoryMenuEntry
{
    std::string id;
    std::string name;
    std::string description;
    int value = 0;
    bool usable = false;
};

class InventoryMenu
{
public:
    void SetEntries(std::vector<InventoryMenuEntry> entries);
    void Update(const InputState& input);
    void Render(
        const Graphics& graphics,
        const ResourceManager& resources,
        float openProgress,
        const std::string& pointerImageId) const;

    std::optional<std::string> ConsumeUseRequest();
    bool ConsumeNavigationFeedback() noexcept;
    bool ConsumeSelectionFeedback() noexcept;
    std::size_t GetSelectedIndex() const noexcept;

private:
    std::vector<InventoryMenuEntry> entries_;
    std::size_t selectedIndex_ = 0;
    std::optional<std::string> useRequest_;
    bool navigationFeedbackPending_ = false;
    bool selectionFeedbackPending_ = false;
};
