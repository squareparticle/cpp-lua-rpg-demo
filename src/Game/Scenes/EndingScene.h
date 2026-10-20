#pragma once

#include "../../Framework/Scenes/BaseScene.h"

class LuaSystem;

class EndingScene final : public BaseScene
{
public:
    EndingScene(ResourceManager& resources, LuaSystem& luaSystem);
    bool Init() override;
    void Update(float deltaTime, const InputState& input) override;
    void Render(const Graphics& graphics) override;

private:
    LuaSystem& luaSystem_;
    float elapsedSeconds_ = 0.0F;
};
