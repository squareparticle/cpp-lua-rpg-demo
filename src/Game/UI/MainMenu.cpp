#include "MainMenu.h"

#include "../State/Inventory.h"
#include "../State/PlayerState.h"
#include "../Input/InputState.h"

#include <algorithm>
#include <utility>

namespace
{
constexpr float OpenDurationSeconds = 0.18F;
}

MainMenu::MainMenu(
    const Inventory& inventory,
    const PlayerState& playerState,
    const std::unordered_map<int, int>& experienceToNextLevelByLevel,
    const std::unordered_map<std::string, ItemDefinition>& itemDefinitions,
    std::string pointerImageId)
    : inventory_(inventory),
      playerState_(playerState),
      experienceToNextLevelByLevel_(experienceToNextLevelByLevel),
      itemDefinitions_(itemDefinitions),
      pointerImageId_(std::move(pointerImageId)),
      rootOptions_({
          {"save", "Save", false},
          {"load", "Load", false},
          {"inventory", "Inventory", true},
          {"status", "Status", true},
          {"quit", "Quit", true}})
{
}

void MainMenu::Open(bool animateOpen) noexcept
{
    const bool wasOpen = open_;
    rootSelectedIndex_ = 0;
    saveRequested_ = false;
    loadRequested_ = false;
    quitRequested_ = false;
    open_ = true;
    if (!wasOpen)
    {
        openFeedbackPending_ = true;
        closeFeedbackPending_ = false;
    }
    BeginPage(MainMenuPage::Root, animateOpen);
}

void MainMenu::Close() noexcept
{
    if (open_)
    {
        closeFeedbackPending_ = true;
        openFeedbackPending_ = false;
    }
    open_ = false;
    opening_ = false;
    openProgress_ = 0.0F;
}

void MainMenu::SetSaveEnabled(bool enabled) noexcept
{
    SetRootOptionEnabled("save", enabled);
}

void MainMenu::SetLoadEnabled(bool enabled) noexcept
{
    SetRootOptionEnabled("load", enabled);
}

void MainMenu::Update(float deltaTime, const InputState& input)
{
    navigationFeedbackPending_ = false;
    selectionFeedbackPending_ = false;
    if (!open_)
    {
        return;
    }
    if (opening_)
    {
        openProgress_ = std::min(
            1.0F,
            openProgress_ + std::max(deltaTime, 0.0F) / OpenDurationSeconds);
        if (openProgress_ >= 1.0F)
        {
            opening_ = false;
        }
        return;
    }
    if (input.menuPressed)
    {
        if (page_ == MainMenuPage::Root)
        {
            Close();
        }
        else
        {
            BeginPage(MainMenuPage::Root, false);
        }
        return;
    }

    if (page_ == MainMenuPage::Inventory)
    {
        inventoryMenu_.Update(input);
        navigationFeedbackPending_ =
            inventoryMenu_.ConsumeNavigationFeedback();
        selectionFeedbackPending_ =
            inventoryMenu_.ConsumeSelectionFeedback();
        return;
    }
    if (page_ == MainMenuPage::Status)
    {
        return;
    }

    if (input.navigateUpPressed && rootSelectedIndex_ > 0)
    {
        --rootSelectedIndex_;
        navigationFeedbackPending_ = true;
    }
    if (input.navigateDownPressed && rootSelectedIndex_ + 1 < rootOptions_.size())
    {
        ++rootSelectedIndex_;
        navigationFeedbackPending_ = true;
    }
    if (!input.confirmPressed || !rootOptions_[rootSelectedIndex_].enabled)
    {
        return;
    }

    selectionFeedbackPending_ = true;
    const std::string& selected = rootOptions_[rootSelectedIndex_].id;
    if (selected == "save")
    {
        saveRequested_ = true;
    }
    else if (selected == "load")
    {
        loadRequested_ = true;
        Close();
    }
    else if (selected == "inventory")
    {
        OpenInventory();
    }
    else if (selected == "status")
    {
        OpenStatus();
    }
    else if (selected == "quit")
    {
        quitRequested_ = true;
        Close();
    }
}

