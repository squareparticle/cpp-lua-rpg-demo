#include "../src/Application/AppConfig.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

namespace
{
bool Expect(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << message << '\n';
        return false;
    }
    return true;
}

bool WriteConfig(
    const std::filesystem::path& path,
    std::string_view startMode)
{
    std::ofstream file(path, std::ios::trunc);
    file
        << "game:\n"
        << "  startMode: " << startMode << "\n"
        << "  startScene: storageRoom\n"
        << "window:\n"
        << "  mode: windowed\n"
        << "  width: 1280\n"
        << "  height: 720\n"
        << "render:\n"
        << "  width: 1280\n"
        << "  height: 720\n"
        << "  scaling: fit\n"
        << "  filtering: nearest\n"
        << "dialog:\n"
        << "  typewriterCharactersPerSecond: 42\n";
    return file.good();
}
}

int main()
{
    const std::filesystem::path path = "AppConfigTests.config.yaml";
    std::error_code error;
    GameConfig game;
    WindowConfig window;
    RenderConfig render;
    DialogConfig dialog;

    if (!WriteConfig(path, "scene")
        || !Expect(LoadAppConfig(path, game, window, render, dialog),
            "Could not load scene startup mode")
        || !Expect(game.startMode == GameStartMode::Scene
                && game.startScene == "storageRoom",
            "Scene startup mode loaded incorrect values"))
    {
        std::filesystem::remove(path, error);
        return 1;
    }

    if (!WriteConfig(path, "saveOrScene")
        || !Expect(LoadAppConfig(path, game, window, render, dialog),
            "Could not load save-or-scene startup mode")
        || !Expect(game.startMode == GameStartMode::SaveOrScene,
            "Save-or-scene startup mode loaded incorrectly"))
    {
        std::filesystem::remove(path, error);
        return 1;
    }

    const bool invalidRejected = WriteConfig(path, "invalid")
        && !LoadAppConfig(path, game, window, render, dialog);
    std::filesystem::remove(path, error);
    return Expect(invalidRejected, "Invalid startup mode was accepted")
        ? 0
        : 1;
}
