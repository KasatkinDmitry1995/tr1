#include <windows.h>
#include <TlHelp32.h>
#include <iostream>
#include "Game.h"
#include "overlay.h"
#include "DrawHelper.h"

int main()
{

	Game game;

	while(true)
	{	
		
		_IN_CROSS_OBJECT prev_val = _IN_CROSS_OBJECT::INC_CROSS_CLEAR;
		bool enabled = true;


		std::cout << "Searching for proccess hl.exe...." << std::endl;

		if(!game.FindGameProccess())
		{
			std::cout << "Triggerbot closed by user" << std::endl;
			return 0;
		}

		std::cout << "HL process found successfully: " << std::endl;

		if(!game.OpenGameProcess())
		{
			std::cout << "OpenProcess failed: " << GetLastError() << std::endl;
			return 0;
		}

		HANDLE hOverlay = StartOverlayThread();
		if (!hOverlay) {
			MessageBoxW(nullptr, L"Не удалось запустить оверлей.",
				L"Ошибка", MB_ICONERROR);
			return 1;
		}

		OverlayData& data = GetOverlayData();

		while (true)
		{

			data.Clear();

			if (GetAsyncKeyState(VK_F3) & 0b1)
			{
				enabled = !enabled;
				std::cout << "Triggerbot is " << (enabled?"enabled":"disabled") << std::endl;
			}

			if (GetAsyncKeyState(VK_F4) & 0b1)
			{
				StopOverlayThread();
				game.CloseHandles();
				std::cout << "Triggerbot closed by user" << std::endl;
				return 0;
			}

			if (enabled && game.IsGameFocused())

				if (game.UpdateGameData())
				{

					data.Clear();

					if (prev_val != game.GetInCrossObject())
					{
						switch (game.GetInCrossObject())
						{
							case _IN_CROSS_OBJECT::INC_ENEMY:
								std::cout << "Enemy in the cross...." << std::endl;
								break;
							case _IN_CROSS_OBJECT::INC_FRIEND:
								std::cout << "Friend in the cross....." << std::endl;
								break;
							case _IN_CROSS_OBJECT::INC_HOSTAGE:
								std::cout << "Hostage in the cross..." << std::endl;
								break;
						}

						prev_val = game.GetInCrossObject();
					}

					if (game.GetInCrossObject() == _IN_CROSS_OBJECT::INC_ENEMY
						&& !game.IsUserInSpects())
					{
						int cx = 640, cy = 360; // примерные координаты центра
						data.AddCircle(cx, cy, 15, RGB(0, 200, 0), 1.0f);
						std::cout << "firing...." << std::endl;
						game.SendFire();
					}

					PlayerView pv = game.GetPV();

					data.AddFilledRect(130, 130, 350, 70, RGB(255, 230, 200));
					data.AddText(140, 135, RGB(0, 0, 255), L"X:%f  Y:%f  Z:%f", pv.X, pv.Y, pv.Z);
					data.AddText(140, 155, RGB(0, 0, 255), L"Xangle:%f   Yangle :%f", pv.Xa, pv.Ya);

					Camera cam;
					cam.position = { pv.X, pv.Y, pv.Z };
					cam.yaw = pv.Xa;  // в градусах
					cam.pitch = -pv.Ya;  // в градусах
					cam.fov = 90.0f;         // стандартный FOV для CS 1.6
					cam.screenW = 1280;
					cam.screenH = 720;

					// Точка в мире (например, позиция врага)
					Vec3 Pos1 = { 100.0f, 200.0f, 100.0f };
					Vec3 Pos2 = { 300.0f, 400.0f, 100.0f };

					// Проецируем
					Vec2 screenPos;
					if (WorldToScreen(Pos1, cam, screenPos)) {
						// Рисуем точку на экране
						data.AddFilledRect(screenPos.x - 3, screenPos.y - 3, 6, 6, RGB(255, 0, 0));
						data.AddText(screenPos.x + 5, screenPos.y - 5, RGB(255, 255, 0), L"Something on the map...");
					}

					if (WorldToScreen(Pos2, cam, screenPos)) {
						// Рисуем точку на экране
						data.AddFilledRect(screenPos.x - 10, screenPos.y - 10, 20, 20, RGB(0, 0, 255));
						data.AddText(screenPos.x + 5, screenPos.y - 5, RGB(255, 0, 0), L"Object...");
					}

				}
				else
					break;

			Sleep(15);
		}

	}
}