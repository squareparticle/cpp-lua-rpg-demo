#include "BaseScene.h"

#include "../Rendering/Camera.h"
#include "../../Platform/Graphics.h"
#include "../Layer/BaseLayer.h"

#include <memory>
#include <utility>

BaseScene::BaseScene(ResourceManager& resources, std::string mapPath)
    : resources_(resources),
      mapPath_(std::move(mapPath)),
      currentCamera_(std::make_unique<Camera>())
{
}

BaseScene::~BaseScene() = default;

const std::string& BaseScene::GetMapPath() const noexcept
{
    return mapPath_;
}

bool BaseScene::SetSceneMetadata(
    std::string id,
    std::string initScript,
    std::string musicId)
{
    if (id.empty() || !sceneId_.empty())
    {
        return false;
    }

    sceneId_ = std::move(id);
    sceneInitScript_ = std::move(initScript);
    musicId_ = std::move(musicId);
    return true;
}

bool BaseScene::AddLayer(std::string id, std::unique_ptr<BaseLayer> layer)
{
    if (id.empty() || layer == nullptr || layersById_.contains(id))
    {
        return false;
    }

    BaseLayer* layerPointer = layer.get();
    layers_.push_back(std::move(layer));
    layersById_.emplace(std::move(id), layerPointer);
    return true;
}

bool BaseScene::SetCameraWindow(
    float x,
    float y,
    float width,
    float height,
    float viewportWidth,
    float viewportHeight) noexcept
{
    return currentCamera_->SetWindow(
        x,
        y,
        width,
        height,
        viewportWidth,
        viewportHeight);
}

bool BaseScene::SetCameraTarget(std::string_view objectId)
{
    static_cast<void>(objectId);
    return false;
}

bool BaseScene::OnGameObjectCreated(
    BaseGameObject& object,
    ObjectLayer& owner)
{
    static_cast<void>(object);
    static_cast<void>(owner);
    return true;
}

bool BaseScene::Init()
{
    if (!currentCamera_->Init())
    {
        return false;
    }

    for (const auto& layer : layers_)
    {
        if (!layer->Init(resources_))
        {
            return false;
        }
    }

    return true;
}

void BaseScene::Update(float deltaTime, const InputState& input)
{
    static_cast<void>(input);

    for (const auto& layer : layers_)
    {
        layer->Update(deltaTime);
    }
}

void BaseScene::Render(const Graphics& graphics)
{
    if (currentCamera_ == nullptr)
    {
        return;
    }

    for (const auto& layer : layers_)
    {
        layer->Render(graphics, *currentCamera_);
    }
}

void BaseScene::Shutdown()
{
    for (const auto& layer : layers_)
    {
        layer->Shutdown();
    }
}

std::optional<SceneChangeRequest>
BaseScene::TakeSceneChangeRequest()
{
    return std::nullopt;
}

const std::string& BaseScene::GetSceneId() const noexcept
{
    return sceneId_;
}

const std::string& BaseScene::GetMusicId() const noexcept
{
    return musicId_;
}

const std::string& BaseScene::GetSceneInitScript() const noexcept
{
    return sceneInitScript_;
}

const Camera* BaseScene::GetCurrentCamera() const noexcept
{
    return currentCamera_.get();
}

Camera* BaseScene::GetCurrentCamera() noexcept
{
    return currentCamera_.get();
}

BaseLayer* BaseScene::FindLayer(std::string_view id) noexcept
{
    const auto layer = layersById_.find(std::string(id));
    return layer != layersById_.end() ? layer->second : nullptr;
}
