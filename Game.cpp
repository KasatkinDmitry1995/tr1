#include "Game.h"
#include <iostream>

bool Game::FindGameProccess()
{
	while (!(base_addr = ProcUtils::FindClientBase(hl_pid, L"hl.exe", L"client.dll")))
	{
		Sleep(200);
				
		if (GetAsyncKeyState(VK_F4) & 0b1)
			return false;
	}

	return true;
}

bool Game::OpenGameProcess()
{
	hlprc = OpenProcess(PROCESS_VM_READ, FALSE, hl_pid);

	if (!hlprc) 
		return false;

	in_cross_addr = base_addr + 0x1211f4;
	is_user_in_spect_addr = base_addr + 0x12B394;

	return true;
}

bool Game::UpdateGameData()
{
	if (!ReadProcessMemory(hlprc, (const void*)in_cross_addr, &in_cross, sizeof(in_cross), &io))
		return false;

	if (!ReadProcessMemory(hlprc, (const void*)is_user_in_spect_addr, &is_user_in_spect, sizeof(is_user_in_spect), &io))
		return false;

	return true;
}

_IN_CROSS_OBJECT Game::GetInCrossObject()
{
	return in_cross;
}

bool Game::IsUserInSpects()
{
	return is_user_in_spect;
}

void Game::SendFire()
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

bool Game::IsGameFocused()
{
	return ProcUtils::IsGameFocused(hl_pid);
}

void Game::CloseHandles()
{
	if (hlprc)
		CloseHandle(hlprc);
}