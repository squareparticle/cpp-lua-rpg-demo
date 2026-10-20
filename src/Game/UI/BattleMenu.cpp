#include "BattleMenu.h"

#include "../Input/InputState.h"

#include <utility>

void BattleMenu::Configure(
    BattleStats playerStats,
    std::vector<BattleMenuTarget> targets)
{
    playerStats_ = std::move(playerStats);
    targets_ = std::move(targets);
    selectedTargetId_.clear();
    commandIndex_ = 0;
    targetIndex_ = 0;
    victoryConfirmed_ = false;
    navigationFeedbackPending_ = false;
    selectionFeedbackPending_ = false;
    state_ = BattleMenuState::Commands;
}

void BattleMenu::Update(const InputState& input)
{
    navigationFeedbackPending_ = false;
    selectionFeedbackPending_ = false;
    if (state_ == BattleMenuState::Victory)
    {
        if (input.confirmPressed)
        {
            victoryConfirmed_ = true;
            selectionFeedbackPending_ = true;
        }
        return;
    }

    if (state_ == BattleMenuState::Commands)
    {
        if (input.navigateUpPressed && commandIndex_ > 0)
        {
            --commandIndex_;
            navigationFeedbackPending_ = true;
        }
        if (input.navigateDownPressed && commandIndex_ < 3)
        {
            ++commandIndex_;
            navigationFeedbackPending_ = true;
        }
        if (input.confirmPressed && commandIndex_ == 0 && !targets_.empty())
        {
            state_ = BattleMenuState::Targets;
            selectionFeedbackPending_ = true;
        }
        return;
    }

    if (state_ == BattleMenuState::Targets)
    {
        if (input.navigateUpPressed && targetIndex_ > 0)
        {
            --targetIndex_;
            navigationFeedbackPending_ = true;
        }
        if (input.navigateDownPressed && targetIndex_ + 1 < targets_.size())
        {
            ++targetIndex_;
            navigationFeedbackPending_ = true;
        }
        if (input.confirmPressed)
        {
            selectedTargetId_ = targets_[targetIndex_].id;
            state_ = BattleMenuState::TargetChosen;
            selectionFeedbackPending_ = true;
        }
    }
}

void BattleMenu::SetBusy() noexcept
{
    selectedTargetId_.clear();
    state_ = BattleMenuState::Busy;
}

void BattleMenu::ShowVictory(
    int experienceGained,
    int experienceUntilNextLevel,
    std::vector<std::string> itemNames)
{
    selectedTargetId_.clear();
    experienceGained_ = experienceGained;
    experienceUntilNextLevel_ = experienceUntilNextLevel;
    itemNames_ = std::move(itemNames);
    victoryConfirmed_ = false;
    state_ = BattleMenuState::Victory;
}

void BattleMenu::ShowDefeat() noexcept
{
    selectedTargetId_.clear();
    state_ = BattleMenuState::Defeat;
}

bool BattleMenu::ConsumeVictoryConfirmed() noexcept
{
    const bool confirmed = victoryConfirmed_;
    victoryConfirmed_ = false;
    return confirmed;
}

bool BattleMenu::ConsumeNavigationFeedback() noexcept
{
    const bool pending = navigationFeedbackPending_;
    navigationFeedbackPending_ = false;
    return pending;
}

bool BattleMenu::ConsumeSelectionFeedback() noexcept
{
    const bool pending = selectionFeedbackPending_;
    selectionFeedbackPending_ = false;
    return pending;
}

BattleMenuState BattleMenu::GetState() const noexcept
{
    return state_;
}

std::size_t BattleMenu::GetCommandIndex() const noexcept
{
    return commandIndex_;
}

std::size_t BattleMenu::GetTargetIndex() const noexcept
{
    return targetIndex_;
}

const std::string& BattleMenu::GetSelectedTargetId() const noexcept
{
    return selectedTargetId_;
}
