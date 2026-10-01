#include <windows.h>
#include <TlHelp32.h>
#include <iostream>

#include "Game.h"
#include "overlay.h"
#include "DrawHelper.h"
#include "FrameTimer.h"
#include "Menu.h"

namespace {

    struct AppState {
        Game         game;
        DrawHelper   dH;
        bool         running = true;
        bool         triggerEnabled = true;
        _IN_CROSS_OBJECT prevCross = _IN_CROSS_OBJECT::INC_CROSS_CLEAR;
        bool insertWasDown = false;
        HWND gameWnd = 0;
    } g_app;

    Menu::Settings& cfg = Menu::Get();


    HWND FindCSWindow() {
        HWND targetWnd = nullptr;
        EnumWindows([](HWND hwnd, LPARAM lp) -> BOOL {
            wchar_t title[256];
            GetWindowTextW(hwnd, title, 256);
            if (wcsstr(title, L"Counter-Strike") && IsWindowVisible(hwnd)) {
                *(HWND*)lp = hwnd;
                return FALSE;
            }
            return TRUE;
            }, (LPARAM)&targetWnd);
        return targetWnd;
    }

    bool InitApp() {
        std::cout << "Searching for process hl.exe...." << std::endl;
        if (!g_app.game.FindGameProccess()) {
            std::cout << "Process not found." << std::endl;
            return false;
        }
        std::cout << "HL process found successfully." << std::endl;

        if (!g_app.game.OpenGameProcess()) {
            std::cout << "OpenProcess failed: " << GetLastError() << std::endl;
            return false;
        }

        g_app.gameWnd = FindCSWindow();
        if (!g_app.gameWnd) {
            MessageBoxW(nullptr, L"Окно CS 1.6 не найдено.",
                L"Ошибка", MB_ICONERROR);
            return false;
        }

        if (!Overlay_Init(g_app.gameWnd)) {
            MessageBoxW(nullptr, L"Не удалось инициализировать оверлей.",
                L"Ошибка", MB_ICONERROR);
            return false;
        }

        g_app.dH.cam.fov = 90.0f;
        return true;
    }

    void ShutdownApp() {
        Overlay_Shutdown();
        g_app.game.CloseHandles();
    }


    void HandleHotkeys() {

        if (GetAsyncKeyState(VK_F3) & 0b1) {
            g_app.triggerEnabled = !g_app.triggerEnabled;
            std::cout << "Triggerbot is "
                << (g_app.triggerEnabled ? "enabled" : "disabled") << std::endl;
        }

        if (GetAsyncKeyState(VK_F4) & 0b1) {
            std::cout << "Triggerbot closed by user" << std::endl;
            g_app.running = false;
        }

        bool insertDown = (GetAsyncKeyState(VK_INSERT) & 0x8000) != 0;
        if (insertDown && !g_app.insertWasDown)
        {
            Menu::Toggle();
            SetClickThrough(!Menu::IsOpen());

            if (Menu::IsOpen()) {
                SetForegroundWindow(Overlay_GetHwnd());
            }
            else {
                SetForegroundWindow(g_app.gameWnd);
            }

        }

        g_app.insertWasDown = insertDown;
    }

    void UpdateTriggerbot() {
        if (!g_app.triggerEnabled || !g_app.game.IsGameFocused())
            return;

        auto cross = g_app.game.GetInCrossObject();

        if (g_app.prevCross != cross) {
            switch (cross) {
            case _IN_CROSS_OBJECT::INC_ENEMY:
                std::cout << "Enemy in the cross...." << std::endl; break;
            case _IN_CROSS_OBJECT::INC_FRIEND:
                std::cout << "Friend in the cross....." << std::endl; break;
            case _IN_CROSS_OBJECT::INC_HOSTAGE:
                std::cout << "Hostage in the cross..." << std::endl; break;
            }
            g_app.prevCross = cross;
        }

        if (cross == _IN_CROSS_OBJECT::INC_ENEMY
            && !g_app.game.IsUserInSpects())
        {
            if (g_app.game.SendFire(cfg.triggerHoldMs))
                std::cout << "firing...." << std::endl;
        }
    }

