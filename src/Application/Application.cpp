#include "Application.h"

#include "../Game/GameController.h"
#include "../Game/Input/InputActions.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <memory>
#include <system_error>
#include <vector>

namespace
{
constexpr wchar_t WindowClassName[] = L"RPG2002ShipWindowClass";
constexpr wchar_t WindowTitle[] = L"RPG2002-Ship";
}

Application::Application() = default;

Application::~Application()
{
    Shutdown();
}

bool Application::Initialize(HINSTANCE instance, int showCommand)
{
    instance_ = instance;

    if (!ConfigureInput())
    {
        return false;
    }

    if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2)
        && GetLastError() != ERROR_ACCESS_DENIED)
    {
        return false;
    }

    if (!LoadConfig())
    {
        MessageBoxW(
            nullptr,
            L"Could not load config.yaml. Check the game, window, render, and dialog settings.",
            WindowTitle,
            MB_OK | MB_ICONERROR);
        return false;
    }

    const HRESULT comResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(comResult))
    {
        return false;
    }
    comInitialized_ = true;

    if (FAILED(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            direct2DFactory_.GetAddressOf())))
    {
        return false;
    }

    if (!CreateMainWindow(showCommand) || !CreateRenderTargets())
    {
        Shutdown();
        return false;
    }

    gameController_ = std::make_unique<GameController>(
        dialogConfig_.typewriterCharactersPerSecond);
    if (!gameController_->InitGame(
            gameRenderTarget_.Get(),
            gameConfig_.startScene,
            gameConfig_.startMode == GameStartMode::SaveOrScene))
    {
        Shutdown();
        return false;
    }

    return true;
}

int Application::Run()
{
    using Clock = std::chrono::steady_clock;

    MSG message{};
    Clock::time_point previousUpdateTime = Clock::now();

    while (message.message != WM_QUIT)
    {
        if (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        else
        {
            const Clock::time_point now = Clock::now();
            const float deltaTime = std::chrono::duration<float>(
                now - previousUpdateTime).count();
            previousUpdateTime = now;

            const InputState input = BuildInputState();
            gameController_->Update(deltaTime, input);
            // ConsumeQuitRequest returns true for one update when the menu has
            // requested shutdown. It clears the request after reading it; the
            // true value saved here still tells Application to close its
            // Win32 window below.
            const bool quitRequested = gameController_->ConsumeQuitRequest();
            // Held actions remain active until key-up. Press edges are
            // consumed after exactly one game update.
            deviceInput_.EndFrame();
            if (quitRequested)
            {
                DestroyWindow(window_);
                continue;
            }
            Render();
        }
    }

    return static_cast<int>(message.wParam);
}

LRESULT CALLBACK Application::WindowProcedure(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    Application* application = nullptr;

    if (message == WM_NCCREATE)
    {
        const auto* createData = reinterpret_cast<CREATESTRUCTW*>(lParam);
        application = static_cast<Application*>(createData->lpCreateParams);
        application->window_ = window;
        SetWindowLongPtrW(
            window,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(application));
    }
    else
    {
        application = reinterpret_cast<Application*>(
            GetWindowLongPtrW(window, GWLP_USERDATA));
    }

    if (application != nullptr)
    {
        const LRESULT result = application->HandleMessage(message, wParam, lParam);

        if (message == WM_NCDESTROY)
        {
            SetWindowLongPtrW(window, GWLP_USERDATA, 0);
            application->window_ = nullptr;
        }

        return result;
    }

    return DefWindowProcW(window, message, wParam, lParam);
}

LRESULT Application::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
        deviceInput_.HandleKeyDown(static_cast<std::uint32_t>(wParam));
        return 0;

    case WM_KEYUP:
        deviceInput_.HandleKeyUp(static_cast<std::uint32_t>(wParam));
        return 0;

    case WM_KILLFOCUS:
        deviceInput_.Reset();
        return 0;

    case WM_SIZE:
        if (windowRenderTarget_ != nullptr && wParam != SIZE_MINIMIZED)
        {
            const D2D1_SIZE_U size = D2D1::SizeU(
                LOWORD(lParam),
                HIWORD(lParam));

            if (windowRenderTarget_->Resize(size) == D2DERR_RECREATE_TARGET)
            {
                DiscardRenderTargets();
            }
        }
        return 0;

    case WM_DPICHANGED:
        if (windowRenderTarget_ != nullptr)
        {
            const auto dpiX = static_cast<float>(LOWORD(wParam));
            const auto dpiY = static_cast<float>(HIWORD(wParam));
            windowRenderTarget_->SetDpi(dpiX, dpiY);
        }

        if (windowConfig_.mode != WindowMode::Fullscreen)
        {
            const auto* suggestedRectangle = reinterpret_cast<RECT*>(lParam);
            SetWindowPos(
                window_,
                nullptr,
                suggestedRectangle->left,
                suggestedRectangle->top,
                suggestedRectangle->right - suggestedRectangle->left,
                suggestedRectangle->bottom - suggestedRectangle->top,
                SWP_NOACTIVATE | SWP_NOZORDER);
        }
        return 0;

    case WM_PAINT:
        ValidateRect(window_, nullptr);
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        break;
    }

    return DefWindowProcW(window_, message, wParam, lParam);
}

