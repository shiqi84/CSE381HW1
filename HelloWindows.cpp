// HelloWindowsDesktop.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <gdiplus.h>
#pragma comment(lib,"gdiplus.lib")
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
void OnPaint(HDC hdc, INT flag);

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow
)
{
    // Init GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);


    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
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
        150,        // Button width
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
        130,         // y position 
        150,        // Button width
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
        250,         // y position 
        150,        // Button width
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
        370,         // y position 
        150,        // Button width
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
        490,         // y position 
        150,        // Button width
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

    //Release GDI+ resources
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}

void OnPaint(HDC hdc, INT flag)
{
    //Generate 10 random x/y coordinates for vertices
    int xCorList[10] = {};
    int yCorList[10] = {};
    for (int i = 0; i < 10; i++) {
        xCorList[i] = rand() % 1160 + 220;
        yCorList[i] = rand() % 560 + 30;
    }

    Gdiplus::Graphics gf(hdc);
    // Color(alpha, red, green, blue)
    Gdiplus::Pen redPen(Gdiplus::Color(255, 255, 0, 0));
    Gdiplus::Pen blackPen(Gdiplus::Color(255, 0, 0, 0));
    Gdiplus::Pen whitePen(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::SolidBrush greenBrush(Gdiplus::Color(255, 0, 163, 30));
    Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));

    switch (flag) {
    case 0:
        // Draw canvas area
        gf.DrawLine(&blackPen, 170, 0, 170, 1000);
        gf.DrawLine(&blackPen, 180, 0, 180, 1000);
        gf.FillRectangle(&blackBrush, 200, 10, 1200, 600);
        gf.DrawRectangle(&whitePen, 200, 10, 1200, 600);

        // "Canvas" Dimensions: 
        // TL------------TR
        // |              |
        // |              |
        // |              |
        // BL------------BR
        // TL: (200, 10)
        // TR: (1400, 10)
        // BL: (200, 610)
        // BR: (1400, 610)
     
        // Start drawing points
        for (int i = 0; i < 10; i++) {
            gf.FillEllipse(&greenBrush, xCorList[i], yCorList[i], 20, 20);
            gf.DrawEllipse(&redPen, xCorList[i], yCorList[i], 20, 20);
        }
        break;
    case 1:
        gf.DrawLine(&redPen, 0, 0, 500, 500);
        break;
    case 2:
        gf.FillRectangle(&greenBrush, 400, 200, 100, 100);
        break;
    case 3:
        gf.DrawRectangle(&redPen, 450, 400, 100, 150);
        break;
    case 4:
        gf.FillEllipse(&greenBrush, 500, 500, 100, 100);
        break;
    case 5:
        gf.DrawEllipse(&redPen, 750, 750, 200, 200);
    }
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
            hdc = BeginPaint(hWnd, &ps);
            // draw here...
            OnPaint(hdc, 0);
            EndPaint(hWnd, &ps);
            MessageBox(hWnd, L"paint ", L"Minkowski Difference", MB_OK);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case click_one:
                    // Painting before or after MessageBox doesn't seem to affect anything
                    hdc = BeginPaint(hWnd, &ps);
                    // draw here...
                    InvalidateRect(hWnd, NULL, true);
                   // OnPaint(hdc, 0);
                   // EndPaint(hWnd, &ps);
                    //MessageBox(hWnd, L"Insert Minkowski Difference Demo Here", L"Minkowski Difference", MB_OK);
                    break;
                case click_two:
                    MessageBox(hWnd, L"Insert Minkowski Sum Demo Here", L"Minkowski Sum", MB_OK);
                    hdc = BeginPaint(hWnd, &ps);
                    // draw here...
                    OnPaint(hdc, 1);
                    EndPaint(hWnd, &ps);
                    break;
                case click_three:
                    MessageBox(hWnd, L"Insert Quickhull Demo Here", L"Quickhull", MB_OK);
                    hdc = BeginPaint(hWnd, &ps);
                    // draw here...
                    OnPaint(hdc, 3);
                    EndPaint(hWnd, &ps);
                    break;
                case click_four:
                    MessageBox(hWnd, L"Insert Point Convex Hull Demo Here", L"Point Convex Hul", MB_OK);
                    hdc = BeginPaint(hWnd, &ps);
                    // draw here...
                    OnPaint(hdc, 4);
                    EndPaint(hWnd, &ps);
                    break;
                case click_five:
                    MessageBox(hWnd, L"Insert GJK Demo Here", L"GJK", MB_OK);
                    hdc = BeginPaint(hWnd, &ps);
                    // draw here...
                    OnPaint(hdc, 5);
                    EndPaint(hWnd, &ps);
                    break;
            }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
            break;
    }

    return 0;
}