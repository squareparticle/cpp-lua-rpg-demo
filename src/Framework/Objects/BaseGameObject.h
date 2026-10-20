#pragma once

#include "../Rendering/SpriteAnimator.h"

#include <string>

class Camera;
class Graphics;
class ResourceManager;

class BaseGameObject
{
public:
    BaseGameObject() = default;
    explicit BaseGameObject(std::string id);
    virtual ~BaseGameObject() = default;

    const std::string& GetId() const noexcept;

    virtual bool Init(ResourceManager& resources);
    virtual void Update(float deltaTime);
    virtual void Render(const Graphics& graphics, const Camera& camera);
    virtual float GetRenderSortKey() const noexcept;
    virtual bool GetWorldPosition(float& x, float& y) const noexcept;
    virtual bool SetWorldPosition(float x, float y) noexcept;
    virtual bool PlaySpriteAnimation(const std::string& animationName);
    virtual bool BeginFadeOut(float durationSeconds) noexcept;
    virtual bool IsFadeOutComplete() const noexcept;
    virtual void Shutdown();

protected:
    SpriteAnimator spriteAnimator_;

private:
    std::string id_;
};
