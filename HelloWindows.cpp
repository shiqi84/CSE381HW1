// HelloWindowsDesktop.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

// define event id here
#define click_one 1101
#define click_two 1102
#define click_three 1103
#define click_four 1104
#define click_five 1105

// Global variables

// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Rocco's Windows Desktop App");

HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("Rocco's Windows Desktop App"),
            NULL);

        return 1;
    }

    // Store instance handle in our global variable
    hInst = hInstance;

    // The parameters to CreateWindowEx explained:
    // WS_EX_OVERLAPPEDWINDOW : An optional extended window style.
    // szWindowClass: the name of the application
    // szTitle: the text that appears in the title bar
    // WS_OVERLAPPEDWINDOW: the type of window to create
    // CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
    // 500, 100: initial size (width, length)
    // NULL: the parent of this window
    // NULL: this application does not have a menu bar
    // hInstance: the first parameter from WinMain
    // NULL: not used in this application
    HWND hWnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    HWND hwndButton1 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"Minkowski Difference",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        10,         // x position 
        10,         // y position 
        100,        // Button width
        100,        // Button height
        hWnd,       // Parent window
        (HMENU)click_one,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL        // Pointer not needed
    );  

    HWND hwndButton2 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"Minkowski Sum",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        10,         // x position 
        120,         // y position 
        100,        // Button width
        100,        // Button height
        hWnd,       // Parent window
        (HMENU)click_two,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL        // Pointer not needed
    );

    HWND hwndButton3 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"Quickhull",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        10,         // x position 
        230,         // y position 
        100,        // Button width
        100,        // Button height
        hWnd,       // Parent window
        (HMENU)click_three,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL        // Pointer not needed
    );

    HWND hwndButton4 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"Point Convex Hull",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        10,         // x position 
        340,         // y position 
        100,        // Button width
        100,        // Button height
        hWnd,       // Parent window
        (HMENU)click_four,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL        // Pointer not needed
    );

    HWND hwndButton5 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"GJK",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        10,         // x position 
        450,         // y position 
        100,        // Button width
        100,        // Button height
        hWnd,       // Parent window
        (HMENU)click_five,       // No menu.
        (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
        NULL        // Pointer not needed
    );

    if (!hWnd)
    {
        MessageBox(NULL,
            _T("Call to CreateWindow failed!"),
            _T("Rocco's Windows Desktop App"),
            NULL);

        return 1;
    }

    // The parameters to ShowWindow explained:
    // hWnd: the value returned from CreateWindow
    // nCmdShow: the fourth parameter from WinMain
    ShowWindow(hWnd,
        nCmdShow);
    UpdateWindow(hWnd);

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

void OnPaint(HWND hWnd, INT flag)
{
    PAINTSTRUCT ps;
    HDC hdc;
    HBRUSH hBrush;
    hdc = BeginPaint(hWnd, &ps);
    if (flag == 0) {
        // Fill the window with a colored rect
        hBrush = CreateSolidBrush(RGB(150, 150, 150));
        FillRect(hdc, &ps.rcPaint, hBrush);

        // Then print out "Hello, Windows desktop!" in the top left corner.
        TCHAR greeting[] = _T("Hello there, welcome to my Window!");
        TextOut(hdc, 120, 100, greeting, _tcslen(greeting));
    }
    if (flag == 1) {
        // Fill the window with a colored rect
        hBrush = CreateSolidBrush(RGB(224, 144, 144));
        FillRect(hdc, &ps.rcPaint, hBrush);

        // Print Text
        TCHAR greeting[] = _T("Minkowski Difference");
        TextOut(hdc, 120, 10, greeting, _tcslen(greeting));
    }
    if (flag == 2) {
        // Fill the window with a colored rect
        hBrush = CreateSolidBrush(RGB(216, 224, 144));
        FillRect(hdc, &ps.rcPaint, hBrush);

        // Print Text
        TCHAR greeting[] = _T("Minkowski Sum");
        TextOut(hdc, 120, 10, greeting, _tcslen(greeting));
    }
    if (flag == 3) {
        // Fill the window with a colored rect
        hBrush = CreateSolidBrush(RGB(149, 224, 144));
        FillRect(hdc, &ps.rcPaint, hBrush);

        // Print Text
        TCHAR greeting[] = _T("Quickhull");
        TextOut(hdc, 120, 10, greeting, _tcslen(greeting));
    }
    if (flag == 4) {
        // Fill the window with a colored rect
        hBrush = CreateSolidBrush(RGB(144, 203, 224));
        FillRect(hdc, &ps.rcPaint, hBrush);

        // Print Text
        TCHAR greeting[] = _T("Point Convex Hull");
        TextOut(hdc, 120, 10, greeting, _tcslen(greeting));
    }
    if (flag == 5) {
        // Fill the window with a colored rect
        hBrush = CreateSolidBrush(RGB(157, 144, 224));
        FillRect(hdc, &ps.rcPaint, hBrush);

        // Print Text
        TCHAR greeting[] = _T("GJK");
        TextOut(hdc, 120, 10, greeting, _tcslen(greeting));
    }
    // End application-specific layout section.
    EndPaint(hWnd, &ps);
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    HBRUSH hBrush;
    TCHAR greeting[] = _T("Hello there, welcome to my Window!");

    switch (message)
    {
        case WM_PAINT:
            OnPaint(hWnd, 0);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case click_one:
                    MessageBox(hWnd, L"Insert Minkowski Difference Demo Here", L"Minkowski Difference", MB_OK);
                    OnPaint(hWnd, 1);
                    break;
                case click_two:
                    MessageBox(hWnd, L"Insert Minkowski Sum Demo Here", L"Minkowski Sum", MB_OK);
                    OnPaint(hWnd, 2);
                    break;
                case click_three:
                    MessageBox(hWnd, L"Insert Quickhull Demo Here", L"Quickhull", MB_OK);
                    OnPaint(hWnd, 3);
                    break;
                case click_four:
                    MessageBox(hWnd, L"Insert Point Convex Hull Demo Here", L"Point Convex Hul", MB_OK);
                    OnPaint(hWnd, 4);
                    break;
                case click_five:
                    MessageBox(hWnd, L"Insert GJK Demo Here", L"GJK", MB_OK);
                    OnPaint(hWnd, 5);
                    break;
            }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
            break;
    }

    return 0;
}