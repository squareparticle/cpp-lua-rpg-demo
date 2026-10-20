#include "EndingScene.h"

#include "../../Platform/Graphics.h"
#include "../Scripting/LuaSystem.h"

#include <d2d1.h>
#include <algorithm>

EndingScene::EndingScene(ResourceManager& resources, LuaSystem& luaSystem)
    : BaseScene(resources, "assets/scenes/ending/map.yaml"),
      luaSystem_(luaSystem)
{
}

bool EndingScene::Init()
{
    return BaseScene::Init()
        && (GetSceneInitScript().empty()
            || luaSystem_.ExecuteSceneFile(GetSceneInitScript(), GetSceneId()));
}

void EndingScene::Update(float deltaTime, const InputState& input)
{
    BaseScene::Update(deltaTime, input);
    elapsedSeconds_ += std::max(deltaTime, 0.0F);
}

void EndingScene::Render(const Graphics& graphics)
{
    if (graphics.GetRenderTarget() == nullptr)
    {
        return;
    }
    const auto size = graphics.GetRenderTarget()->GetSize();
    const GraphicsRect bounds{0.0F, 0.0F, size.width, size.height};
    graphics.FillRectangle(bounds, {0x000000, 1.0F});
    graphics.DrawTextLayout("To Be Continued", bounds, 48.0F,
        ShowAllCharacters, {0xFFFFFF, std::clamp(elapsedSeconds_, 0.0F, 1.0F)},
        true);
}