bool Application::ConfigureInput()
{
    return deviceInput_.BindKey(InputActions::MoveLeft, VK_LEFT)
        && deviceInput_.BindKey(InputActions::MoveLeft, 'A')
        && deviceInput_.BindKey(InputActions::MoveRight, VK_RIGHT)
        && deviceInput_.BindKey(InputActions::MoveRight, 'D')
        && deviceInput_.BindKey(InputActions::MoveUp, VK_UP)
        && deviceInput_.BindKey(InputActions::MoveUp, 'W')
        && deviceInput_.BindKey(InputActions::MoveDown, VK_DOWN)
        && deviceInput_.BindKey(InputActions::MoveDown, 'S')
        && deviceInput_.BindKey(InputActions::NavigateUp, VK_UP)
        && deviceInput_.BindKey(InputActions::NavigateUp, 'W')
        && deviceInput_.BindKey(InputActions::NavigateDown, VK_DOWN)
        && deviceInput_.BindKey(InputActions::NavigateDown, 'S')
        && deviceInput_.BindKey(InputActions::PrimaryAction, 'E')
        && deviceInput_.BindKey(InputActions::Menu, VK_ESCAPE)
        && deviceInput_.BindKey(InputActions::Inventory, 'I')
        && deviceInput_.BindKey(InputActions::Confirm, VK_RETURN);
}

InputState Application::BuildInputState() const
{
    return InputState{
        .leftHeld = deviceInput_.IsActionHeld(InputActions::MoveLeft),
        .rightHeld = deviceInput_.IsActionHeld(InputActions::MoveRight),
        .upHeld = deviceInput_.IsActionHeld(InputActions::MoveUp),
        .downHeld = deviceInput_.IsActionHeld(InputActions::MoveDown),
        .navigateUpPressed =
            deviceInput_.WasActionPressed(InputActions::NavigateUp),
        .navigateDownPressed =
            deviceInput_.WasActionPressed(InputActions::NavigateDown),
        .primaryActionPressed =
            deviceInput_.WasActionPressed(InputActions::PrimaryAction),
        .menuPressed = deviceInput_.WasActionPressed(InputActions::Menu),
        .inventoryPressed =
            deviceInput_.WasActionPressed(InputActions::Inventory),
        .confirmPressed = deviceInput_.WasActionPressed(InputActions::Confirm)};
}

