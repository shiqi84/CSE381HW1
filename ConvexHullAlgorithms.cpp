#include <windows.h>
#include <Windowsx.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <d2d1.h>

#pragma comment(lib, "d2d1")

#include "basewin.h"

#include <list>
#include <memory>
using namespace std;

// define event id here
#define click_one 1101
#define click_two 1102
#define click_three 1103
#define click_four 1104
#define click_five 1105
#define IDR_ACCEL1                      101
#define ID_TOGGLE_MODE                40002
#define ID_DRAW_MODE                  40003
#define ID_SELECT_MODE                40004

// Global variables
INT demoSelection = 0;

// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Convex Hull Algorithms");

HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}


class DPIScale
{
    static float scaleX;
    static float scaleY;

public:
    static void Initialize(HWND m_hwnd)
    {
        FLOAT dpiX, dpiY;
        // For some reason, this line doesn't work when you build...
        dpiX = (FLOAT)GetDpiForWindow(m_hwnd);
        dpiY = dpiX;
        // Can try replacing it with GetDpiForWindow(hWnd) if we can somehow access hWnd from here
        scaleX = dpiX / 96.0f;
        scaleY = dpiY / 96.0f;
    }

    template <typename T>
    static float PixelsToDipsX(T x)
    {
        return static_cast<float>(x) / scaleX;
    }

    template <typename T>
    static float PixelsToDipsY(T y)
    {
        return static_cast<float>(y) / scaleY;
    }
};

float DPIScale::scaleX = 1.0f;
float DPIScale::scaleY = 1.0f;


// Only used for reference
/*
void OnPaint(HDC hdc)
{
    Gdiplus::Graphics gf(hdc);
    // Color(alpha, red, green, blue)
    Gdiplus::Pen redPen(Gdiplus::Color(255, 255, 0, 0));
    Gdiplus::Pen blackPen(Gdiplus::Color(255, 0, 0, 0));
    Gdiplus::Pen whitePen(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::SolidBrush greenBrush(Gdiplus::Color(255, 0, 163, 30));
    Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));

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

    //Generate 10 random x/y coordinates for vertices
    int xCorList[10] = {};
    int yCorList[10] = {};
    for (int i = 0; i < 10; i++) {
        xCorList[i] = rand() % 1160 + 220;
        yCorList[i] = rand() % 560 + 30;
    }

    // Different ways to paint canvas for different demos
    switch (demoSelection) {
    case 0:
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
*/

struct MyEllipse
{
    D2D1_ELLIPSE    ellipse;
    D2D1_COLOR_F    color;

    void Draw(ID2D1RenderTarget* pRT, ID2D1SolidColorBrush* pBrush)
    {
        pBrush->SetColor(color);
        pRT->FillEllipse(ellipse, pBrush);
        pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
        pRT->DrawEllipse(ellipse, pBrush, 1.0f);
    }

    BOOL HitTest(float x, float y)
    {
        const float a = ellipse.radiusX;
        const float b = ellipse.radiusY;
        const float x1 = x - ellipse.point.x;
        const float y1 = y - ellipse.point.y;
        const float d = ((x1 * x1) / (a * a)) + ((y1 * y1) / (b * b));
        return d <= 1.0f;
    }
};

D2D1::ColorF::Enum colors[] = { D2D1::ColorF::Green, D2D1::ColorF::Green, D2D1::ColorF::Green };

class MainWindow : public BaseWindow<MainWindow>
{
    enum Mode
    {
        DrawMode,
        SelectMode,
        DragMode,
    };

    HCURSOR                 hCursor;

    ID2D1Factory*           pFactory;
    ID2D1HwndRenderTarget*  pRenderTarget;
    ID2D1SolidColorBrush*   pBrush;
    D2D1_POINT_2F           ptMouse;

    Mode                    mode;
    size_t                  nextColor;

