#include <windows.h>
#include <Windowsx.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <d2d1.h>
#include <set>
#pragma comment(lib, "d2d1")

#include "basewin.h"

#include <vector>
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



struct MyPolygon
{
    std::vector<D2D1_POINT_2F>    polygon;
    D2D1_COLOR_F                  color;

    void SetColor(D2D1_COLOR_F newColor) {
        color = newColor;
    }

    void Draw(ID2D1RenderTarget* pRT, ID2D1SolidColorBrush* pBrush)
    {

        pBrush->SetColor(color);

        for (int i = 0; i < polygon.size() - 1; i++) {
            pRT->DrawLine(
                D2D1::Point2F(polygon[i].x, polygon[i].y),
                D2D1::Point2F(polygon[i + 1].x, polygon[i + 1].y),
                pBrush,
                1.0f
            );
        }
        pRT->DrawLine(
            D2D1::Point2F(polygon[polygon.size() - 1].x, polygon[polygon.size() - 1].y),
            D2D1::Point2F(polygon[0].x, polygon[0].y),
            pBrush,
            1.0f
        );

    }

    BOOL HitTest(float x, float y)
    {
        D2D1_POINT_2F point;
        point.x = x;
        point.y = y;
        return contains(point);
    }

    // returns if p lies on line segment p1->p2
    BOOL onSegment(D2D1_POINT_2F p1, D2D1_POINT_2F p2, D2D1_POINT_2F p3) {
        if (p2.x <= max(p1.x, p3.x) && p2.x >= min(p1.x, p3.x) && p2.y <= max(p1.y, p3.y) && p2.y >= min(p1.y, p3.y))
            return true;
        return false;
    }

    // returns the orientation of three points
    int orientation(D2D1_POINT_2F p1, D2D1_POINT_2F p2, D2D1_POINT_2F p3) {
        // calculate slope of both line segments
        float val = (p2.y - p1.y) * (p3.x - p2.x) - (p2.x - p1.x) * (p3.y - p2.y);
        if (val == 0)
            return 0;           // colinear
        else if (val > 0)
            return 1;           // clockwise
        else
            return 2;           // counter clockwise
    }
    
    // returns if line segment start1->end1 intersects line segment start2->end2
    BOOL intersecting(D2D1_POINT_2F start1, D2D1_POINT_2F end1, D2D1_POINT_2F start2, D2D1_POINT_2F end2) {
        // generate the four orientations
        int orientation1 = orientation(start1, end1, start2);
        int orientation2 = orientation(start1, end1, end2);
        int orientation3 = orientation(start2, end2, start1);
        int orientation4 = orientation(start2, end2, end1);

        // if the general case, or any special cases are met, line segments are intersecting

        // general case - when orientation1/orientation2 are different and orientation3/orientation4 are different
        if (orientation1 != orientation2 && orientation3 != orientation4)
            return true;

        // special case - when orientation1 is colinear and start2 lies on line segment start1->end1
        if (orientation1 == 0 && onSegment(start1, start2, end1))
            return true;

        // special case - when orientation2 is colinear and end2 lies on line segment start1->end1
        if (orientation2 == 0 && onSegment(start1, end2, end1))
            return true;

        // special case - when orientation3 is colinear and start1 lies on line segment start2->end2
        if (orientation3 == 0 && onSegment(start2, start1, end2))
            return true;

        // special case - when orientation4 is colinear and end1 lies on line segment start2->end2
        if (orientation4 == 0 && onSegment(start2, end1, end2))
            return true;

        return false;
    }

    // returns if this polygon contains the given point
    BOOL contains(D2D1_POINT_2F p) {
        // create end-point for ray tracing
        D2D1_POINT_2F tmp = D2D1::Point2F(5000.0f, p.y);
        int intersections = 0;
        int i = 0;
        do {
            int next = (i + 1) % polygon.size();
            // does this ray intersect with line segment polygon[i]->polygon[next]?
            if (intersecting(polygon[i], polygon[next], p, tmp)) {
                // if p colinear with line segment polygon[i]->polygon[next] and lies on segment, point is in polygon
                if (orientation(polygon[i], p, polygon[next]) == 0)
                    return onSegment(polygon[i], p, polygon[next]);
                intersections++;
            }
            i = next;
        } while (i != 0);
        // if number of intersections is odd, point is in polygon. otherwise, it is not
        return (intersections % 2 == 1);
    }
};
MyPolygon testingPolygon;
MyPolygon testingPolygon1;
MyPolygon testingPolygon2;

