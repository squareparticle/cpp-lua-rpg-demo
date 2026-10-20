#pragma once

#include "BaseLayer.h"

#include <memory>
#include <string_view>
#include <vector>

class BaseGameObject;

class ObjectLayer final : public BaseLayer
{
public:
    explicit ObjectLayer(bool sortByY = false);
    ~ObjectLayer() override;

    bool AddObject(std::unique_ptr<BaseGameObject> object);
    BaseGameObject* FindObject(std::string_view id) noexcept;
    bool RemoveObject(std::string_view id);

    bool Init(ResourceManager& resources) override;
    void Update(float deltaTime) override;
    void Render(const Graphics& graphics, const Camera& camera) override;
    void Shutdown() override;

private:
    std::vector<std::unique_ptr<BaseGameObject>> objects_;
    bool sortByY_ = false;
};
