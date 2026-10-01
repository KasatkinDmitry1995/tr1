// menu.h
#pragma once

namespace Menu {

    // Настройки, которые читает логика
    struct Settings {
        bool  triggerbotEnabled = false;
        int   triggerHoldMs = 100;   // сколько держать ЛКМ
        bool  espEnabled = true;
        bool  showNames = true;

        // Управление из меню
        bool  requestExit = false; // "OK" — выйти из программы
        bool  requestUnload = false; // "Выключить чит" — остановить логику
    };

    Settings& Get();

    void Render();      // вызывать каждый кадр
    bool IsOpen();      // открыто ли меню
    void Toggle();      // переключить видимость
}