struct Vector2D
{
    float x;
    float y;
    bool operator<(const Vector2D& v) const
    {
        return (this->x < v.x);
    }
};


// Global variables
INT demoSelection = 0;

// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");
int ModeFlag = 0;
// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Convex Hull Algorithms");
static   set<Vector2D>  Vec;
static  Vector2D ellipse[10];
static  Vector2D ellipse1[5];
static  Vector2D ellipse2[5];
static  Vector2D ellipseMsk[25];
static std::vector<D2D1_POINT_2F> arr;
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

    ID2D1Factory* pFactory;
    ID2D1HwndRenderTarget* pRenderTarget;
    ID2D1SolidColorBrush* pBrush;
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
    void    GenerateRandomPoints(int num);
    void    DrawTestPolygon(int flag);
    void    MoveSelection(float x, float y);
    HRESULT CreateGraphicsResources();
    void    DiscardGraphicsResources();
    void    OnPaint(HDC hdc);
    void    Resize();
    void    OnLButtonDown(int pixelX, int pixelY, DWORD flags);
    void    OnLButtonUp();
    void    OnMouseMove(int pixelX, int pixelY, DWORD flags);
    void    OnKeyDown(UINT vkey);
    int     FindSide(Vector2D p1, Vector2D p2, Vector2D p);
    float   dist(Vector2D p, Vector2D q);
    float   lineDist(Vector2D p1, Vector2D p2, Vector2D p);
    bool    PointCmp(const D2D1_POINT_2F& a, const D2D1_POINT_2F& b, const D2D1_POINT_2F& center);
    void    ClockwiseSortPoints(std::vector<D2D1_POINT_2F>& vPoints);
    void    quickHull(Vector2D a[], int n, Vector2D p1, Vector2D p2, int side, MyPolygon& testingPolygon);
    void    AddAxes();

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
void MainWindow::AddAxes() {
    // Draw grid lines
    pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Gray));
    for (float i = 200; i < 1300; i += 10) {
        pRenderTarget->DrawLine(
            D2D1::Point2F(i, 10.0f),
            D2D1::Point2F(i, 700.0f),
            pBrush,
            0.5f
        );;
    }
    for (float i = 10; i < 710; i += 10) {
        pRenderTarget->DrawLine(
            D2D1::Point2F(200.0f, i),
            D2D1::Point2F(1300.0f, i),
            pBrush,
            0.5f
        );;
    }
    // Draw axes
    pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));
    pRenderTarget->DrawLine(
        D2D1::Point2F(750.0f, 10.0f),
        D2D1::Point2F(750.0f, 700.0f),
        pBrush,
        2.0f
    );;
    pRenderTarget->DrawLine(
        D2D1::Point2F(200.0f, 350.0f),
        D2D1::Point2F(1300.0f, 350.0f),
        pBrush,
        2.0f
    );;
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
        D2D1_SIZE_F rtSize = pRenderTarget->GetSize();
        static const WCHAR demoTitle[] = L"Convex Hull Algorithms by Rocco Persico and Wan Shiqi";

        pRenderTarget->BeginDraw();

        // Clear the canvas
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::DarkGray));

        pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White));

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
            200.0f, 10.0f, 1300.0f, 700.0f
        );
        // Draw canvas outline.
        pRenderTarget->DrawRectangle(&rectangle1, pBrush);

        pBrush->SetColor(D2D1::ColorF(D2D1::ColorF::Black));

        // Fill canvas area.
        pRenderTarget->FillRectangle(&rectangle1, pBrush);






        switch (demoSelection) {
            case 0:
            {
                demoSelection = -1;
                break;
            }
            case 1:
            {
                // Insert Minkowski Difference Demo Here
                if (ModeFlag != 1) {
                    ModeFlag = 1;
                    ellipses.clear();
                    GenerateRandomPoints(5);
                }
                AddAxes();
                //draw the first polygon here
                testingPolygon1.polygon.clear();
                int min_x = 0;
                int max_x = 0;
                for (int i = 0; i < 5; i++)
                {
                    if (ellipse1[i].x < ellipse1[min_x].x)
                        min_x = i;
                    if (ellipse1[i].x > ellipse1[max_x].x)
                        max_x = i;
                }

                quickHull(ellipse1, 5, ellipse1[min_x], ellipse1[max_x], 1, testingPolygon1);
                quickHull(ellipse1, 5, ellipse1[min_x], ellipse1[max_x], -1, testingPolygon1);
                std::vector<D2D1_POINT_2F> tmp1;
                tmp1.push_back(testingPolygon1.polygon[0]);
                for (int i = 1; i < testingPolygon1.polygon.size(); i++) {

                    bool repeated = false;
                    for (int j = 0; j < tmp1.size(); j++) {
                        if (tmp1[j].x == testingPolygon1.polygon[i].x && tmp1[j].y == testingPolygon1.polygon[i].y) {
                            repeated = true;
                        }

                    }
                    if (!repeated) {
                        tmp1.push_back(testingPolygon1.polygon[i]);
                    }
                }

                ClockwiseSortPoints(tmp1);
                testingPolygon1.polygon = tmp1;
                testingPolygon1.SetColor(D2D1::ColorF(D2D1::ColorF::HotPink));
                testingPolygon1.Draw(pRenderTarget, pBrush);

                //draw the second polygon
                testingPolygon2.polygon.clear();
                min_x = 0;
                max_x = 0;
                for (int i = 0; i < 5; i++)
                {
                    if (ellipse2[i].x < ellipse2[min_x].x)
                        min_x = i;
                    if (ellipse2[i].x > ellipse2[max_x].x)
                        max_x = i;
                }

                quickHull(ellipse2, 5, ellipse2[min_x], ellipse2[max_x], 1, testingPolygon2);
                quickHull(ellipse2, 5, ellipse2[min_x], ellipse2[max_x], -1, testingPolygon2);
                std::vector<D2D1_POINT_2F> tmp2;
                tmp2.push_back(testingPolygon2.polygon[0]);
                for (int i = 1; i < testingPolygon2.polygon.size(); i++) {

                    bool repeated = false;
                    for (int j = 0; j < tmp2.size(); j++) {
                        if (tmp2[j].x == testingPolygon2.polygon[i].x && tmp2[j].y == testingPolygon2.polygon[i].y) {
                            repeated = true;
                        }

                    }
                    if (!repeated) {
                        tmp2.push_back(testingPolygon2.polygon[i]);
                    }
                }

                ClockwiseSortPoints(tmp2);
                testingPolygon2.polygon = tmp2;
                testingPolygon2.SetColor(D2D1::ColorF(D2D1::ColorF::HotPink));
                testingPolygon2.Draw(pRenderTarget, pBrush);

                // do difference here
                testingPolygon.polygon.clear();
                min_x = 0;
                max_x = 0;
                for (int i = 0; i < 5; i++) {
                    for (int j = 0; j < 5; j++) {
                        ellipseMsk[i * 5 + j] = { ellipse1[i].x - ellipse2[j].x + 750, ellipse1[i].y - ellipse2[j].y + 360 };
                    }
                }
                for (int i = 0; i < 25; i++)
                {
                    if (ellipseMsk[i].x < ellipseMsk[min_x].x)
                        min_x = i;
                    if (ellipseMsk[i].x > ellipseMsk[max_x].x)
                        max_x = i;
                }

                quickHull(ellipseMsk, 25, ellipseMsk[min_x], ellipseMsk[max_x], 1, testingPolygon);
                quickHull(ellipseMsk, 25, ellipseMsk[min_x], ellipseMsk[max_x], -1, testingPolygon);
                std::vector<D2D1_POINT_2F> tmp;
                tmp.push_back(testingPolygon.polygon[0]);
                for (int i = 1; i < testingPolygon.polygon.size(); i++) {

                    bool repeated = false;
                    for (int j = 0; j < tmp.size(); j++) {
                        if (tmp[j].x == testingPolygon.polygon[i].x && tmp[j].y == testingPolygon.polygon[i].y) {
                            repeated = true;
                        }

                    }
                    if (!repeated) {
                        tmp.push_back(testingPolygon.polygon[i]);
                    }
                }

                ClockwiseSortPoints(tmp);
                testingPolygon.polygon = tmp;
                testingPolygon.SetColor(D2D1::ColorF(D2D1::ColorF::LightSkyBlue));
                testingPolygon.Draw(pRenderTarget, pBrush);

                break;
            }
            case 2:
            {
                // Insert Minkowski Sum Demo Here
                if (ModeFlag != 2) {

                    ModeFlag = 2;
                    ellipses.clear();
                    GenerateRandomPoints(5);
                }
                AddAxes();
                //draw the first polygon here
                testingPolygon1.polygon.clear();
                int min_x = 0;
                int max_x = 0;
                for (int i = 0; i < 5; i++)
                {
                    if (ellipse1[i].x < ellipse1[min_x].x)
                        min_x = i;
                    if (ellipse1[i].x > ellipse1[max_x].x)
                        max_x = i;
                }

                quickHull(ellipse1, 5, ellipse1[min_x], ellipse1[max_x], 1, testingPolygon1);
                quickHull(ellipse1, 5, ellipse1[min_x], ellipse1[max_x], -1, testingPolygon1);
                std::vector<D2D1_POINT_2F> tmp1;
                tmp1.push_back(testingPolygon1.polygon[0]);
                for (int i = 1; i < testingPolygon1.polygon.size(); i++) {

                    bool repeated = false;
                    for (int j = 0; j < tmp1.size(); j++) {
                        if (tmp1[j].x == testingPolygon1.polygon[i].x && tmp1[j].y == testingPolygon1.polygon[i].y) {
                            repeated = true;
                        }

                    }
                    if (!repeated) {
                        tmp1.push_back(testingPolygon1.polygon[i]);
                    }
                }


                ClockwiseSortPoints(tmp1);
                testingPolygon1.polygon = tmp1;
                testingPolygon1.SetColor(D2D1::ColorF(D2D1::ColorF::HotPink));
                testingPolygon1.Draw(pRenderTarget, pBrush);


                //draw the second polygon
                testingPolygon2.polygon.clear();
                min_x = 0;
                max_x = 0;
                for (int i = 0; i < 5; i++)
                {
                    if (ellipse2[i].x < ellipse2[min_x].x)
                        min_x = i;
                    if (ellipse2[i].x > ellipse2[max_x].x)
                        max_x = i;
                }

                quickHull(ellipse2, 5, ellipse2[min_x], ellipse2[max_x], 1, testingPolygon2);
                quickHull(ellipse2, 5, ellipse2[min_x], ellipse2[max_x], -1, testingPolygon2);
                std::vector<D2D1_POINT_2F> tmp2;
                tmp2.push_back(testingPolygon2.polygon[0]);
                for (int i = 1; i < testingPolygon2.polygon.size(); i++) {

                    bool repeated = false;
                    for (int j = 0; j < tmp2.size(); j++) {
                        if (tmp2[j].x == testingPolygon2.polygon[i].x && tmp2[j].y == testingPolygon2.polygon[i].y) {
                            repeated = true;
                        }

                    }
                    if (!repeated) {
                        tmp2.push_back(testingPolygon2.polygon[i]);
                    }
                }


                ClockwiseSortPoints(tmp2);
                testingPolygon2.polygon = tmp2;
                testingPolygon2.SetColor(D2D1::ColorF(D2D1::ColorF::HotPink));
                testingPolygon2.Draw(pRenderTarget, pBrush);
                // do sum here
                testingPolygon.polygon.clear();
                min_x = 0;
                max_x = 0;
                for (int i = 0; i < 5; i++) {
                    for (int j = 0; j < 5; j++) {
                        ellipseMsk[i * 5 + j] = { ellipse1[i].x + ellipse2[j].x - 750, ellipse1[i].y + ellipse2[j].y - 360 };
                    }
                }
                //   MinkowskiSum(testingPolygon1.polygon, testingPolygon2.polygon);
                for (int i = 0; i < 25; i++)
                {
                    if (ellipseMsk[i].x < ellipseMsk[min_x].x)
                        min_x = i;
                    if (ellipseMsk[i].x > ellipseMsk[max_x].x)
                        max_x = i;
                }

                quickHull(ellipseMsk, 25, ellipseMsk[min_x], ellipseMsk[max_x], 1, testingPolygon);
                quickHull(ellipseMsk, 25, ellipseMsk[min_x], ellipseMsk[max_x], -1, testingPolygon);
                std::vector<D2D1_POINT_2F> tmp;
                tmp.push_back(testingPolygon.polygon[0]);
                for (int i = 1; i < testingPolygon.polygon.size(); i++) {

                    bool repeated = false;
                    for (int j = 0; j < tmp.size(); j++) {
                        if (tmp[j].x == testingPolygon.polygon[i].x && tmp[j].y == testingPolygon.polygon[i].y) {
                            repeated = true;
                        }

                    }
                    if (!repeated) {
                        tmp.push_back(testingPolygon.polygon[i]);
                    }
                }


                ClockwiseSortPoints(tmp);
                testingPolygon.polygon = tmp;
                testingPolygon.SetColor(D2D1::ColorF(D2D1::ColorF::LightSkyBlue));
                testingPolygon.Draw(pRenderTarget, pBrush);

                break;
            }
            case 3:
            {
                // Insert QuickHull Demo Here
                testingPolygon.polygon.clear();
                if (ModeFlag != 3) {
                    ModeFlag = 3;
                    ellipses.clear();
                    GenerateRandomPoints(10);
                }
                int min_x = 0;
                int max_x = 0;
                for (int i = 0; i < 10; i++)
                {
                    if (ellipse[i].x < ellipse[min_x].x)
                        min_x = i;
                    if (ellipse[i].x > ellipse[max_x].x)
                        max_x = i;
                }

                quickHull(ellipse, 10, ellipse[min_x], ellipse[max_x], 1, testingPolygon);
                quickHull(ellipse, 10, ellipse[min_x], ellipse[max_x], -1, testingPolygon);
                std::vector<D2D1_POINT_2F> tmp;
                tmp.push_back(testingPolygon.polygon[0]);
                for (int i = 1; i < testingPolygon.polygon.size(); i++) {

                    bool repeated = false;
                    for (int j = 0; j < tmp.size(); j++) {
                        if (tmp[j].x == testingPolygon.polygon[i].x && tmp[j].y == testingPolygon.polygon[i].y) {
                            repeated = true;
                        }

                    }
                    if (!repeated) {
                        tmp.push_back(testingPolygon.polygon[i]);
                    }
                }
                ClockwiseSortPoints(tmp);
                testingPolygon.polygon = tmp;
                testingPolygon.SetColor(D2D1::ColorF(D2D1::ColorF::LightSkyBlue));
                testingPolygon.Draw(pRenderTarget, pBrush);

                break;
            }
            case 4:
            {
                // Insert Point Convex Hull Demo Here
                testingPolygon.polygon.clear();
                DrawTestPolygon(1);
                if (ModeFlag != 4) {
                    ellipses.clear();
                    ModeFlag = 4;
                    GenerateRandomPoints(1);
                }
                break;
            }
            case 5:
            {
                // Insert GJK Demo Here
                if (ModeFlag != 5) {
                    ModeFlag = 5;
                    ellipses.clear();
                    GenerateRandomPoints(5);
                }
                AddAxes();
                //draw the first polygon here
                testingPolygon1.polygon.clear();
                int min_x = 0;
                int max_x = 0;
                for (int i = 0; i < 5; i++)
                {
                    if (ellipse1[i].x < ellipse1[min_x].x)
                        min_x = i;
                    if (ellipse1[i].x > ellipse1[max_x].x)
                        max_x = i;
                }

                quickHull(ellipse1, 5, ellipse1[min_x], ellipse1[max_x], 1, testingPolygon1);
                quickHull(ellipse1, 5, ellipse1[min_x], ellipse1[max_x], -1, testingPolygon1);
                std::vector<D2D1_POINT_2F> tmp1;
                tmp1.push_back(testingPolygon1.polygon[0]);
                for (int i = 1; i < testingPolygon1.polygon.size(); i++) {

                    bool repeated = false;
                    for (int j = 0; j < tmp1.size(); j++) {
                        if (tmp1[j].x == testingPolygon1.polygon[i].x && tmp1[j].y == testingPolygon1.polygon[i].y) {
                            repeated = true;
                        }

                    }
                    if (!repeated) {
                        tmp1.push_back(testingPolygon1.polygon[i]);
                    }
                }

                ClockwiseSortPoints(tmp1);
                testingPolygon1.polygon = tmp1;
                testingPolygon1.SetColor(D2D1::ColorF(D2D1::ColorF::HotPink));
                testingPolygon1.Draw(pRenderTarget, pBrush);

                //draw the second polygon
                testingPolygon2.polygon.clear();
                min_x = 0;
                max_x = 0;
                for (int i = 0; i < 5; i++)
                {
                    if (ellipse2[i].x < ellipse2[min_x].x)
                        min_x = i;
                    if (ellipse2[i].x > ellipse2[max_x].x)
                        max_x = i;
                }

                quickHull(ellipse2, 5, ellipse2[min_x], ellipse2[max_x], 1, testingPolygon2);
                quickHull(ellipse2, 5, ellipse2[min_x], ellipse2[max_x], -1, testingPolygon2);
                std::vector<D2D1_POINT_2F> tmp2;
                tmp2.push_back(testingPolygon2.polygon[0]);
                for (int i = 1; i < testingPolygon2.polygon.size(); i++) {

                    bool repeated = false;
                    for (int j = 0; j < tmp2.size(); j++) {
                        if (tmp2[j].x == testingPolygon2.polygon[i].x && tmp2[j].y == testingPolygon2.polygon[i].y) {
                            repeated = true;
                        }

                    }
                    if (!repeated) {
                        tmp2.push_back(testingPolygon2.polygon[i]);
                    }
                }

                ClockwiseSortPoints(tmp2);
                testingPolygon2.polygon = tmp2;
                testingPolygon2.SetColor(D2D1::ColorF(D2D1::ColorF::HotPink));
                testingPolygon2.Draw(pRenderTarget, pBrush);

                // do difference here
                testingPolygon.polygon.clear();
                min_x = 0;
                max_x = 0;
                for (int i = 0; i < 5; i++) {
                    for (int j = 0; j < 5; j++) {
                        ellipseMsk[i * 5 + j] = { ellipse1[i].x - ellipse2[j].x + 750, ellipse1[i].y - ellipse2[j].y + 360 };
                    }
                }
                for (int i = 0; i < 25; i++)
                {
                    if (ellipseMsk[i].x < ellipseMsk[min_x].x)
                        min_x = i;
                    if (ellipseMsk[i].x > ellipseMsk[max_x].x)
                        max_x = i;
                }

                quickHull(ellipseMsk, 25, ellipseMsk[min_x], ellipseMsk[max_x], 1, testingPolygon);
                quickHull(ellipseMsk, 25, ellipseMsk[min_x], ellipseMsk[max_x], -1, testingPolygon);
                std::vector<D2D1_POINT_2F> tmp;
                tmp.push_back(testingPolygon.polygon[0]);
                for (int i = 1; i < testingPolygon.polygon.size(); i++) {

                    bool repeated = false;
                    for (int j = 0; j < tmp.size(); j++) {
                        if (tmp[j].x == testingPolygon.polygon[i].x && tmp[j].y == testingPolygon.polygon[i].y) {
                            repeated = true;
                        }

                    }
                    if (!repeated) {
                        tmp.push_back(testingPolygon.polygon[i]);
                    }
                }

                ClockwiseSortPoints(tmp);
                testingPolygon.polygon = tmp;
                bool colliding = testingPolygon.contains(D2D1::Point2F(750.0f, 350.0f));
                if (colliding) {
                    testingPolygon.SetColor(D2D1::ColorF(D2D1::ColorF::Yellow));
                }
                else {
                    testingPolygon.SetColor(D2D1::ColorF(D2D1::ColorF::LightSkyBlue));
                }
                testingPolygon.Draw(pRenderTarget, pBrush);
                break;
            }
        }


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


