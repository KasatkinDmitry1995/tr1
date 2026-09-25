#include <windows.h>
#include <TlHelp32.h>
#include <iostream>
#include "Game.h"

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

		while (true)
		{
			if (GetAsyncKeyState(VK_F3) & 0b1)
			{
				enabled = !enabled;
				std::cout << "Triggerbot is " << (enabled?"enabled":"disabled") << std::endl;
			}

			if (GetAsyncKeyState(VK_F4) & 0b1)
			{
				game.CloseHandles();
				std::cout << "Triggerbot closed by user" << std::endl;
				return 0;
			}

			if (enabled && game.IsGameFocused())

				if (game.UpdateGameData())
				{
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
						std::cout << "firing...." << std::endl;
						game.SendFire();
					}
				}
				else
					break;

			Sleep(15);
		}

	}
}