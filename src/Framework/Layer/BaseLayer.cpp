#include "BaseLayer.h"

#include "../Rendering/Camera.h"
#include "../../Platform/Graphics.h"
#include "../Resources/ResourceManager.h"

bool BaseLayer::Init(ResourceManager& resources)
{
    static_cast<void>(resources);
    return true;
}

void BaseLayer::Update(float deltaTime)
{
    static_cast<void>(deltaTime);
}

void BaseLayer::Render(const Graphics& graphics, const Camera& camera)
{
    static_cast<void>(graphics);
    static_cast<void>(camera);
}

void BaseLayer::Shutdown()
{
}