int MainWindow::FindSide(Vector2D p1, Vector2D p2, Vector2D p)
{
    float val = (p.y - p1.y) * (p2.x - p1.x) -
        (p2.y - p1.y) * (p.x - p1.x);

    if (val > 0)
        return 1;
    if (val < 0)
        return -1;
    return 0;
}
float MainWindow::dist(Vector2D p, Vector2D q)
{
    return abs((p.y - q.y) * (p.y - q.y) +
        (p.x - q.x) * (p.x - q.x));
}
float MainWindow::lineDist(Vector2D p1, Vector2D p2, Vector2D p)
{
    return abs((p.y - p1.y) * (p2.x - p1.x) -
        (p2.y - p1.y) * (p.x - p1.x));
}
bool  MainWindow::PointCmp(const D2D1_POINT_2F& a, const D2D1_POINT_2F& b, const D2D1_POINT_2F& center)
{
    if (a.x >= 0 && b.x < 0)
        return true;
    if (a.x == 0 && b.x == 0)
        return a.y > b.y;
    float det = (a.x - center.x) * (b.y - center.y) - (b.x - center.x) * (a.y - center.y);
    if (det < 0)
        return true;
    if (det > 0)
        return false;
    float d1 = (a.x - center.x) * (a.x - center.x) + (a.y - center.y) * (a.y - center.y);
    float d2 = (b.x - center.x) * (b.x - center.y) + (b.y - center.y) * (b.y - center.y);
    return d1 > d2;
}
void  MainWindow::ClockwiseSortPoints(std::vector<D2D1_POINT_2F>& vPoints)
{
    D2D1_POINT_2F center;
    double x = 0, y = 0;
    for (int i = 0; i < vPoints.size(); i++)
    {
        x += vPoints[i].x;
        y += vPoints[i].y;
    }
    center.x = (float)x / vPoints.size();
    center.y = (float)y / vPoints.size();

    for (int i = 0; i < vPoints.size() - 1; i++)
    {
        for (int j = 0; j < vPoints.size() - i - 1; j++)
        {
            if (PointCmp(vPoints[j], vPoints[j + 1], center))
            {
                D2D1_POINT_2F tmp = vPoints[j];
                vPoints[j] = vPoints[j + 1];
                vPoints[j + 1] = tmp;
            }
        }
    }
}
void MainWindow::quickHull(Vector2D a[], int n, Vector2D p1, Vector2D p2, int side, MyPolygon& mypolygon)
{
    int ind = -1;
    float max_dist = 0;

    for (int i = 0; i < n; i++)
    {
        float temp = lineDist(p1, p2, a[i]);
        if (FindSide(p1, p2, a[i]) == side && temp > max_dist)
        {
            ind = i;
            max_dist = temp;
        }
    }

    if (ind == -1)
    {
        bool tmp = true;
        for (int i = 0; i < mypolygon.polygon.size(); i++) {
            if (mypolygon.polygon[i].x == p1.x && mypolygon.polygon[i].y == p1.y) {
                tmp = false;
            }
        }
        if (tmp) {
            mypolygon.polygon.push_back(D2D1::Point2F(p1.x, p1.y));
        }
        tmp = true;
        for (int i = 0; i < mypolygon.polygon.size(); i++) {
            if (mypolygon.polygon[i].x == p2.x && mypolygon.polygon[i].y == p2.y) {
                tmp = false;
            }
        }
        if (tmp) {
            mypolygon.polygon.push_back(D2D1::Point2F(p2.x, p2.y));
        }
        return;
    }

    quickHull(a, n, a[ind], p1, -FindSide(a[ind], p1, p2), mypolygon);
    quickHull(a, n, a[ind], p2, -FindSide(a[ind], p2, p1), mypolygon);
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

            if (demoSelection == 2 || demoSelection == 1 || demoSelection == 5) {
                for (int i = 0; i < sizeof(ellipse2) / sizeof(Vector2D); i++) {
                    if (Selection()->ellipse.point.x == ellipse2[i].x && Selection()->ellipse.point.y == ellipse2[i].y) {
                        ellipse2[i].x = dipX + ptMouse.x;
                        ellipse2[i].y = dipY + ptMouse.y;
                    }
                    if (Selection()->ellipse.point.x == ellipse1[i].x && Selection()->ellipse.point.y == ellipse1[i].y) {
                        ellipse1[i].x = dipX + ptMouse.x;
                        ellipse1[i].y = dipY + ptMouse.y;
                    }
                }

                Selection()->ellipse.point.x = dipX + ptMouse.x;
                Selection()->ellipse.point.y = dipY + ptMouse.y;
                D2D1_POINT_2F  point = { Selection()->ellipse.point.x , Selection()->ellipse.point.y };

            }
            // Move the ellipse.
            else {
                for (int i = 0; i < sizeof(ellipse) / sizeof(Vector2D); i++) {
                    if (Selection()->ellipse.point.x == ellipse[i].x && Selection()->ellipse.point.y == ellipse[i].y) {
                        ellipse[i].x = dipX + ptMouse.x;
                        ellipse[i].y = dipY + ptMouse.y;
                    }
                }
                Selection()->ellipse.point.x = dipX + ptMouse.x;
                Selection()->ellipse.point.y = dipY + ptMouse.y;
                D2D1_POINT_2F  point = { Selection()->ellipse.point.x , Selection()->ellipse.point.y };


                if (demoSelection == 4) {
                    if (testingPolygon.contains(point)) {
                        Selection()->color.r = 100.0f;
                        Selection()->color.g = 0.0;
                        Selection()->color.b = 0.0;
                    }
                    else {
                        Selection()->color.r = 0.0f;
                        Selection()->color.g = 100.0;
                        Selection()->color.b = 0.0;

                    }

                }
            }
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
void MainWindow::GenerateRandomPoints(int num) {
    // Start drawing points
    if (ModeFlag == 1 || ModeFlag == 5) {
        for (int i = 0; i < num; i++) {
            float x = FLOAT(rand() % 200 + 500);
            float y = FLOAT(rand() % 250 + 50);
            InsertEllipse(x, y);
            ellipse1[i] = { x,y };
        }
        for (int i = 0; i < num; i++) {
            float x = FLOAT(rand() % 200 + 800);
            float y = FLOAT(rand() % 250 + 50);
            InsertEllipse(x, y);
            ellipse2[i] = { x,y };
        }
    }
    else if(ModeFlag == 2){
        for (int i = 0; i < num; i++) {
            float x = FLOAT(rand() % 200 + 500);
            float y = FLOAT(rand() % 250 + 50);
            InsertEllipse(x, y);
            ellipse1[i] = { x,y };
        }
        for (int i = 0; i < num; i++) {
            float x = FLOAT(rand() % 200 + 800);
            float y = FLOAT(rand() % 250 + 400);
            InsertEllipse(x, y);
            ellipse2[i] = { x,y };
        }
    }
    else {
        for (int i = 0; i < num; i++) {
            float x = FLOAT(rand() % 900 + 300);
            float y = FLOAT(rand() % 550 + 50);
            InsertEllipse(x, y);
            ellipse[i] = { x,y };
        }
    }

}
void MainWindow::DrawTestPolygon(int flag) {
    if (flag == 1) {
        // Draw Polygon!
        std::vector<D2D1_POINT_2F> points;
        D2D1_POINT_2F point1 = D2D1::Point2F(500.0f, 140.0f);
        D2D1_POINT_2F point2 = D2D1::Point2F(700.0f, 220.0f);
        D2D1_POINT_2F point3 = D2D1::Point2F(800.0f, 340.0f);
        D2D1_POINT_2F point4 = D2D1::Point2F(530.0f, 440.0f);
        D2D1_POINT_2F point5 = D2D1::Point2F(500.0f, 540.0f);
        D2D1_POINT_2F point6 = D2D1::Point2F(450.0f, 340.0f);
        points.push_back(point1);
        points.push_back(point2);
        points.push_back(point3);
        points.push_back(point4);
        points.push_back(point5);
        points.push_back(point6);
        testingPolygon.polygon = points;
        testingPolygon.SetColor(D2D1::ColorF(D2D1::ColorF::HotPink));
        testingPolygon.Draw(pRenderTarget, pBrush);
    }
    if (flag == 2) {
        // Draw another Polygon!
        std::vector<D2D1_POINT_2F> points2;
        D2D1_POINT_2F point1 = D2D1::Point2F(800.0f, 200.0f);
        D2D1_POINT_2F point2 = D2D1::Point2F(800.0f, 400.0f);
        D2D1_POINT_2F point3 = D2D1::Point2F(900.0f, 500.0f);
        D2D1_POINT_2F point4 = D2D1::Point2F(1000.0f, 400.0f);
        D2D1_POINT_2F point5 = D2D1::Point2F(1000.0f, 200.0f);
        D2D1_POINT_2F point6 = D2D1::Point2F(900.0f, 100.0f);
        points2.push_back(point1);
        points2.push_back(point2);
        points2.push_back(point3);
        points2.push_back(point4);
        points2.push_back(point5);
        points2.push_back(point6);
        testingPolygon2.polygon = points2;
        testingPolygon2.SetColor(D2D1::ColorF(D2D1::ColorF::LightSkyBlue));
        testingPolygon2.Draw(pRenderTarget, pBrush);
    }
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
        case click_one:
            // Set selection for demo
            demoSelection = 1;
            // Clear area and trigger repaint
            InvalidateRect(m_hwnd, NULL, true);
            // Pop-up Message Box
            //MessageBox(m_hwnd, L"Insert Minkowski Difference Demo Here", L"Minkowski Difference", MB_OK);
            break;
        case click_two:
            // Set selection for demo
            demoSelection = 2;
            // Clear area and trigger repaint
            InvalidateRect(m_hwnd, NULL, true);
            // Pop-up Message Box
            //MessageBox(m_hwnd, L"Insert Minkowski Sum Demo Here", L"Minkowski Sum", MB_OK);
            break;
        case click_three:
            // Set selection for demo
            demoSelection = 3;
            // Clear area and trigger repaint
            InvalidateRect(m_hwnd, NULL, true);
            // Pop-up Message Box
            //MessageBox(m_hwnd, L"Insert Quickhull Demo Here", L"Quickhull", MB_OK);
            break;
        case click_four:
            // Set selection for demo
            demoSelection = 4;
            // Clear area and trigger repaint
            InvalidateRect(m_hwnd, NULL, true);
            // Pop-up Message Box
            //MessageBox(m_hwnd, L"Insert Point Convex Hull Demo Here", L"Point Convex Hul", MB_OK);
            break;
        case click_five:
            // Set selection for demo
            demoSelection = 5;
            // Clear area and trigger repaint
            InvalidateRect(m_hwnd, NULL, true);
            // Pop-up Message Box
            //MessageBox(m_hwnd, L"Insert GJK Demo Here", L"GJK", MB_OK);
            break;
        }
        return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}
