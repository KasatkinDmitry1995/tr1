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
		DrawHelper dH;
		dH.cam.fov = 90.0f;         // стандартный FOV для CS 1.6
		
		while (true)
		{
			data.Clear();

			dH.cam.screenW = data.width;
			dH.cam.screenH = data.height;

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

			

			if (game.UpdateGameData())
			{
				if (enabled && game.IsGameFocused())
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
							if (game.SendFire(150))
								std::cout << "firing...." << std::endl;

				}

				PlayerView pv = game.GetPV();

				dH.cam.position = { pv.X, pv.Y, pv.Z };
				dH.cam.yaw = pv.Xa;  // в градусах
				dH.cam.pitch = -pv.Ya;  // в градусах
				dH.UpdateCamData();

				PlayerInfo pi;

				for (int i = 0; i < 32; i++)
				{
					pi = game.GetPlayerInfo(i);

					if (!pi.isDrawable || (pi.coords.X == 0 && pi.coords.Y == 0 && pi.coords.Z == 0))
						continue;

					float dx = pi.coords.X - pv.X;
					float dy = pi.coords.Y - pv.Y;
					float dz = pi.coords.Z - pv.Z;

					float thickness = fmax(1.f, fmin(5.f, 1000/sqrtf(dx * dx + dy * dy + dz * dz)));
					COLORREF color = pi.isT ? RGB(255, 50, 50): RGB(50, 50, 255);

					

					Vec2 p[8];

					if (dH.WorldToScreen({ pi.coords.X + 15, pi.coords.Y + 15, pi.coords.Z + 10 }, p[0])
						&& dH.WorldToScreen({ pi.coords.X + 15, pi.coords.Y + 15, pi.coords.Z - 50 }, p[1])
						&& dH.WorldToScreen({ pi.coords.X + 15, pi.coords.Y - 15, pi.coords.Z + 10 }, p[2])
						&& dH.WorldToScreen({ pi.coords.X + 15, pi.coords.Y - 15, pi.coords.Z - 50 }, p[3])
						&& dH.WorldToScreen({ pi.coords.X - 15, pi.coords.Y + 15, pi.coords.Z + 10 }, p[4])
						&& dH.WorldToScreen({ pi.coords.X - 15, pi.coords.Y + 15, pi.coords.Z - 50 }, p[5])
						&& dH.WorldToScreen({ pi.coords.X - 15, pi.coords.Y - 15, pi.coords.Z + 10 }, p[6])
						&& dH.WorldToScreen({ pi.coords.X - 15, pi.coords.Y - 15, pi.coords.Z - 50 }, p[7]))
					{
						data.AddLine(p[0].x, p[0].y, p[1].x, p[1].y, color, thickness);
						data.AddLine(p[2].x, p[2].y, p[3].x, p[3].y, color, thickness);
						data.AddLine(p[4].x, p[4].y, p[5].x, p[5].y, color, thickness);
						data.AddLine(p[6].x, p[6].y, p[7].x, p[7].y, color, thickness);
						data.AddLine(p[0].x, p[0].y, p[2].x, p[2].y, color, thickness);
						data.AddLine(p[0].x, p[0].y, p[4].x, p[4].y, color, thickness);
						data.AddLine(p[2].x, p[2].y, p[6].x, p[6].y, color, thickness);
						data.AddLine(p[4].x, p[4].y, p[6].x, p[6].y, color, thickness);
						data.AddLine(p[1].x, p[1].y, p[3].x, p[3].y, color, thickness);
						data.AddLine(p[1].x, p[1].y, p[5].x, p[5].y, color, thickness);
						data.AddLine(p[3].x, p[3].y, p[7].x, p[7].y, color, thickness);
						data.AddLine(p[5].x, p[5].y, p[7].x, p[7].y, color, thickness);
					}

				}

			}
				else
					break;

			Sleep(5);
		}

	}
}