#pragma once

#include "../Battle/BattleStats.h"

#include <string>

class PlayerState
{
public:
    bool Initialize(
        std::string name,
        int level,
        int experience,
        int maxHp,
        int maxMp,
        int attack,
        int defense,
        int gold);
    bool AddExperience(int amount) noexcept;
    void SetHp(int hp) noexcept;
    void SetMp(int mp) noexcept;

    bool IsInitialized() const noexcept;
    const std::string& GetName() const noexcept;
    int GetLevel() const noexcept;
    int GetExperience() const noexcept;
    int GetExperienceUntilNextLevel(
        int experienceToNextLevel) const noexcept;
    int GetHp() const noexcept;
    int GetMaxHp() const noexcept;
    int GetMp() const noexcept;
    int GetMaxMp() const noexcept;
    int GetAttack() const noexcept;
    int GetDefense() const noexcept;
    int GetGold() const noexcept;
    const BattleStats& GetBattleStats() const noexcept;

private:
    bool initialized_ = false;
    int experience_ = 0;
    int gold_ = 0;
    BattleStats battleStats_;
};
