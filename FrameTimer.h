// FrameTimer.h
#pragma once

#include <Windows.h>

class FrameTimer {
public:
    FrameTimer();
    ~FrameTimer();

    // Запрещаем копирование — таймер уникален
    FrameTimer(const FrameTimer&) = delete;
    FrameTimer& operator=(const FrameTimer&) = delete;

    // Установить целевой FPS. 0 = без ограничения.
    void SetTargetFPS(double fps);

    // Вызвать в начале каждого кадра — запоминает время старта
    void BeginFrame();

    // Вызвать в конце кадра — ждёт до наступления следующего кадра
    void EndFrame();

    // Сколько миллисекунд прошло с начала текущего кадра
    double GetFrameTimeMs() const;

private:
    HANDLE m_hTimer = nullptr;
    bool   m_highResTimer = false;
    bool   m_timePeriodSet = false;

    double m_targetFrameMs = 0.0;  // 0 = без ограничения

    LARGE_INTEGER m_freq = {};
    LARGE_INTEGER m_frameStart = {};

    double ElapsedMs(LARGE_INTEGER since) const;
    void   WaitUntil(LARGE_INTEGER targetTime) const;
};
