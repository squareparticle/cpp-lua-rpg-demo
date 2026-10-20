#pragma once

#include <string>

class Graphics;

struct PlayerStatusMenuData
{
    std::string name;
    int level = 1;
    int hp = 0;
    int maxHp = 0;
    int mp = 0;
    int maxMp = 0;
    int experience = 0;
    int experienceUntilNextLevel = 0;
    int gold = 0;
};

class StatusMenu
{
public:
    void SetStatus(PlayerStatusMenuData status);
    void Render(const Graphics& graphics, float openProgress) const;

private:
    PlayerStatusMenuData status_;
};
