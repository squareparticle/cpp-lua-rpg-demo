#include "PlayerBattle.h"

#include "../../Framework/Rendering/Camera.h"
#include "../../Framework/Resources/ResourceManager.h"
#include "../State/PlayerState.h"

#include <algorithm>
#include <cassert>
#include <utility>

PlayerBattle::PlayerBattle(
    std::string id,
    std::string spriteId,
    std::string animationName,
    float x,
    float y,
    float scale,
    std::vector<std::string> moveIds,
    bool active)
    : BaseGameObject(std::move(id)),
      spriteId_(std::move(spriteId)),
      animationName_(std::move(animationName)),
      x_(x),
      y_(y),
      scale_(scale),
      moveIds_(std::move(moveIds)),
      active_(active)
{
}

bool PlayerBattle::BindPlayerState(PlayerState& playerState)
{
    if (!playerState.IsInitialized() || playerState_ != nullptr)
    {
        return false;
    }

    playerState_ = &playerState;
    return true;
}

bool PlayerBattle::Init(ResourceManager& resources)
{
    const SpriteAsset* asset = resources.GetSprite(spriteId_);
    // A battle player is only valid after the arena connects it to the
    // persistent state used by battle mechanics, menus, and future saves.
    return playerState_ != nullptr
        && asset != nullptr
        && spriteAnimator_.Init(*asset, animationName_);
}

void PlayerBattle::Update(float deltaTime)
{
    if (active_)
    {
        spriteAnimator_.Update(deltaTime);
    }
}

void PlayerBattle::Render(
    const Graphics& graphics,
    const Camera& camera)
{
    if (active_)
    {
        spriteAnimator_.Render(
            graphics,
            x_ - static_cast<float>(camera.GetX()),
            y_ - static_cast<float>(camera.GetY()),
            scale_);
    }
}

const BattleStats& PlayerBattle::GetStats() const noexcept
{
    // BattleArenaScene binds the persistent state before object Init. Keeping
    // that as a required lifecycle step avoids a second, stale stats copy.
    assert(playerState_ != nullptr);
    return playerState_->GetBattleStats();
}

const std::vector<std::string>& PlayerBattle::GetMoveIds() const noexcept
{
    return moveIds_;
}

bool PlayerBattle::KnowsMove(std::string_view moveId) const noexcept
{
    return std::ranges::find(moveIds_, moveId) != moveIds_.end();
}

bool PlayerBattle::PlayBattleAnimation(
    const std::string& animationName,
    bool loop)
{
    return spriteAnimator_.PlayAnimation(animationName, loop);
}

void PlayerBattle::ReturnToIdle()
{
    spriteAnimator_.PlayAnimation(animationName_);
}

std::size_t PlayerBattle::GetAnimationFrameIndex() const noexcept
{
    return spriteAnimator_.GetFrameIndex();
}

bool PlayerBattle::IsAnimationFinished() const noexcept
{
    return spriteAnimator_.IsAnimationFinished();
}

void PlayerBattle::ApplyDamage(int amount) noexcept
{
    if (playerState_ != nullptr)
    {
        playerState_->SetHp(
            playerState_->GetHp() - std::max(0, amount));
    }
}

bool PlayerBattle::SpendMp(int amount) noexcept
{
    if (playerState_ == nullptr
        || amount < 0
        || amount > playerState_->GetMp())
    {
        return false;
    }
    playerState_->SetMp(playerState_->GetMp() - amount);
    return true;
}
