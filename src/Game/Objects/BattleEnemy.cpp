#include "BattleEnemy.h"

#include "../../Framework/Rendering/Camera.h"
#include "../../Framework/Resources/ResourceManager.h"

#include <algorithm>
#include <utility>

BattleEnemy::BattleEnemy(
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
    bool active)
    : BaseGameObject(std::move(id)),
      spriteId_(std::move(spriteId)),
      animationName_(std::move(animationName)),
      x_(x),
      y_(y),
      scale_(scale),
      stats_(std::move(stats)),
      moveIds_(std::move(moveIds)),
      battleScript_(std::move(battleScript)),
      experienceReward_(experienceReward),
      itemDropIds_(std::move(itemDropIds)),
      active_(active)
{
}

bool BattleEnemy::Init(ResourceManager& resources)
{
    const SpriteAsset* asset = resources.GetSprite(spriteId_);
    return asset != nullptr && spriteAnimator_.Init(*asset, animationName_);
}

void BattleEnemy::Update(float deltaTime)
{
    if (active_)
    {
        spriteAnimator_.Update(deltaTime);
    }
}

void BattleEnemy::Render(
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

const BattleStats& BattleEnemy::GetStats() const noexcept
{
    return stats_;
}

const std::vector<std::string>& BattleEnemy::GetMoveIds() const noexcept
{
    return moveIds_;
}

const std::string& BattleEnemy::GetBattleScript() const noexcept
{
    return battleScript_;
}

int BattleEnemy::GetExperienceReward() const noexcept
{
    return experienceReward_;
}

const std::vector<std::string>& BattleEnemy::GetItemDropIds() const noexcept
{
    return itemDropIds_;
}

bool BattleEnemy::KnowsMove(std::string_view moveId) const noexcept
{
    return std::ranges::find(moveIds_, moveId) != moveIds_.end();
}

bool BattleEnemy::PlayBattleAnimation(
    const std::string& animationName,
    bool loop)
{
    return spriteAnimator_.PlayAnimation(animationName, loop);
}

void BattleEnemy::ReturnToIdle()
{
    spriteAnimator_.PlayAnimation(animationName_);
}

std::size_t BattleEnemy::GetAnimationFrameIndex() const noexcept
{
    return spriteAnimator_.GetFrameIndex();
}

bool BattleEnemy::IsAnimationFinished() const noexcept
{
    return spriteAnimator_.IsAnimationFinished();
}

void BattleEnemy::ApplyDamage(int amount) noexcept
{
    stats_.hp = std::max(0, stats_.hp - std::max(0, amount));
    if (stats_.hp == 0)
    {
        active_ = false;
    }
}

bool BattleEnemy::SpendMp(int amount) noexcept
{
    if (amount < 0 || amount > stats_.mp)
    {
        return false;
    }
    stats_.mp -= amount;
    return true;
}

bool BattleEnemy::IsActive() const noexcept
{
    return active_;
}

void BattleEnemy::SetActive(bool active) noexcept
{
    active_ = active;
}