bool MainMenu::IsOpen() const noexcept
{
    return open_;
}

bool MainMenu::ConsumeSaveRequest() noexcept
{
    const bool requested = saveRequested_;
    saveRequested_ = false;
    return requested;
}

bool MainMenu::ConsumeLoadRequest() noexcept
{
    const bool requested = loadRequested_;
    loadRequested_ = false;
    return requested;
}

bool MainMenu::ConsumeQuitRequest() noexcept
{
    const bool requested = quitRequested_;
    quitRequested_ = false;
    return requested;
}

std::optional<std::string> MainMenu::ConsumeItemUseRequest()
{
    return inventoryMenu_.ConsumeUseRequest();
}

bool MainMenu::ConsumeOpenFeedback() noexcept
{
    const bool pending = openFeedbackPending_;
    openFeedbackPending_ = false;
    return pending;
}

bool MainMenu::ConsumeCloseFeedback() noexcept
{
    const bool pending = closeFeedbackPending_;
    closeFeedbackPending_ = false;
    return pending;
}

bool MainMenu::ConsumeNavigationFeedback() noexcept
{
    const bool pending = navigationFeedbackPending_;
    navigationFeedbackPending_ = false;
    return pending;
}

bool MainMenu::ConsumeSelectionFeedback() noexcept
{
    const bool pending = selectionFeedbackPending_;
    selectionFeedbackPending_ = false;
    return pending;
}

MainMenuPage MainMenu::GetPage() const noexcept
{
    return page_;
}

std::size_t MainMenu::GetRootSelectedIndex() const noexcept
{
    return rootSelectedIndex_;
}

std::size_t MainMenu::GetInventorySelectedIndex() const noexcept
{
    return inventoryMenu_.GetSelectedIndex();
}

void MainMenu::BeginPage(MainMenuPage page, bool animateOpen) noexcept
{
    page_ = page;
    opening_ = animateOpen;
    openProgress_ = animateOpen ? 0.0F : 1.0F;
}

bool MainMenu::OpenInventory(bool animateOpen)
{
    std::vector<InventoryMenuEntry> entries;
    for (const std::string& itemId : inventory_.GetItemIds())
    {
        const auto definition = itemDefinitions_.find(itemId);
        if (definition == itemDefinitions_.end())
        {
            return false;
        }
        entries.push_back({
            itemId,
            definition->second.name,
            definition->second.description,
            definition->second.value,
            !definition->second.useScript.empty()});
    }
    const bool wasOpen = open_;
    inventoryMenu_.SetEntries(std::move(entries));
    saveRequested_ = false;
    loadRequested_ = false;
    quitRequested_ = false;
    open_ = true;
    if (!wasOpen)
    {
        openFeedbackPending_ = true;
        closeFeedbackPending_ = false;
    }
    BeginPage(MainMenuPage::Inventory, animateOpen);
    return true;
}

bool MainMenu::OpenStatus()
{
    const auto experienceToNextLevel =
        experienceToNextLevelByLevel_.find(playerState_.GetLevel());
    if (!playerState_.IsInitialized()
        || experienceToNextLevel == experienceToNextLevelByLevel_.end())
    {
        return false;
    }

    statusMenu_.SetStatus({
        playerState_.GetName(),
        playerState_.GetLevel(),
        playerState_.GetHp(),
        playerState_.GetMaxHp(),
        playerState_.GetMp(),
        playerState_.GetMaxMp(),
        playerState_.GetExperience(),
        playerState_.GetExperienceUntilNextLevel(
            experienceToNextLevel->second),
        playerState_.GetGold()});
    BeginPage(MainMenuPage::Status, true);
    return true;
}

void MainMenu::SetRootOptionEnabled(
    std::string_view id,
    bool enabled) noexcept
{
    const auto option = std::ranges::find(rootOptions_, id, &RootOption::id);
    if (option != rootOptions_.end())
    {
        option->enabled = enabled;
    }
}
