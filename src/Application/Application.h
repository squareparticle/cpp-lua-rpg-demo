#pragma once

#include "AppConfig.h"
#include "../Game/Input/InputState.h"
#include "../Platform/Input/DeviceInput.h"

#include <d2d1.h>
#include <windows.h>
#include <wrl/client.h>

#include <memory>

class GameController;

class Application
{
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool Initialize(HINSTANCE instance, int showCommand);
    int Run();

private:
    static LRESULT CALLBACK WindowProcedure(
        HWND window,
        UINT message,
        WPARAM wParam,
        LPARAM lParam);

    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);
    bool CreateMainWindow(int showCommand);
    bool CreateRenderTargets();
    bool ConfigureInput();
    InputState BuildInputState() const;
    bool LoadConfig();
    void DiscardRenderTargets();
    void Render();
    void Shutdown();

    HINSTANCE instance_ = nullptr;
    HWND window_ = nullptr;
    Microsoft::WRL::ComPtr<ID2D1Factory> direct2DFactory_;
    Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> windowRenderTarget_;
    Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> gameRenderTarget_;
    Microsoft::WRL::ComPtr<ID2D1Bitmap> gameSurface_;
    std::unique_ptr<GameController> gameController_;
    GameConfig gameConfig_;
    WindowConfig windowConfig_;
    RenderConfig renderConfig_;
    DialogConfig dialogConfig_;
    DeviceInput deviceInput_;
    bool windowClassRegistered_ = false;
    bool comInitialized_ = false;
};
