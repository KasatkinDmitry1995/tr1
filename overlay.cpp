// overlay.cpp
#include "overlay.h"
#include <cstdarg>

namespace {
    HWND  g_overlayWnd = nullptr;
    HWND  g_targetWnd = nullptr;
    HANDLE g_overlayThread = nullptr;
    DWORD g_overlayThreadId = 0;
    bool  g_running = false;

    int   g_cx = 0, g_cy = 0;

    // Глобальный контейнер данных
    OverlayData g_data;
}

// ---------- Поиск окна CS 1.6 ----------
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    wchar_t title[256];
    GetWindowTextW(hwnd, title, 256);
    if (wcsstr(title, L"Counter-Strike") != nullptr && IsWindowVisible(hwnd)) {
        *(HWND*)lParam = hwnd;
        return FALSE;
    }
    return TRUE;
}

HWND FindCSWindow() {
    HWND result = nullptr;
    EnumWindows(EnumWindowsProc, (LPARAM)&result);
    return result;
}

// ---------- Отрисовка ----------
void DrawOverlay(HWND hwnd) {
    HDC hdcScreen = GetDC(hwnd);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmMem = CreateCompatibleBitmap(hdcScreen, g_cx, g_cy);
    HGDIOBJ oldBmp = SelectObject(hdcMem, hbmMem);

    // Чёрный фон (прозрачный)
    RECT rc = { 0, 0, g_cx, g_cy };
    HBRUSH bg = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdcMem, &rc, bg);
    DeleteObject(bg);

    // Рисуем всё, что накопилось в g_data.
    // Блокируем мьютекс на время отрисовки, чтобы данные не изменились посередине.
    {
        std::lock_guard<std::mutex> lock(g_data.mtx);

        // Залитые прямоугольники
        for (const auto& r : g_data.filledRects) {
            RECT rr = { (LONG)r.x, (LONG)r.y,
                        (LONG)(r.x + r.w), (LONG)(r.y + r.h) };
            HBRUSH brush = CreateSolidBrush(r.color);
            FillRect(hdcMem, &rr, brush);
            DeleteObject(brush);
        }

        // Линии
        for (const auto& l : g_data.lines) {
            HPEN pen = CreatePen(PS_SOLID, (int)l.thickness, l.color);
            HGDIOBJ oldPen = SelectObject(hdcMem, pen);
            MoveToEx(hdcMem, (int)l.x1, (int)l.y1, nullptr);
            LineTo(hdcMem, (int)l.x2, (int)l.y2);
            SelectObject(hdcMem, oldPen);
            DeleteObject(pen);
        }

        // Прямоугольники
        for (const auto& r : g_data.rects) {
            HPEN pen = CreatePen(PS_SOLID, (int)r.thickness, r.color);
            HGDIOBJ oldPen = SelectObject(hdcMem, pen);
            HGDIOBJ oldBrush = SelectObject(hdcMem, GetStockObject(NULL_BRUSH));
            Rectangle(hdcMem, (int)r.x, (int)r.y,
                (int)(r.x + r.w), (int)(r.y + r.h));
            SelectObject(hdcMem, oldBrush);
            SelectObject(hdcMem, oldPen);
            DeleteObject(pen);
        }

        // Круги
        for (const auto& c : g_data.circles) {
            HPEN pen = CreatePen(PS_SOLID, (int)c.thickness, c.color);
            HGDIOBJ oldPen = SelectObject(hdcMem, pen);
            HGDIOBJ oldBrush = SelectObject(hdcMem, GetStockObject(NULL_BRUSH));
            Ellipse(hdcMem,
                (int)(c.cx - c.radius), (int)(c.cy - c.radius),
                (int)(c.cx + c.radius), (int)(c.cy + c.radius));
            SelectObject(hdcMem, oldBrush);
            SelectObject(hdcMem, oldPen);
            DeleteObject(pen);
        }

        // Текст
        SetBkMode(hdcMem, TRANSPARENT);
        for (const auto& t : g_data.texts) {
            SetTextColor(hdcMem, t.color);
            TextOutW(hdcMem, (int)t.x, (int)t.y, t.text, (int)wcslen(t.text));
        }

        // Линии
        for (const auto& l : g_data.wrects) {
            HPEN pen = CreatePen(PS_SOLID, (int)l.thickness, l.color);
            HGDIOBJ oldPen = SelectObject(hdcMem, pen);
            MoveToEx(hdcMem, (int)l.p1.x, (int)l.p1.y, nullptr);
            LineTo(hdcMem, (int)l.p2.x, (int)l.p2.y);
            LineTo(hdcMem, (int)l.p3.x, (int)l.p3.y);
            LineTo(hdcMem, (int)l.p4.x, (int)l.p4.y);
            LineTo(hdcMem, (int)l.p1.x, (int)l.p1.y);
            SelectObject(hdcMem, oldPen);
            DeleteObject(pen);
        }
    }

    BitBlt(hdcScreen, 0, 0, g_cx, g_cy, hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdcMem, oldBmp);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdcScreen);
}

