#include "Application.h"

#include <windows.h>

int WINAPI wWinMain(
    _In_ HINSTANCE instance,
    _In_opt_ HINSTANCE previousInstance,
    _In_ PWSTR commandLine,
    _In_ int showCommand)
{
    static_cast<void>(previousInstance);
    static_cast<void>(commandLine);

    Application application;
    if (!application.Initialize(instance, showCommand))
    {
        MessageBoxW(
            nullptr,
            L"The application could not be initialized.",
            L"RPG2002-Ship",
            MB_OK | MB_ICONERROR);
        return 1;
    }

    return application.Run();
}
