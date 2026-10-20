#pragma once

class Camera;
class Graphics;
class ResourceManager;

class BaseLayer
{
public:
    virtual ~BaseLayer() = default;

    virtual bool Init(ResourceManager& resources);
    virtual void Update(float deltaTime);
    virtual void Render(const Graphics& graphics, const Camera& camera);
    virtual void Shutdown();
};