bool Application::CreateMainWindow(int showCommand)
{
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProcedure;
    windowClass.hInstance = instance_;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = WindowClassName;

    if (RegisterClassExW(&windowClass) == 0)
    {
        return false;
    }
    windowClassRegistered_ = true;

    const DWORD windowStyle = windowConfig_.mode == WindowMode::Windowed
        ? WS_OVERLAPPEDWINDOW
        : WS_POPUP;

    const UINT dpi = GetDpiForSystem();
    const int requestedClientWidth = MulDiv(windowConfig_.width, dpi, 96);
    const int requestedClientHeight = MulDiv(windowConfig_.height, dpi, 96);
    RECT windowRectangle{0, 0, requestedClientWidth, requestedClientHeight};
    if (windowConfig_.mode == WindowMode::Windowed &&
        !AdjustWindowRectExForDpi(
            &windowRectangle,
            windowStyle,
            FALSE,
            0,
            dpi))
    {
        return false;
    }

    int windowX = CW_USEDEFAULT;
    int windowY = CW_USEDEFAULT;
    int windowWidth = windowRectangle.right - windowRectangle.left;
    int windowHeight = windowRectangle.bottom - windowRectangle.top;

    if (windowConfig_.mode != WindowMode::Windowed)
    {
        MONITORINFO monitorInfo{};
        monitorInfo.cbSize = sizeof(monitorInfo);
        const HMONITOR monitor = MonitorFromPoint({0, 0}, MONITOR_DEFAULTTOPRIMARY);
        if (!GetMonitorInfoW(monitor, &monitorInfo))
        {
            return false;
        }

        const RECT& bounds = windowConfig_.mode == WindowMode::Fullscreen
            ? monitorInfo.rcMonitor
            : monitorInfo.rcWork;

        if (windowConfig_.mode == WindowMode::Fullscreen)
        {
            windowX = bounds.left;
            windowY = bounds.top;
            windowWidth = bounds.right - bounds.left;
            windowHeight = bounds.bottom - bounds.top;
        }
        else
        {
            windowWidth = std::min(
                windowWidth,
                static_cast<int>(bounds.right - bounds.left));
            windowHeight = std::min(
                windowHeight,
                static_cast<int>(bounds.bottom - bounds.top));
            windowX = bounds.left + ((bounds.right - bounds.left) - windowWidth) / 2;
            windowY = bounds.top + ((bounds.bottom - bounds.top) - windowHeight) / 2;
        }
    }

    window_ = CreateWindowExW(
        0,
        WindowClassName,
        WindowTitle,
        windowStyle,
        windowX,
        windowY,
        windowWidth,
        windowHeight,
        nullptr,
        nullptr,
        instance_,
        this);

    if (window_ == nullptr)
    {
        return false;
    }

    ShowWindow(
        window_,
        windowConfig_.mode == WindowMode::Windowed ? showCommand : SW_SHOW);
    UpdateWindow(window_);
    return true;
}

bool Application::LoadConfig()
{
    std::error_code currentPathError;
    const std::filesystem::path workingDirectoryConfig =
        std::filesystem::current_path(currentPathError) / "config.yaml";
    if (!currentPathError && std::filesystem::exists(workingDirectoryConfig))
    {
        return LoadAppConfig(
            workingDirectoryConfig,
            gameConfig_,
            windowConfig_,
            renderConfig_,
            dialogConfig_);
    }

    std::vector<wchar_t> executablePath(MAX_PATH);
    DWORD pathLength = 0;

    while (true)
    {
        pathLength = GetModuleFileNameW(
            nullptr,
            executablePath.data(),
            static_cast<DWORD>(executablePath.size()));
        if (pathLength == 0)
        {
            return false;
        }

        if (pathLength < executablePath.size())
        {
            break;
        }

        executablePath.resize(executablePath.size() * 2);
    }

    const std::filesystem::path configPath =
        std::filesystem::path(executablePath.data()).parent_path() / "config.yaml";
    return LoadAppConfig(
        configPath,
        gameConfig_,
        windowConfig_,
        renderConfig_,
        dialogConfig_);
}