    list<shared_ptr<MyEllipse>>             ellipses;
    list<shared_ptr<MyEllipse>>::iterator   selection;

    shared_ptr<MyEllipse> Selection()
    {
        if (selection == ellipses.end())
        {
            return nullptr;
        }
        else
        {
            return (*selection);
        }
    }

    void    ClearSelection() { selection = ellipses.end(); }
    HRESULT InsertEllipse(float x, float y);

    BOOL    HitTest(float x, float y);
    void    SetMode(Mode m);
    void    MoveSelection(float x, float y);
    HRESULT CreateGraphicsResources();
    void    DiscardGraphicsResources();
    void    OnPaint(HDC hdc);
    void    Resize();
    void    OnLButtonDown(int pixelX, int pixelY, DWORD flags);
    void    OnLButtonUp();
    void    OnMouseMove(int pixelX, int pixelY, DWORD flags);
    void    OnKeyDown(UINT vkey);

public:

    MainWindow() : pFactory(NULL), pRenderTarget(NULL), pBrush(NULL),
        ptMouse(D2D1::Point2F()), nextColor(0), selection(ellipses.end())
    {
    }

    PCWSTR  ClassName() const { return L"Convex Hull Algorithms"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    HWND    GetHWND() { return m_hwnd; }
};

HRESULT MainWindow::CreateGraphicsResources()
{
    HRESULT hr = S_OK;
    if (pRenderTarget == NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &pRenderTarget);

        if (SUCCEEDED(hr))
        {
            const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 1.0f, 0);
            hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);
        }
    }
    return hr;
}

void MainWindow::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
}

void MainWindow::OnPaint(HDC hdc)
{
    HRESULT hr = CreateGraphicsResources();
    if (SUCCEEDED(hr))
    {
        PAINTSTRUCT ps;
        D2D1_SIZE_F rtSize = pRenderTarget->GetSize();
        static const WCHAR demoTitle[] = L"Convex Hull Algorithms by Rocco Persico and Wan Shiqi";

        //BeginPaint(m_hwnd, &ps);

        pRenderTarget->BeginDraw();

        // Clear the canvas
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::LightGray));

        // Change brush color to black
        pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));
        
        // Draw lines
        pRenderTarget->DrawLine(
            D2D1::Point2F(170.0f, 0.0f),
            D2D1::Point2F(170.0f, 1000.0f),
            pBrush,
            0.5f
        );;
        pRenderTarget->DrawLine(
            D2D1::Point2F(180.0f, 0.0f),
            D2D1::Point2F(180.0f, 1000.0f),
            pBrush,
            0.5f
        );;

        // Create canvas rectange
        D2D1_RECT_F rectangle1 = D2D1::RectF(
            200.0f, 10.0f, 1200.0f, 600.0f
        );

        // Draw canvas outline.
        pRenderTarget->DrawRectangle(&rectangle1, pBrush);

        // Change brush color to white
        pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));

        // Fill canvas area.
        pRenderTarget->FillRectangle(&rectangle1, pBrush);


        // Draw canvas area
        //gf.DrawLine(&blackPen, 170, 0, 170, 1000);
        //gf.DrawLine(&blackPen, 180, 0, 180, 1000);
        //gf.FillRectangle(&blackBrush, 200, 10, 1200, 600);
        //gf.DrawRectangle(&whitePen, 200, 10, 1200, 600);

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

        //Generate 10 random x/y coordinates for vertices
        /*
        int xCorList[10] = {};
        int yCorList[10] = {};
        for (int i = 0; i < 10; i++) {
            xCorList[i] = rand() % 1160 + 220;
            yCorList[i] = rand() % 560 + 30;
        }

        for (int i = 0; i < 10; i++) {
            gf.FillEllipse(&greenBrush, xCorList[i], yCorList[i], 20, 20);
            gf.DrawEllipse(&redPen, xCorList[i], yCorList[i], 20, 20);
        }
        */

        // Different ways to paint canvas for different demos
        switch (demoSelection) {
        case 0:
            // Draw title text
            /*
            pRenderTarget->DrawText(
                demoTitle,
                ARRAYSIZE(demoTitle) - 1,
                0,
                D2D1::RectF(0.0f, 0.0f, 100.0f, 100.0f),
                pBrush
                );*/
            // Start drawing points
            for (int i = 0; i < 10; i++) {
                float x = rand() % 1160 + 220;
                float y = rand() % 560 + 30;
                InsertEllipse(x, y);
            }
            demoSelection = 1;
            break;
        case 1:
            // Insert Minkowski Difference Demo Here
            break;
        case 2:
            // Insert Minkowski Sum Demo Here
            break;
        case 3:
            // Insert QuickHull Demo Here
            break;
        case 4:
            // Insert Point Convex Hull Demo Here
            break;
        case 5:
            // Insert GJK Demo Here
            break;
        }

        // Start a new ellipse.
        // InsertEllipse(1.0, 1.0f);

        for (auto i = ellipses.begin(); i != ellipses.end(); ++i)
        {
            (*i)->Draw(pRenderTarget, pBrush);
        }

        if (Selection())
        {
            pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Red));
            pRenderTarget->DrawEllipse(Selection()->ellipse, pBrush, 2.0f);
        }

        hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
        {
            DiscardGraphicsResources();
        }
        //EndPaint(m_hwnd, &ps);
    }
}

