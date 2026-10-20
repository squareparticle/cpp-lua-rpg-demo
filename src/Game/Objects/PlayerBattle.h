#pragma once

#include "../../Framework/Objects/BaseGameObject.h"
#include "../Battle/BattleStats.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

class PlayerState;

class PlayerBattle final : public BaseGameObject
{
public:
    PlayerBattle(
        std::string id,
        std::string spriteId,
        std::string animationName,
        float x,
        float y,
        float scale,
        std::vector<std::string> moveIds,
        bool active);

    bool BindPlayerState(PlayerState& playerState);
    bool Init(ResourceManager& resources) override;
    void Update(float deltaTime) override;
    void Render(const Graphics& graphics, const Camera& camera) override;

    // PlayerState owns this data. PlayerBattle only presents the common
    // battle-facing view shared with BattleEnemy.
    const BattleStats& GetStats() const noexcept;
    const std::vector<std::string>& GetMoveIds() const noexcept;
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

private:
    std::string spriteId_;
    std::string animationName_;
    float x_ = 0.0F;
    float y_ = 0.0F;
    float scale_ = 1.0F;
    PlayerState* playerState_ = nullptr;
    std::vector<std::string> moveIds_;
    bool active_ = false;
};
