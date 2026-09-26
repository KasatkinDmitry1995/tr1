// overlay.h
#pragma once

#include <Windows.h>
#include <vector>
#include <mutex>

// ---------- Примитивы ----------
struct Line {
    float x1, y1, x2, y2;
    float thickness;
    COLORREF color;
};

struct RectShape {
    float x, y, w, h;
    float thickness;
    COLORREF color;
};

struct CircleShape {
    float cx, cy, radius;
    float thickness;
    COLORREF color;
};

struct FilledRect {
    float x, y, w, h;
    COLORREF color;
};

struct TextLabel {
    float x, y;
    COLORREF color;
    wchar_t text[128];
};

// ---------- Контейнер данных ----------
struct OverlayData {
    std::vector<Line>        lines;
    std::vector<RectShape>   rects;
    std::vector<CircleShape> circles;
    std::vector<FilledRect>  filledRects;
    std::vector<TextLabel>   texts;

    std::mutex mtx; // защита при доступе из разных потоков

    void Clear() {
        std::lock_guard<std::mutex> lock(mtx);
        lines.clear();
        rects.clear();
        circles.clear();
        filledRects.clear();
        texts.clear();
    }

    void AddLine(float x1, float y1, float x2, float y2,
        COLORREF color, float thickness = 1.0f) {
        std::lock_guard<std::mutex> lock(mtx);
        lines.push_back({ x1, y1, x2, y2, thickness, color });
    }

    void AddRect(float x, float y, float w, float h,
        COLORREF color, float thickness = 1.0f) {
        std::lock_guard<std::mutex> lock(mtx);
        rects.push_back({ x, y, w, h, thickness, color });
    }

    void AddCircle(float cx, float cy, float radius,
        COLORREF color, float thickness = 1.0f) {
        std::lock_guard<std::mutex> lock(mtx);
        circles.push_back({ cx, cy, radius, thickness, color });
    }

    void AddFilledRect(float x, float y, float w, float h, COLORREF color) {
        std::lock_guard<std::mutex> lock(mtx);
        filledRects.push_back({ x, y, w, h, color });
    }

    void AddText(float x, float y, COLORREF color, const wchar_t* fmt, ...) {
        TextLabel label = {};
        label.x = x;
        label.y = y;
        label.color = color;

        va_list args;
        va_start(args, fmt);
        vswprintf_s(label.text, 128, fmt, args);
        va_end(args);

        std::lock_guard<std::mutex> lock(mtx);
        texts.push_back(label);
    }
};

// ---------- Публичный интерфейс ----------

// Запускает оверлей. Никаких callback — просто рисует то,
// что лежит в OverlayData.
HANDLE StartOverlayThread();

// Останавливает оверлей.
void StopOverlayThread();

// Возвращает глобальный OverlayData — заполняйте откуда угодно.
OverlayData& GetOverlayData();