// FrameTimer.cpp
#include "FrameTimer.h"

#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

FrameTimer::FrameTimer() {
    QueryPerformanceFrequency(&m_freq);

    // 1. Пытаемся создать high-resolution таймер (Windows 10 1803+)
    m_hTimer = CreateWaitableTimerExW(
        nullptr, nullptr,
        CREATE_WAITABLE_TIMER_MANUAL_RESET | CREATE_WAITABLE_TIMER_HIGH_RESOLUTION,
        TIMER_ALL_ACCESS);

    if (m_hTimer) {
        m_highResTimer = true;
    }
    else {
        // 2. Fallback: обычный waitable timer + timeBeginPeriod(1)
        m_hTimer = CreateWaitableTimerW(nullptr, TRUE, nullptr);
        if (m_hTimer) {
            timeBeginPeriod(1);
            m_timePeriodSet = true;
        }
    }
}

FrameTimer::~FrameTimer() {
    if (m_hTimer) {
        CloseHandle(m_hTimer);
        m_hTimer = nullptr;
    }
    if (m_timePeriodSet) {
        timeEndPeriod(1);
        m_timePeriodSet = false;
    }
}

void FrameTimer::SetTargetFPS(double fps) {
    if (fps <= 0.0) {
        m_targetFrameMs = 0.0;
    }
    else {
        m_targetFrameMs = 1000.0 / fps;
    }
}

void FrameTimer::BeginFrame() {
    QueryPerformanceCounter(&m_frameStart);
}

void FrameTimer::EndFrame() {
    if (m_targetFrameMs <= 0.0) {
        // Без ограничения FPS — просто отдаём квант
        SwitchToThread();
        return;
    }

    // Целевое время следующего кадра
    LONGLONG targetTicks = m_frameStart.QuadPart
        + (LONGLONG)(m_targetFrameMs * m_freq.QuadPart / 1000.0);

    LARGE_INTEGER targetTime;
    targetTime.QuadPart = targetTicks;

    // Проверяем, не пора ли уже
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    if (now.QuadPart >= targetTicks) {
        // Опоздали — не ждём
        return;
    }

    WaitUntil(targetTime);
}

double FrameTimer::GetFrameTimeMs() const {
    return ElapsedMs(m_frameStart);
}

double FrameTimer::ElapsedMs(LARGE_INTEGER since) const {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return double(now.QuadPart - since.QuadPart) * 1000.0 / m_freq.QuadPart;
}

void FrameTimer::WaitUntil(LARGE_INTEGER targetTime) const {
    // Переводим targetTime в абсолютное время FILETIME для SetWaitableTimer.
    // SetWaitableTimer ожидает абсолютное время в 100-нс интервалах, начиная с 1601 года.
    // Проще использовать относительный интервал — вычисляем, сколько осталось.

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    double remainingMs = double(targetTime.QuadPart - now.QuadPart)
        * 1000.0 / m_freq.QuadPart;

    if (remainingMs <= 0.0) return;

    // Округляем вниз до ближайшей миллисекунды, чтобы не переспать
    LONGLONG waitMs = (LONGLONG)remainingMs;
    if (waitMs < 1) waitMs = 1;

    // Отрицательное значение = относительный интервал (100-нс единицы)
    LARGE_INTEGER dueTime;
    dueTime.QuadPart = -(waitMs * 10000LL);  // 1 мс = 10000 * 100нс

    SetWaitableTimer(m_hTimer, &dueTime, 0, nullptr, nullptr, FALSE);
    WaitForSingleObject(m_hTimer, INFINITE);

    // После ожидания может остаться остаток меньше 1 мс — добираем коротким spin
    QueryPerformanceCounter(&now);
    if (now.QuadPart < targetTime.QuadPart) {
        // Короткое ожидание — не жжёт CPU, если остаток большой
        while (now.QuadPart < targetTime.QuadPart) {
            YieldProcessor();  // hint для CPU
            QueryPerformanceCounter(&now);
        }
    }
}