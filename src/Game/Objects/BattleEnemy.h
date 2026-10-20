#pragma once

#include "../../Framework/Objects/BaseGameObject.h"
#include "../Battle/BattleStats.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

class BattleEnemy final : public BaseGameObject
{
public:
    BattleEnemy(
        std::string id,
        std::string spriteId,
        std::string animationName,
        float x,
        float y,
        float scale,
        BattleStats stats,
        std::vector<std::string> moveIds,
        std::string battleScript,
        int experienceReward,
        std::vector<std::string> itemDropIds,
        bool active);

    bool Init(ResourceManager& resources) override;
    void Update(float deltaTime) override;
    void Render(const Graphics& graphics, const Camera& camera) override;

    const BattleStats& GetStats() const noexcept;
    const std::vector<std::string>& GetMoveIds() const noexcept;
    const std::string& GetBattleScript() const noexcept;
    int GetExperienceReward() const noexcept;
    const std::vector<std::string>& GetItemDropIds() const noexcept;
    // C++ uses this focused query to validate a proposed action. GetMoveIds
    // supplies the copied Lua tactics snapshot and future move menus.
    bool KnowsMove(std::string_view moveId) const noexcept;
    bool PlayBattleAnimation(
        const std::string& animationName,
        bool loop = false);
    void ReturnToIdle();
    std::size_t GetAnimationFrameIndex() const noexcept;
    bool IsAnimationFinished() const noexcept;
    void ApplyDamage(int amount) noexcept;
    bool SpendMp(int amount) noexcept;
    bool IsActive() const noexcept;
    void SetActive(bool active) noexcept;

private:
    std::string spriteId_;
    std::string animationName_;
    float x_ = 0.0F;
    float y_ = 0.0F;
    float scale_ = 1.0F;
    BattleStats stats_;
    std::vector<std::string> moveIds_;
    std::string battleScript_;
    int experienceReward_ = 0;
    std::vector<std::string> itemDropIds_;
    bool active_ = false;
};
