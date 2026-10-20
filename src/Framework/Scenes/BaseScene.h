#pragma once

#include "SceneChangeRequest.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

class BaseLayer;
class BaseGameObject;
class ObjectLayer;
class Camera;
class Graphics;
class ResourceManager;
struct InputState;

class BaseScene
{
public:
    explicit BaseScene(ResourceManager& resources, std::string mapPath);
    virtual ~BaseScene();

    BaseScene(const BaseScene&) = delete;
    BaseScene& operator=(const BaseScene&) = delete;

    const std::string& GetMapPath() const noexcept;
    const std::string& GetSceneId() const noexcept;
    const std::string& GetMusicId() const noexcept;
    bool SetSceneMetadata(
        std::string id,
        std::string initScript,
        std::string musicId);
    bool AddLayer(std::string id, std::unique_ptr<BaseLayer> layer);
    bool SetCameraWindow(
        float x,
        float y,
        float width,
        float height,
        float viewportWidth,
        float viewportHeight) noexcept;
    virtual bool SetCameraTarget(std::string_view objectId);
    // Called before layer ownership is transferred. Specialized scenes use it
    // to retain non-owning pointers to object types they coordinate.
    virtual bool OnGameObjectCreated(
        BaseGameObject& object,
        ObjectLayer& owner);

    virtual bool Init();
    virtual void Update(float deltaTime, const InputState& input);
    virtual void Render(const Graphics& graphics);
    virtual void Shutdown();
    virtual std::optional<SceneChangeRequest>
        TakeSceneChangeRequest();

protected:
    const std::string& GetSceneInitScript() const noexcept;
    Camera* GetCurrentCamera() noexcept;
    const Camera* GetCurrentCamera() const noexcept;
    BaseLayer* FindLayer(std::string_view id) noexcept;

    ResourceManager& resources_;
    std::string mapPath_;

private:
    std::string sceneId_;
    std::string sceneInitScript_;
    std::string musicId_;
    std::unique_ptr<Camera> currentCamera_;
    std::vector<std::unique_ptr<BaseLayer>> layers_;
    std::unordered_map<std::string, BaseLayer*> layersById_;
};
