#include "AppConfig.h"

#include <yaml-cpp/yaml.h>

#include <cmath>
#include <fstream>
#include <string>

namespace
{
constexpr int MinimumDimension = 1;
constexpr int MaximumDimension = 16384;

bool IsValidDimension(int dimension)
{
    return dimension >= MinimumDimension && dimension <= MaximumDimension;
}
}

bool LoadAppConfig(
    const std::filesystem::path& filename,
    GameConfig& gameConfig,
    WindowConfig& windowConfig,
    RenderConfig& renderConfig,
    DialogConfig& dialogConfig)
{
    try
    {
        std::ifstream input(filename);
        if (!input)
        {
            return false;
        }

        const YAML::Node root = YAML::Load(input);
        const YAML::Node game = root["game"];
        const YAML::Node window = root["window"];
        const YAML::Node render = root["render"];
        const YAML::Node dialog = root["dialog"];
        if (!game || !game.IsMap()
            || !window || !window.IsMap()
            || !render || !render.IsMap()
            || !dialog || !dialog.IsMap())
        {
            return false;
        }

        const std::string startMode = game["startMode"].as<std::string>();
        if (startMode == "scene")
        {
            gameConfig.startMode = GameStartMode::Scene;
        }
        else if (startMode == "saveOrScene")
        {
            gameConfig.startMode = GameStartMode::SaveOrScene;
        }
        else
        {
            return false;
        }

        gameConfig.startScene = game["startScene"].as<std::string>();
        if (gameConfig.startScene.empty())
        {
            return false;
        }

        const std::string mode = window["mode"].as<std::string>();
        if (mode == "windowed")
        {
            windowConfig.mode = WindowMode::Windowed;
        }
        else if (mode == "fullscreen")
        {
            windowConfig.mode = WindowMode::Fullscreen;
        }
        else if (mode == "borderless")
        {
            windowConfig.mode = WindowMode::Borderless;
        }
        else
        {
            return false;
        }

        windowConfig.width = window["width"].as<int>();
        windowConfig.height = window["height"].as<int>();

        renderConfig.width = render["width"].as<int>();
        renderConfig.height = render["height"].as<int>();

        const std::string scaling = render["scaling"].as<std::string>();
        if (scaling == "fit")
        {
            renderConfig.scaling = ScalingMode::Fit;
        }
        else if (scaling == "stretch")
        {
            renderConfig.scaling = ScalingMode::Stretch;
        }
        else
        {
            return false;
        }

        const std::string filtering = render["filtering"].as<std::string>();
        if (filtering == "nearest")
        {
            renderConfig.filtering = ScalingFilter::Nearest;
        }
        else if (filtering == "linear")
        {
            renderConfig.filtering = ScalingFilter::Linear;
        }
        else
        {
            return false;
        }

        dialogConfig.typewriterCharactersPerSecond =
            dialog["typewriterCharactersPerSecond"].as<float>();

        return IsValidDimension(windowConfig.width) &&
            IsValidDimension(windowConfig.height) &&
            IsValidDimension(renderConfig.width) &&
            IsValidDimension(renderConfig.height) &&
            std::isfinite(dialogConfig.typewriterCharactersPerSecond) &&
            dialogConfig.typewriterCharactersPerSecond > 0.0F;
    }
    catch (const YAML::Exception&)
    {
        return false;
    }
}
