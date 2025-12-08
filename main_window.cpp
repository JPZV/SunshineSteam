#include "main_windows.hpp"
#ifdef _WIN32

#include <windows.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

CloseCallback onClose;

int StartDummyWindow(CloseCallback _onClose)
{
    onClose = _onClose;
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "SunshineSteam";

    if (!RegisterClassEx(&wc)) {
        // Handle error if registration fails
        MessageBox(NULL, "Window Registration Failed!", "Error", MB_ICONERROR);
        return 101;
    }

    HWND hwnd = CreateWindowEx(
        0,
        "SunshineSteam",
        "SunshineSteam",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        260, 32,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (hwnd == NULL) {
        // Handle error if window creation fails
        MessageBox(NULL, "Window Creation Failed!", "Error", MB_ICONERROR);
        return 102;
    }

    ShowWindow(hwnd, SW_SHOWMINNOACTIVE);
    UpdateWindow(hwnd);
    return 0;
}

bool WindowLoop()
{
    MSG msg = {};
    if (!GetMessage(&msg, NULL, 0, 0)) return false;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    return true;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CLOSE:
            if (onClose) onClose();
            DestroyWindow(hwnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
#endif