    void DrawPlayerESP(const PlayerInfo& pi, const PlayerView& pv) {
        OverlayColor color = pi.isT
            ? OverlayColor(255, 50, 50, 200)
            : OverlayColor(50, 50, 255, 200);

        Vec2 p[8];
        if (!(g_app.dH.WorldToScreen({ pi.coords.X + 15, pi.coords.Y + 15, pi.coords.Z + 10 }, p[0])
            && g_app.dH.WorldToScreen({ pi.coords.X + 15, pi.coords.Y + 15, pi.coords.Z - 50 }, p[1])
            && g_app.dH.WorldToScreen({ pi.coords.X + 15, pi.coords.Y - 15, pi.coords.Z + 10 }, p[2])
            && g_app.dH.WorldToScreen({ pi.coords.X + 15, pi.coords.Y - 15, pi.coords.Z - 50 }, p[3])
            && g_app.dH.WorldToScreen({ pi.coords.X - 15, pi.coords.Y + 15, pi.coords.Z + 10 }, p[4])
            && g_app.dH.WorldToScreen({ pi.coords.X - 15, pi.coords.Y + 15, pi.coords.Z - 50 }, p[5])
            && g_app.dH.WorldToScreen({ pi.coords.X - 15, pi.coords.Y - 15, pi.coords.Z + 10 }, p[6])
            && g_app.dH.WorldToScreen({ pi.coords.X - 15, pi.coords.Y - 15, pi.coords.Z - 50 }, p[7])))
        {
            return;
        }

        const float thickness = 1.5f;

        // Вертикальные рёбра
        data.AddLine(p[0].x, p[0].y, p[1].x, p[1].y, color, thickness);
        data.AddLine(p[2].x, p[2].y, p[3].x, p[3].y, color, thickness);
        data.AddLine(p[4].x, p[4].y, p[5].x, p[5].y, color, thickness);
        data.AddLine(p[6].x, p[6].y, p[7].x, p[7].y, color, thickness);

        // Верхняя грань
        data.AddLine(p[0].x, p[0].y, p[2].x, p[2].y, color, thickness);
        data.AddLine(p[0].x, p[0].y, p[4].x, p[4].y, color, thickness);
        data.AddLine(p[2].x, p[2].y, p[6].x, p[6].y, color, thickness);
        data.AddLine(p[4].x, p[4].y, p[6].x, p[6].y, color, thickness);

        // Нижняя грань
        data.AddLine(p[1].x, p[1].y, p[3].x, p[3].y, color, thickness);
        data.AddLine(p[1].x, p[1].y, p[5].x, p[5].y, color, thickness);
        data.AddLine(p[3].x, p[3].y, p[7].x, p[7].y, color, thickness);
        data.AddLine(p[5].x, p[5].y, p[7].x, p[7].y, color, thickness);

        if(cfg.showNames)
            if (g_app.dH.WorldToScreen({ pi.coords.X, pi.coords.Y, pi.coords.Z }, p[0]))
                data.AddText(p[0].x, p[0].y, OverlayColor(255, 150, 50, 200), 32.f, L"%S", pi.name);
    }

    void DrawAllESP(const PlayerView& pv) {
        for (int i = 0; i < 32; i++) {
            PlayerInfo pi = g_app.game.GetPlayerInfo(i);

            if (!pi.isDrawable)
                continue;
            if (pi.coords.X == 0 && pi.coords.Y == 0 && pi.coords.Z == 0)
                continue;

            DrawPlayerESP(pi, pv);
        }
    }

    void UpdateLogic() {

        HandleHotkeys();

        // Проверка кнопок меню
        cfg = Menu::Get();

        if (cfg.requestExit) {
            cfg.requestExit = false;
            g_app.running = false;          // выходим из программы
        }

        if (cfg.requestUnload) {
            cfg.requestUnload = false;
            // Например: выключаем всю логику чита, оставляя окно
            cfg.triggerbotEnabled = false;
            cfg.espEnabled = false;
            // Или: полностью останавливаем поток логики
        }

        if (!g_app.running) return;

        data.Clear();

        if (!cfg.requestUnload)
        {
            g_app.dH.cam.screenW = data.width;
            g_app.dH.cam.screenH = data.height;

            if (!g_app.game.UpdateGameData())
                return;

            if (cfg.triggerbotEnabled)
                UpdateTriggerbot();

            g_app.triggerEnabled = cfg.triggerbotEnabled;

            if (cfg.espEnabled)
            { 
                PlayerView pv = g_app.game.GetPV();
                g_app.dH.cam.position = { pv.X, pv.Y, pv.Z };
                g_app.dH.cam.yaw = pv.Xa;
                g_app.dH.cam.pitch = -pv.Ya;
                g_app.dH.UpdateCamData();

                DrawAllESP(pv);
            }
        }
    }

} // namespace


int main()
{
    if (!InitApp()) {
        ShutdownApp();
        return 1;
    }

    FrameTimer frameTimer;
    frameTimer.SetTargetFPS(100.0);

    //LARGE_INTEGER freq, lastRead;
    //QueryPerformanceFrequency(&freq);
    //QueryPerformanceCounter(&lastRead);
    //const double READ_INTERVAL = 1.0 / 60.0;

    while (g_app.running)
    {

        frameTimer.BeginFrame();

        if (!Overlay_PumpMessages())
            break;

        Overlay_BeginFrame();

        //LARGE_INTEGER now;
        //QueryPerformanceCounter(&now);
        //double elapsed = double(now.QuadPart - lastRead.QuadPart) / freq.QuadPart;

        //if (elapsed >= READ_INTERVAL) {
         //   lastRead = now;
            UpdateLogic();
        //}

        Overlay_EndFrame();
        frameTimer.EndFrame();
    }

    ShutdownApp();
    return 0;
}