// ---------- Процедура окна ----------
LRESULT CALLBACK OverlayProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        DrawOverlay(hwnd);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_TIMER: {
        if (!g_targetWnd || !IsWindow(g_targetWnd)) {
            g_running = false;
            PostQuitMessage(0);
            return 0;
        }

        RECT r;
        GetClientRect(g_targetWnd, &r);
        POINT pt = { 0, 0 };
        ClientToScreen(g_targetWnd, &pt);

        int newCx = r.right - r.left;
        int newCy = r.bottom - r.top;

        SetWindowPos(hwnd, HWND_TOPMOST, pt.x, pt.y, newCx, newCy,
            SWP_NOACTIVATE | SWP_SHOWWINDOW);

        if (newCx != g_cx || newCy != g_cy) {
            g_cx = newCx;
            g_cy = newCy;
        }

        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ---------- Создание/уничтожение ----------
bool CreateOverlayInternal() {
    g_targetWnd = FindCSWindow();
    if (!g_targetWnd) {
        MessageBoxW(nullptr, L"Окно CS 1.6 не найдено.", L"Ошибка", MB_ICONERROR);
        return false;
    }

    RECT r;
    GetClientRect(g_targetWnd, &r);
    POINT pt = { 0, 0 };
    ClientToScreen(g_targetWnd, &pt);
    g_cx = r.right - r.left;
    g_cy = r.bottom - r.top;

    HINSTANCE hInstance = GetModuleHandleW(nullptr);
    const wchar_t CLASS_NAME[] = L"CSOverlayWindow";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = OverlayProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = nullptr;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    g_overlayWnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASS_NAME, L"", WS_POPUP,
        pt.x, pt.y, g_cx, g_cy,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!g_overlayWnd) {
        MessageBoxW(nullptr, L"Не удалось создать окно.", L"Ошибка", MB_ICONERROR);
        return false;
    }

    SetLayeredWindowAttributes(g_overlayWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    ShowWindow(g_overlayWnd, SW_SHOW);
    UpdateWindow(g_overlayWnd);
    SetTimer(g_overlayWnd, 1, 5, nullptr);
    return true;
}

void DestroyOverlayInternal() {
    if (g_overlayWnd) {
        KillTimer(g_overlayWnd, 1);
        DestroyWindow(g_overlayWnd);
        g_overlayWnd = nullptr;
    }
    g_targetWnd = nullptr;
}

DWORD WINAPI OverlayThreadProc(LPVOID) {
    if (!CreateOverlayInternal()) {
        g_running = false;
        return 1;
    }
    MSG msg;
    while (g_running && GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    DestroyOverlayInternal();
    g_running = false;
    return 0;
}

// ---------- Публичные функции ----------
HANDLE StartOverlayThread() {
    if (g_running) return g_overlayThread;
    g_running = true;
    g_overlayThread = CreateThread(nullptr, 0, OverlayThreadProc,
        nullptr, 0, &g_overlayThreadId);
    return g_overlayThread;
}

void StopOverlayThread() {
    g_running = false;
    if (g_overlayWnd) PostMessage(g_overlayWnd, WM_CLOSE, 0, 0);
    if (g_overlayThread) {
        WaitForSingleObject(g_overlayThread, 3000);
        CloseHandle(g_overlayThread);
        g_overlayThread = nullptr;
    }
}

OverlayData& GetOverlayData() {
    return g_data;
}