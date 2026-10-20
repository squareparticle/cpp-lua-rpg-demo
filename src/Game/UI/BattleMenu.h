#pragma once

#include "../Battle/BattleStats.h"

#include <cstddef>
#include <string>
#include <vector>

class Graphics;
class ResourceManager;
struct InputState;

struct BattleMenuTarget
{
    std::string id;
    std::string name;
};

enum class BattleMenuState
{
    Commands,
    Targets,
    TargetChosen,
    Busy,
    Victory,
    Defeat
};

class BattleMenu
{
public:
    void Configure(
        BattleStats playerStats,
        std::vector<BattleMenuTarget> targets);
    void Update(const InputState& input);
    void SetBusy() noexcept;
    void ShowVictory(
        int experienceGained,
        int experienceUntilNextLevel,
        std::vector<std::string> itemNames);
    void ShowDefeat() noexcept;
    bool ConsumeVictoryConfirmed() noexcept;
    bool ConsumeNavigationFeedback() noexcept;
    bool ConsumeSelectionFeedback() noexcept;
    void Render(
        const Graphics& graphics,
        const ResourceManager& resources) const;

    BattleMenuState GetState() const noexcept;
    std::size_t GetCommandIndex() const noexcept;
    std::size_t GetTargetIndex() const noexcept;
    const std::string& GetSelectedTargetId() const noexcept;

private:
    BattleStats playerStats_;
    std::vector<BattleMenuTarget> targets_;
    std::string selectedTargetId_;
    std::size_t commandIndex_ = 0;
    std::size_t targetIndex_ = 0;
    int experienceGained_ = 0;
    int experienceUntilNextLevel_ = 0;
    std::vector<std::string> itemNames_;
    bool victoryConfirmed_ = false;
    bool navigationFeedbackPending_ = false;
    bool selectionFeedbackPending_ = false;
    BattleMenuState state_ = BattleMenuState::Commands;
};
