#pragma once

#include "../../Framework/Objects/BaseGameObject.h"

#include <vector>

class CollisionMask;
class GroundRegionMap;
struct GroundRegion;
struct InputState;

struct CollisionProbe
{
    float x = 0.0F;
    float y = 0.0F;
};

class Player final : public BaseGameObject
{
public:
    Player(
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
        std::vector<CollisionProbe> collisionProbes);

    bool Init(ResourceManager& resources) override;
    void ApplyInput(
        float deltaTime,
        const InputState& input,
        const CollisionMask& collisionMask,
        const GroundRegionMap& groundRegions);
    void Update(float deltaTime) override;
    void Render(const Graphics& graphics, const Camera& camera) override;
    float GetRenderSortKey() const noexcept override;
    float GetCameraX() const noexcept;
    float GetCameraY() const noexcept;
    float GetInteractionX() const noexcept;
    float GetInteractionY() const noexcept;
    float GetX() const noexcept;
    float GetY() const noexcept;
    void SetPosition(float x, float y) noexcept;
    bool GetWorldPosition(float& x, float& y) const noexcept override;
    bool SetWorldPosition(float x, float y) noexcept override;
    bool PlaySpriteAnimation(const std::string& animationName) override;
    const GroundRegion* FindGroundRegionUnderfoot(
        const CollisionMask& collisionMask,
        const GroundRegionMap& groundRegions) const noexcept;

private:
    enum class Facing
    {
        Left,
        Right,
        Up,
        Down
    };

    void PlayAnimation(const std::string& animationName);
    bool CanOccupy(
        float x,
        float y,
        const CollisionMask& collisionMask,
        const GroundRegionMap& groundRegions) const;
    const char* GetIdleAnimation() const noexcept;

    std::string spriteId_;
    std::string animationName_;
    float x_ = 0.0F;
    float y_ = 0.0F;
    float speed_ = 0.0F;
    float scale_ = 1.0F;
    float sortPointY_ = 0.0F;
    float cameraPointX_ = 0.0F;
    float cameraPointY_ = 0.0F;
    float interactionPointX_ = 0.0F;
    float interactionPointY_ = 0.0F;
    std::vector<CollisionProbe> collisionProbes_;
    Facing facing_ = Facing::Down;
};