void MainWindow::Resize()
{
    if (pRenderTarget != NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        pRenderTarget->Resize(size);

        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

void MainWindow::OnLButtonDown(int pixelX, int pixelY, DWORD flags)
{
    const float dipX = DPIScale::PixelsToDipsX(pixelX);
    const float dipY = DPIScale::PixelsToDipsY(pixelY);

    if (mode == DrawMode)
    {
        POINT pt = { pixelX, pixelY };

        if (DragDetect(m_hwnd, pt))
        {
            SetCapture(m_hwnd);

            // Start a new ellipse.
            InsertEllipse(dipX, dipY);
        }
    }
    else
    {
        ClearSelection();

        if (HitTest(dipX, dipY))
        {
            SetCapture(m_hwnd);

            ptMouse = Selection()->ellipse.point;
            ptMouse.x -= dipX;
            ptMouse.y -= dipY;

            SetMode(DragMode);
        }
    }
    InvalidateRect(m_hwnd, NULL, FALSE);
}

void MainWindow::OnLButtonUp()
{
    if ((mode == DrawMode) && Selection())
    {
        ClearSelection();
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
    else if (mode == DragMode)
    {
        SetMode(SelectMode);
    }
    ReleaseCapture();
}


void MainWindow::OnMouseMove(int pixelX, int pixelY, DWORD flags)
{
    const float dipX = DPIScale::PixelsToDipsX(pixelX);
    const float dipY = DPIScale::PixelsToDipsY(pixelY);

    if ((flags & MK_LBUTTON) && Selection())
    {
        if (mode == DrawMode)
        {
            // Resize the ellipse.
            const float width = (dipX - ptMouse.x) / 2;
            const float height = (dipY - ptMouse.y) / 2;
            const float x1 = ptMouse.x + width;
            const float y1 = ptMouse.y + height;

            Selection()->ellipse = D2D1::Ellipse(D2D1::Point2F(x1, y1), width, height);
        }
        else if (mode == DragMode)
        {
            // Move the ellipse.
            Selection()->ellipse.point.x = dipX + ptMouse.x;
            Selection()->ellipse.point.y = dipY + ptMouse.y;
        }
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}


void MainWindow::OnKeyDown(UINT vkey)
{
    switch (vkey)
    {
    case VK_BACK:
    case VK_DELETE:
        if ((mode == SelectMode) && Selection())
        {
            ellipses.erase(selection);
            ClearSelection();
            SetMode(SelectMode);
            InvalidateRect(m_hwnd, NULL, FALSE);
        };
        break;

    case VK_LEFT:
        MoveSelection(-1, 0);
        break;

    case VK_RIGHT:
        MoveSelection(1, 0);
        break;

    case VK_UP:
        MoveSelection(0, -1);
        break;

    case VK_DOWN:
        MoveSelection(0, 1);
        break;

    case VK_F1:
        SetMode(DrawMode);
        break;

    case VK_F2:
        SetMode(SelectMode);
        break;

    case VK_F3:
        SetMode(DragMode);
        break;

    }
}

HRESULT MainWindow::InsertEllipse(float x, float y)
{
    try
    {
        selection = ellipses.insert(
            ellipses.end(),
            shared_ptr<MyEllipse>(new MyEllipse()));

        Selection()->ellipse.point = ptMouse = D2D1::Point2F(x, y);
        Selection()->ellipse.radiusX = Selection()->ellipse.radiusY = 8.0f;
        Selection()->color = D2D1::ColorF(colors[nextColor]);

        nextColor = (nextColor + 1) % ARRAYSIZE(colors);
    }
    catch (std::bad_alloc)
    {
        return E_OUTOFMEMORY;
    }
    return S_OK;
}


BOOL MainWindow::HitTest(float x, float y)
{
    for (auto i = ellipses.rbegin(); i != ellipses.rend(); ++i)
    {
        if ((*i)->HitTest(x, y))
        {
            selection = (++i).base();
            return TRUE;
        }
    }
    return FALSE;
}

void MainWindow::MoveSelection(float x, float y)
{
    if ((mode == SelectMode) && Selection())
    {
        Selection()->ellipse.point.x += x;
        Selection()->ellipse.point.y += y;
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

void MainWindow::SetMode(Mode m)
{
    mode = m;

    LPWSTR cursor{};
    switch (mode)
    {
    case DrawMode:
        cursor = IDC_CROSS;
        break;

    case SelectMode:
        cursor = IDC_HAND;
        break;

    case DragMode:
        cursor = IDC_SIZEALL;
        break;
    }

    hCursor = LoadCursor(NULL, cursor);
    SetCursor(hCursor);
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    MainWindow win;

    if (!win.Create(L"Convex Hull Algorithms", WS_OVERLAPPEDWINDOW))
    {
        return 0;
    }

    HWND hwndButton1 = CreateWindowEx(
        NULL,
        L"BUTTON", // Predefined class; Unicode assumed 
        L"Minkowski Difference",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        10,         // x position 
        10,         // y position 
        150,        // Button width
        100,        // Button height
        win.GetHWND(),       // Parent window
        (HMENU)click_one,       // No menu.
        (HINSTANCE)GetWindowLongPtr(win.GetHWND(), GWLP_HINSTANCE),
        NULL        // Pointer not needed
    );
    HWND hwndButton2 = CreateWindowEx(
        NULL, 
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"Minkowski Sum",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        10,         // x position 
        130,         // y position 
        150,        // Button width
        100,        // Button height
        win.GetHWND(),       // Parent window
        (HMENU)click_two,       // No menu.
        (HINSTANCE)GetWindowLongPtr(win.GetHWND(), GWLP_HINSTANCE),
        NULL        // Pointer not needed
    );

    HWND hwndButton3 = CreateWindowEx(
        NULL, 
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"Quickhull",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        10,         // x position 
        250,         // y position 
        150,        // Button width
        100,        // Button height
        win.GetHWND(),       // Parent window
        (HMENU)click_three,       // No menu.
        (HINSTANCE)GetWindowLongPtr(win.GetHWND(), GWLP_HINSTANCE),
        NULL        // Pointer not needed
    );

    HWND hwndButton4 = CreateWindowEx(
        NULL, 
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"Point Convex Hull",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        10,         // x position 
        370,         // y position 
        150,        // Button width
        100,        // Button height
        win.GetHWND(),       // Parent window
        (HMENU)click_four,       // No menu.
        (HINSTANCE)GetWindowLongPtr(win.GetHWND(), GWLP_HINSTANCE),
        NULL        // Pointer not needed
    );

    HWND hwndButton5 = CreateWindowEx(
        NULL, 
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"GJK",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        10,         // x position 
        490,         // y position 
        150,        // Button width
        100,        // Button height
        win.GetHWND(),       // Parent window
        (HMENU)click_five,       // No menu.
        (HINSTANCE)GetWindowLongPtr(win.GetHWND(), GWLP_HINSTANCE),
        NULL        // Pointer not needed
    );

    HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCEL1));

    ShowWindow(win.Window(), nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(win.Window(), hAccel, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    switch (uMsg)
    {
    case WM_CREATE:
        if (FAILED(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
        {
            return -1;  // Fail CreateWindowEx.
        }
        DPIScale::Initialize(m_hwnd);
        SetMode(SelectMode);
        return 0;

    case WM_DESTROY:
        DiscardGraphicsResources();
        SafeRelease(&pFactory);
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(m_hwnd, &ps);
        OnPaint(hdc);
        EndPaint(m_hwnd, &ps);
        return 0;

    case WM_SIZE:
        Resize();
        return 0;

    case WM_LBUTTONDOWN:
        OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
        return 0;

    case WM_LBUTTONUP:
        OnLButtonUp();
        return 0;

    case WM_MOUSEMOVE:
        OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam);
        return 0;

    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT)
        {
            SetCursor(hCursor);
            return TRUE;
        }
        break;

    case WM_KEYDOWN:
        OnKeyDown((UINT)wParam);
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        /*
        * These cases don't seem to do anything
        * Replaced by checking VK_F1, VK_F2, and VK_F3 in OnKeyDown()
        case ID_DRAW_MODE:
            SetMode(DrawMode);
            break;

        case ID_SELECT_MODE:
            SetMode(SelectMode);
            break;

        case ID_TOGGLE_MODE:
            if (mode == DrawMode)
            {
                SetMode(SelectMode);
            }
            else
            {
                SetMode(DrawMode);
            }
            break;
        */
        case click_one:
            // Set selection for demo
            demoSelection = 1;
            // Clear area and trigger repaint
            InvalidateRect(m_hwnd, NULL, true);
            // Pop-up Message Box
            MessageBox(m_hwnd, L"Insert Minkowski Difference Demo Here", L"Minkowski Difference", MB_OK);
            break;
        case click_two:
            // Set selection for demo
            demoSelection = 2;
            // Clear area and trigger repaint
            InvalidateRect(m_hwnd, NULL, true);
            // Pop-up Message Box
            MessageBox(m_hwnd, L"Insert Minkowski Sum Demo Here", L"Minkowski Sum", MB_OK);
            break;
        case click_three:
            // Set selection for demo
            demoSelection = 3;
            // Clear area and trigger repaint
            InvalidateRect(m_hwnd, NULL, true);
            // Pop-up Message Box
            MessageBox(m_hwnd, L"Insert Quickhull Demo Here", L"Quickhull", MB_OK);
            break;
        case click_four:
            // Set selection for demo
            demoSelection = 4;
            // Clear area and trigger repaint
            InvalidateRect(m_hwnd, NULL, true);
            // Pop-up Message Box
            MessageBox(m_hwnd, L"Insert Point Convex Hull Demo Here", L"Point Convex Hul", MB_OK);
            break;
        case click_five:
            // Set selection for demo
            demoSelection = 5;
            // Clear area and trigger repaint
            InvalidateRect(m_hwnd, NULL, true);
            // Pop-up Message Box
            MessageBox(m_hwnd, L"Insert GJK Demo Here", L"GJK", MB_OK);
            break;
        }
        return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}