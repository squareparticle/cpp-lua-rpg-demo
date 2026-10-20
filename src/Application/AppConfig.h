#pragma once

#include <filesystem>
#include <string>

enum class GameStartMode
{
    Scene,
    SaveOrScene
};

struct GameConfig
{
    GameStartMode startMode = GameStartMode::Scene;
    std::string startScene = "boatScene";
};

enum class WindowMode
{
    Windowed,
    Fullscreen,
    Borderless
};

enum class ScalingMode
{
    Fit,
    Stretch
};

enum class ScalingFilter
{
    Nearest,
    Linear
};

struct WindowConfig
{
    WindowMode mode = WindowMode::Windowed;
    int width = 1280;
    int height = 720;
};

struct RenderConfig
{
    int width = 1280;
    int height = 720;
    ScalingMode scaling = ScalingMode::Fit;
    ScalingFilter filtering = ScalingFilter::Nearest;
};

struct DialogConfig
{
    float typewriterCharactersPerSecond = 42.0F;
};

bool LoadAppConfig(
    const std::filesystem::path& filename,
    GameConfig& gameConfig,
    WindowConfig& windowConfig,
    RenderConfig& renderConfig,
    DialogConfig& dialogConfig);
