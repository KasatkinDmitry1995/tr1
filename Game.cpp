#include "Game.h"
#include <iostream>

class Game
{
	private:
		unsigned int in_cross_addr = 0;
		unsigned int hl_pid = 0;
		unsigned int base_addr = 0;
		unsigned int is_user_in_spect_addr = 0;
		HANDLE hlprc;
		_IN_CROSS_OBJECT in_cross;
		unsigned char is_user_in_spect = 0;
		SIZE_T io;

	public:
		bool FindGameProccess()
		{
			while (!(base_addr = ProcUtils::FindClientBase(hl_pid, L"hl.exe", L"client.dll")))
			{
				Sleep(200);
				
				if (GetAsyncKeyState(VK_F4) & 0b1)
					return false;
			}

			return true;
		}

		bool OpenGameProcess()
		{
			hlprc = OpenProcess(PROCESS_VM_READ, FALSE, hl_pid);

			if (!hlprc) 
				return false;

			in_cross_addr = base_addr + 0x1211f4;
			is_user_in_spect_addr = base_addr + 0x12B394;

			return true;
		}

		bool UpdateGameData()
		{
			if (!ReadProcessMemory(hlprc, (const void*)in_cross_addr, &in_cross, sizeof(in_cross), &io))
				return false;

			if (!ReadProcessMemory(hlprc, (const void*)is_user_in_spect_addr, &is_user_in_spect, sizeof(is_user_in_spect), &io))
				return false;

			return true;
		}

		_IN_CROSS_OBJECT GetInCrossObject()
		{
			return in_cross;
		}

		bool IsUserInSpects()
		{
			return is_user_in_spect;
		}

		void SendFire()
		{
			INPUT down = {};
			down.type = INPUT_MOUSE;
			down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

			INPUT up = {};
			up.type = INPUT_MOUSE;
			up.mi.dwFlags = MOUSEEVENTF_LEFTUP;

			SendInput(1, &down, sizeof(INPUT));
			Sleep(50);
			SendInput(1, &up, sizeof(INPUT));
			Sleep(25);
		}

		bool IsGameFocused()
		{
			return ProcUtils::IsGameFocused(hl_pid);
		}

		void CloseHandles()
		{
			if (hlprc)
				CloseHandle(hlprc);
		}
};