bool Application::CreateRenderTargets()
{
    if (windowRenderTarget_ != nullptr &&
        gameRenderTarget_ != nullptr &&
        gameSurface_ != nullptr)
    {
        return true;
    }

    DiscardRenderTargets();

    RECT clientRectangle{};
    if (!GetClientRect(window_, &clientRectangle))
    {
        return false;
    }

    const auto width = static_cast<UINT32>(
        std::max(clientRectangle.right - clientRectangle.left, 1L));
    const auto height = static_cast<UINT32>(
        std::max(clientRectangle.bottom - clientRectangle.top, 1L));

    const auto dpi = static_cast<float>(GetDpiForWindow(window_));
    HRESULT result = direct2DFactory_->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(),
            dpi,
            dpi),
        D2D1::HwndRenderTargetProperties(
            window_,
            D2D1::SizeU(width, height)),
        windowRenderTarget_.GetAddressOf());

    if (FAILED(result))
    {
        return false;
    }

    const D2D1_SIZE_F logicalSize = D2D1::SizeF(
        static_cast<float>(renderConfig_.width),
        static_cast<float>(renderConfig_.height));
    const D2D1_SIZE_U pixelSize = D2D1::SizeU(
        static_cast<UINT32>(renderConfig_.width),
        static_cast<UINT32>(renderConfig_.height));

    result = windowRenderTarget_->CreateCompatibleRenderTarget(
        &logicalSize,
        &pixelSize,
        nullptr,
        D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_NONE,
        gameRenderTarget_.GetAddressOf());

    if (SUCCEEDED(result))
    {
        result = gameRenderTarget_->GetBitmap(gameSurface_.GetAddressOf());
    }

    if (SUCCEEDED(result) && gameController_ != nullptr)
    {
        if (!gameController_->SetRenderTarget(gameRenderTarget_.Get()))
        {
            DiscardRenderTargets();
            return false;
        }
    }

    if (FAILED(result))
    {
        DiscardRenderTargets();
    }

    return SUCCEEDED(result);
}

void Application::DiscardRenderTargets()
{
    if (gameController_ != nullptr)
    {
        gameController_->SetRenderTarget(nullptr);
    }

    gameSurface_.Reset();
    gameRenderTarget_.Reset();
    windowRenderTarget_.Reset();
}

void Application::Render()
{
    if (window_ == nullptr || IsIconic(window_))
    {
        return;
    }

    if (!CreateRenderTargets())
    {
        return;
    }

    gameRenderTarget_->BeginDraw();
    gameRenderTarget_->Clear(D2D1::ColorF(0x102030));
    gameController_->Render();

    const HRESULT gameResult = gameRenderTarget_->EndDraw();
    if (gameResult == D2DERR_RECREATE_TARGET)
    {
        DiscardRenderTargets();
        return;
    }
    if (FAILED(gameResult))
    {
        return;
    }

    const D2D1_SIZE_F windowSize = windowRenderTarget_->GetSize();
    D2D1_RECT_F destination = D2D1::RectF(
        0.0F,
        0.0F,
        windowSize.width,
        windowSize.height);

    if (renderConfig_.scaling == ScalingMode::Fit)
    {
        const float scale = std::min(
            windowSize.width / static_cast<float>(renderConfig_.width),
            windowSize.height / static_cast<float>(renderConfig_.height));
        const float width = static_cast<float>(renderConfig_.width) * scale;
        const float height = static_cast<float>(renderConfig_.height) * scale;
        const float left = (windowSize.width - width) * 0.5F;
        const float top = (windowSize.height - height) * 0.5F;
        destination = D2D1::RectF(left, top, left + width, top + height);
    }

    const D2D1_BITMAP_INTERPOLATION_MODE interpolation =
        renderConfig_.filtering == ScalingFilter::Nearest
        ? D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR
        : D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;

    windowRenderTarget_->BeginDraw();
    windowRenderTarget_->Clear(D2D1::ColorF(D2D1::ColorF::Black));
    windowRenderTarget_->DrawBitmap(
        gameSurface_.Get(),
        destination,
        1.0F,
        interpolation);

    if (windowRenderTarget_->EndDraw() == D2DERR_RECREATE_TARGET)
    {
        DiscardRenderTargets();
    }
}

void Application::Shutdown()
{
    gameController_.reset();
    gameSurface_.Reset();
    gameRenderTarget_.Reset();
    windowRenderTarget_.Reset();
    direct2DFactory_.Reset();

    if (window_ != nullptr)
    {
        DestroyWindow(window_);
        window_ = nullptr;
    }

    if (windowClassRegistered_)
    {
        UnregisterClassW(WindowClassName, instance_);
        windowClassRegistered_ = false;
    }

    if (comInitialized_)
    {
        CoUninitialize();
        comInitialized_ = false;
    }
}
