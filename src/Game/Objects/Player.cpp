#include "Player.h"

#include "../../Framework/Rendering/Camera.h"
#include "../../Framework/Collision/CollisionMask.h"
#include "../../Platform/Graphics.h"
#include "../World/GroundRegionMap.h"
#include "../../Framework/Resources/ResourceManager.h"
#include "../Input/InputState.h"

#include <cmath>
#include <utility>

Player::Player(
    std::string id,
    std::string spriteId,
    std::string animationName,
    float x,
    float y,
    float speed,
    float scale,
    float sortPointY,
    float cameraPointX,
    float cameraPointY,
    float interactionPointX,
    float interactionPointY,
    std::vector<CollisionProbe> collisionProbes)
    : BaseGameObject(std::move(id)),
      spriteId_(std::move(spriteId)),
      animationName_(std::move(animationName)),
      x_(x),
      y_(y),
      speed_(speed),
      scale_(scale),
      sortPointY_(sortPointY),
      cameraPointX_(cameraPointX),
      cameraPointY_(cameraPointY),
      interactionPointX_(interactionPointX),
      interactionPointY_(interactionPointY),
      collisionProbes_(std::move(collisionProbes))
{
}

bool Player::Init(ResourceManager& resources)
{
    const SpriteAsset* asset = resources.GetSprite(spriteId_);
    return asset != nullptr && spriteAnimator_.Init(*asset, animationName_);
}

void Player::ApplyInput(
    float deltaTime,
    const InputState& input,
    const CollisionMask& collisionMask,
    const GroundRegionMap& groundRegions)
{
    float movementX = static_cast<float>(input.rightHeld)
        - static_cast<float>(input.leftHeld);
    float movementY = static_cast<float>(input.downHeld)
        - static_cast<float>(input.upHeld);

    if (movementX == 0.0F && movementY == 0.0F)
    {
        PlayAnimation(GetIdleAnimation());
        return;
    }

    if (movementX != 0.0F && movementY != 0.0F)
    {
        constexpr float diagonalScale = 0.70710678F;
        movementX *= diagonalScale;
        movementY *= diagonalScale;
    }

    const float deltaX = movementX * speed_ * deltaTime;
    const float deltaY = movementY * speed_ * deltaTime;

    if (CanOccupy(x_ + deltaX, y_, collisionMask, groundRegions))
    {
        x_ += deltaX;
    }
    if (CanOccupy(x_, y_ + deltaY, collisionMask, groundRegions))
    {
        y_ += deltaY;
    }

    if (movementY < 0.0F)
    {
        facing_ = Facing::Up;
        PlayAnimation("walkUp");
    }
    else if (movementY > 0.0F)
    {
        facing_ = Facing::Down;
        PlayAnimation("walkDown");
    }
    else if (movementX < 0.0F)
    {
        facing_ = Facing::Left;
        PlayAnimation("walkLeft");
    }
    else
    {
        facing_ = Facing::Right;
        PlayAnimation("walkRight");
    }
}

void Player::Update(float deltaTime)
{
    spriteAnimator_.Update(deltaTime);
}

void Player::Render(const Graphics& graphics, const Camera& camera)
{
    spriteAnimator_.Render(
        graphics,
        x_ - static_cast<float>(camera.GetX()),
        y_ - static_cast<float>(camera.GetY()),
        scale_);
}

float Player::GetRenderSortKey() const noexcept
{
    // Visual depth is authored independently from collision and interaction.
    // For a standing character this point normally sits at the feet.
    return y_ + sortPointY_ * scale_;
}

float Player::GetCameraX() const noexcept
{
    return x_ + cameraPointX_ * scale_;
}

float Player::GetCameraY() const noexcept
{
    return y_ + cameraPointY_ * scale_;
}

float Player::GetInteractionX() const noexcept
{
    // Interaction has its own authored anchor. Collision probes answer whether
    // movement is legal and may change independently from where the player
    // should be considered to stand when talking to nearby objects.
    return x_ + interactionPointX_ * scale_;
}

float Player::GetInteractionY() const noexcept
{
    return y_ + interactionPointY_ * scale_;
}

float Player::GetX() const noexcept
{
    return x_;
}

float Player::GetY() const noexcept
{
    return y_;
}

void Player::SetPosition(float x, float y) noexcept
{
    x_ = x;
    y_ = y;
}

bool Player::GetWorldPosition(float& x, float& y) const noexcept
{
    x = x_;
    y = y_;
    return true;
}

bool Player::SetWorldPosition(float x, float y) noexcept
{
    SetPosition(x, y);
    return true;
}

bool Player::PlaySpriteAnimation(const std::string& animationName)
{
    if (!BaseGameObject::PlaySpriteAnimation(animationName))
    {
        return false;
    }
    animationName_ = animationName;
    return true;
}

const GroundRegion* Player::FindGroundRegionUnderfoot(
    const CollisionMask& collisionMask,
    const GroundRegionMap& groundRegions) const noexcept
{
    for (const CollisionProbe& probe : collisionProbes_)
    {
        const int sampleX = static_cast<int>(
            std::floor(x_ + probe.x * scale_));
        const int sampleY = static_cast<int>(
            std::floor(y_ + probe.y * scale_));
        const std::optional<std::uint32_t> color =
            collisionMask.GetColorAt(sampleX, sampleY);
        if (color.has_value())
        {
            const GroundRegion* region = groundRegions.FindByColor(*color);
            if (region != nullptr)
            {
                return region;
            }
        }
    }

    return nullptr;
}

void Player::PlayAnimation(const std::string& animationName)
{
    if (animationName != animationName_ && spriteAnimator_.PlayAnimation(animationName))
    {
        animationName_ = animationName;
    }
}

bool Player::CanOccupy(
    float x,
    float y,
    const CollisionMask& collisionMask,
    const GroundRegionMap& groundRegions) const
{
    for (const CollisionProbe& probe : collisionProbes_)
    {
        const int sampleX = static_cast<int>(std::floor(x + probe.x * scale_));
        const int sampleY = static_cast<int>(std::floor(y + probe.y * scale_));
        const std::optional<std::uint32_t> color =
            collisionMask.GetColorAt(sampleX, sampleY);
        // Black and out-of-bounds are permanently blocked. Other authored
        // colors defer to the scene's mutable semantic-region table.
        if (!color.has_value()
            || *color == CollisionMask::BlockedColor
            || groundRegions.BlocksMovement(*color))
        {
            return false;
        }
    }

    return true;
}

const char* Player::GetIdleAnimation() const noexcept
{
    switch (facing_)
    {
    case Facing::Left:
        return "idleLeft";
    case Facing::Right:
        return "idleRight";
    case Facing::Up:
        return "idleUp";
    case Facing::Down:
    default:
        return "idleDown";
    }
}
