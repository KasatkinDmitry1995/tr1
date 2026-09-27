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
		
		DrawHelper dH;
		dH.cam.fov = 90.0f;         // стандартный FOV для CS 1.6
		dH.cam.screenW = 1760;
		dH.cam.screenH = 990;

		Vec3 coords;
		Vec2 screenPos;

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
					
						if(game.SendFire(150))
							std::cout << "firing...." << std::endl;
					}

					PlayerView pv = game.GetPV();

					dH.cam.position = { pv.X, pv.Y, pv.Z };
					dH.cam.yaw = pv.Xa;  // в градусах
					dH.cam.pitch = -pv.Ya;  // в градусах
					dH.UpdateCamData();

					for (int i = 0; i < 32; i++)
					{
						coords = game.GetPlayerCoords(i);

						if (coords.x == 0 && coords.y == 0 && coords.z == 0)
							continue;

						if (dH.WorldToScreen(coords, screenPos)) {
							data.AddFilledRect(screenPos.x - 3, screenPos.y - 3, 6, 6, RGB(255, 0, 0));
							data.AddText(screenPos.x + 5, screenPos.y - 5, RGB(255, 255, 0), L"Player....");
						}
					}

				}
				else
					break;

			Sleep(5);
		}

	}
}