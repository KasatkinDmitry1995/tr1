// overlay.h
#pragma once

#include <Windows.h>
#include "Structs.h"
#include <vector>
#include <string>
#include <cstdarg>
#include <cstdio>

// ---------- Цвет ----------
struct OverlayColor {
    unsigned char r, g, b, a;
    OverlayColor() : r(255), g(255), b(255), a(255) {}
    OverlayColor(unsigned char r_, unsigned char g_,
        unsigned char b_, unsigned char a_ = 255)
        : r(r_), g(g_), b(b_), a(a_) {
    }
};

// ---------- Примитивы ----------
struct Line { float x1, y1, x2, y2; float thickness; OverlayColor color; };
struct RectShape { float x, y, w, h; float thickness; OverlayColor color; };
struct CircleShape { float cx, cy, radius; float thickness; OverlayColor color; };
struct FilledRect { float x, y, w, h; OverlayColor color; };
struct TextLabel { float x, y; float size; OverlayColor color; std::wstring text; };

// ---------- Контейнер данных ----------
struct OverlayData {
    std::vector<Line>        lines;
    std::vector<RectShape>   rects;
    std::vector<CircleShape> circles;
    std::vector<FilledRect>  filledRects;
    std::vector<TextLabel>   texts;
    int width = 0, height = 0;

    void Clear() {
        lines.clear();
        rects.clear();
        circles.clear();
        filledRects.clear();
        texts.clear();
    }

    void AddLine(float x1, float y1, float x2, float y2,
        OverlayColor color, float thickness = 1.0f) {
        lines.push_back({ x1, y1, x2, y2, thickness, color });
    }
    void AddRect(float x, float y, float w, float h,
        OverlayColor color, float thickness = 1.0f) {
        rects.push_back({ x, y, w, h, thickness, color });
    }
    void AddCircle(float cx, float cy, float radius,
        OverlayColor color, float thickness = 1.0f) {
        circles.push_back({ cx, cy, radius, thickness, color });
    }
    void AddFilledRect(float x, float y, float w, float h, OverlayColor color) {
        filledRects.push_back({ x, y, w, h, color });
    }
    void AddText(float x, float y, OverlayColor color,
        float size, const wchar_t* fmt, ...) {
        TextLabel label;
        label.x = x; label.y = y; label.size = size; label.color = color;
        wchar_t buf[512];
        va_list args; va_start(args, fmt);
        vswprintf_s(buf, 512, fmt, args);
        va_end(args);
        label.text = buf;
        texts.push_back(std::move(label));
    }
    void AddText(float x, float y, OverlayColor color,
        const wchar_t* fmt, ...) {
        TextLabel label;
        label.x = x; label.y = y; label.size = 0.0f; label.color = color;
        wchar_t buf[512];
        va_list args; va_start(args, fmt);
        vswprintf_s(buf, 512, fmt, args);
        va_end(args);
        label.text = buf;
        texts.push_back(std::move(label));
    }
};

extern OverlayData data;

// ---------- Публичный API оверлея ----------

// Создаёт окно, инициализирует D3D11 + ImGui. Вызывать в том потоке,
// где будет крутиться цикл.
bool Overlay_Init(HWND targetWnd);

// Уничтожает всё. Вызывать в том же потоке, что Init.
void Overlay_Shutdown();

// Начать новый кадр: очистка, ImGui::NewFrame
void Overlay_BeginFrame();

// Закончить кадр: отрисовать data, Present
void Overlay_EndFrame();

// Проверить, не пришло ли WM_QUIT. Возвращает false, если пора выходить.
bool Overlay_PumpMessages();

// Получить HWND оверлея (для WM_NCHITTEST и т.п.)
HWND Overlay_GetHwnd();

void SetClickThrough(bool enabled);