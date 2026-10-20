#include "PlayerState.h"

#include <algorithm>
#include <limits>
#include <utility>

bool PlayerState::Initialize(
    std::string name,
    int level,
    int experience,
    int maxHp,
    int maxMp,
    int attack,
    int defense,
    int gold)
{
    if (initialized_)
    {
        return true;
    }
    if (name.empty()
        || level < 1
        || experience < 0
        || maxHp <= 0
        || maxMp < 0
        || attack <= 0
        || defense < 0
        || gold < 0)
    {
        return false;
    }

    battleStats_ = BattleStats{
        std::move(name),
        level,
        maxHp,
        maxHp,
        maxMp,
        maxMp,
        attack,
        defense};
    experience_ = experience;
    gold_ = gold;
    initialized_ = true;
    return true;
}

bool PlayerState::AddExperience(int amount) noexcept
{
    if (!initialized_
        || amount < 0
        || experience_ > std::numeric_limits<int>::max() - amount)
    {
        return false;
    }
    experience_ += amount;
    return true;
}

void PlayerState::SetHp(int hp) noexcept
{
    battleStats_.hp = std::clamp(hp, 0, battleStats_.maxHp);
}

void PlayerState::SetMp(int mp) noexcept
{
    battleStats_.mp = std::clamp(mp, 0, battleStats_.maxMp);
}

bool PlayerState::IsInitialized() const noexcept
{
    return initialized_;
}

const std::string& PlayerState::GetName() const noexcept
{
    return battleStats_.name;
}

int PlayerState::GetLevel() const noexcept
{
    return battleStats_.level;
}

int PlayerState::GetExperience() const noexcept
{
    return experience_;
}

int PlayerState::GetExperienceUntilNextLevel(
    int experienceToNextLevel) const noexcept
{
    return std::max(0, experienceToNextLevel - experience_);
}

int PlayerState::GetHp() const noexcept
{
    return battleStats_.hp;
}

int PlayerState::GetMaxHp() const noexcept
{
    return battleStats_.maxHp;
}

int PlayerState::GetMp() const noexcept
{
    return battleStats_.mp;
}

int PlayerState::GetMaxMp() const noexcept
{
    return battleStats_.maxMp;
}

int PlayerState::GetAttack() const noexcept
{
    return battleStats_.attack;
}

int PlayerState::GetDefense() const noexcept
{
    return battleStats_.defense;
}

int PlayerState::GetGold() const noexcept
{
    return gold_;
}

const BattleStats& PlayerState::GetBattleStats() const noexcept
{
    return battleStats_;
}
