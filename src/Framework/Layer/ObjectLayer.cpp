#include "ObjectLayer.h"

#include "../Objects/BaseGameObject.h"

#include <algorithm>
#include <utility>

ObjectLayer::ObjectLayer(bool sortByY)
    : sortByY_(sortByY)
{
}
ObjectLayer::~ObjectLayer() = default;

bool ObjectLayer::AddObject(std::unique_ptr<BaseGameObject> object)
{
    if (object == nullptr || FindObject(object->GetId()) != nullptr)
    {
        return false;
    }

    objects_.push_back(std::move(object));
    return true;
}

BaseGameObject* ObjectLayer::FindObject(std::string_view id) noexcept
{
    const auto object = std::ranges::find_if(
        objects_,
        [id](const std::unique_ptr<BaseGameObject>& candidate)
        {
            return candidate->GetId() == id;
        });
    return object != objects_.end() ? object->get() : nullptr;
}

bool ObjectLayer::RemoveObject(std::string_view id)
{
    const auto object = std::ranges::find_if(
        objects_,
        [id](const std::unique_ptr<BaseGameObject>& candidate)
        {
            return candidate->GetId() == id;
        });
    if (object == objects_.end())
    {
        return false;
    }

    (*object)->Shutdown();
    objects_.erase(object);
    return true;
}

bool ObjectLayer::Init(ResourceManager& resources)
{
    for (const auto& object : objects_)
    {
        if (!object->Init(resources))
        {
            return false;
        }
    }

    return true;
}

void ObjectLayer::Update(float deltaTime)
{
    for (const auto& object : objects_)
    {
        object->Update(deltaTime);
    }
}

void ObjectLayer::Render(const Graphics& graphics, const Camera& camera)
{
    if (sortByY_)
    {
        std::vector<BaseGameObject*> renderOrder;
        renderOrder.reserve(objects_.size());
        for (const auto& object : objects_)
        {
            renderOrder.push_back(object.get());
        }

        // Objects move, so their order is recalculated each frame. Stable sort
        // preserves YAML placement order when two feet share the same Y value.
        std::stable_sort(
            renderOrder.begin(),
            renderOrder.end(),
            [](const BaseGameObject* left, const BaseGameObject* right)
            {
                return left->GetRenderSortKey() < right->GetRenderSortKey();
            });

        for (BaseGameObject* object : renderOrder)
        {
            object->Render(graphics, camera);
        }
        return;
    }

    for (const auto& object : objects_)
    {
        object->Render(graphics, camera);
    }
}

void ObjectLayer::Shutdown()
{
    for (const auto& object : objects_)
    {
        object->Shutdown();
    }
}
