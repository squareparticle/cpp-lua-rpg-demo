#pragma once

#include "InventoryMenu.h"
#include "StatusMenu.h"
#include "../Items/ItemDefinition.h"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

class Graphics;
class Inventory;
class PlayerState;
class ResourceManager;
struct InputState;

enum class MainMenuPage
{
    Root,
    Inventory,
    Status
};

class MainMenu
{
public:
    MainMenu(
        const Inventory& inventory,
        const PlayerState& playerState,
        const std::unordered_map<int, int>& experienceToNextLevelByLevel,
        const std::unordered_map<std::string, ItemDefinition>& itemDefinitions,
        std::string pointerImageId);

    void Open(bool animateOpen = true) noexcept;
    bool OpenInventory(bool animateOpen = true);
    void Close() noexcept;
    void SetSaveEnabled(bool enabled) noexcept;
    void SetLoadEnabled(bool enabled) noexcept;
    void Update(float deltaTime, const InputState& input);
    void Render(
        const Graphics& graphics,
        const ResourceManager& resources) const;

    bool IsOpen() const noexcept;
    bool ConsumeSaveRequest() noexcept;
    bool ConsumeLoadRequest() noexcept;
    bool ConsumeQuitRequest() noexcept;
    std::optional<std::string> ConsumeItemUseRequest();
    bool ConsumeOpenFeedback() noexcept;
    bool ConsumeCloseFeedback() noexcept;
    bool ConsumeNavigationFeedback() noexcept;
    bool ConsumeSelectionFeedback() noexcept;
    MainMenuPage GetPage() const noexcept;
    std::size_t GetRootSelectedIndex() const noexcept;
    std::size_t GetInventorySelectedIndex() const noexcept;

private:
    struct RootOption
    {
        std::string id;
        std::string text;
        bool enabled = true;
    };

    void BeginPage(MainMenuPage page, bool animateOpen) noexcept;
    bool OpenStatus();
    void SetRootOptionEnabled(std::string_view id, bool enabled) noexcept;
    void RenderRoot(
        const Graphics& graphics,
        const ResourceManager& resources) const;

    const Inventory& inventory_;
    const PlayerState& playerState_;
    const std::unordered_map<int, int>& experienceToNextLevelByLevel_;
    const std::unordered_map<std::string, ItemDefinition>& itemDefinitions_;
    std::string pointerImageId_;
    std::vector<RootOption> rootOptions_;
    InventoryMenu inventoryMenu_;
    StatusMenu statusMenu_;
    MainMenuPage page_ = MainMenuPage::Root;
    std::size_t rootSelectedIndex_ = 0;
    float openProgress_ = 0.0F;
    bool open_ = false;
    bool opening_ = false;
    bool saveRequested_ = false;
    bool loadRequested_ = false;
    bool quitRequested_ = false;
    bool openFeedbackPending_ = false;
    bool closeFeedbackPending_ = false;
    bool navigationFeedbackPending_ = false;
    bool selectionFeedbackPending_ = false;
};
