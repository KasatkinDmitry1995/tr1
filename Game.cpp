#include "Game.h"
#include <iostream>

bool wasClickSend = false;

bool Game::FindGameProccess()
{

	while (true)
	{
		hl_pid = ProcUtils::FindProccess(L"hl.exe");

		if (hl_pid != 0)
			client_base_addr = ProcUtils::FindModule(hl_pid, L"client.dll");

		if (client_base_addr != 0)
			 hw_base_addr = ProcUtils::FindModule(hl_pid, L"hw.dll");

		if (!hl_pid || !client_base_addr || !hw_base_addr)
		{
			Sleep(200);

			if (GetAsyncKeyState(VK_F4) & 0b1)
				return false;
		}
		else
			return true;
	}

}

bool Game::OpenGameProcess()
{
	hlprc = OpenProcess(PROCESS_VM_READ, FALSE, hl_pid);

	if (!hlprc) 
		return false;

	in_cross_addr = client_base_addr + 0x1211f4;
	is_user_in_spect_addr = client_base_addr + 0x12B394;
	userPV_addr = client_base_addr + 0x11D470;

	return true;
}

bool Game::UpdateGameData()
{
	if (!ReadProcessMemory(hlprc, (const void*)in_cross_addr, &in_cross, sizeof(in_cross), &io))
		return false;

	if (!ReadProcessMemory(hlprc, (const void*)is_user_in_spect_addr, &is_user_in_spect, sizeof(is_user_in_spect), &io))
		return false;

	if (!ReadProcessMemory(hlprc, (const void*)userPV_addr, &pv, sizeof(pv), &io))
		return false;

	unsigned int ptr = 0;

	if (!ReadProcessMemory(hlprc, (const void*)(hw_base_addr + 0x9E40A), &ptr, sizeof(unsigned int), &io))
		return false;

	for (int i = 0; i < 32; i++)
	{
		if (!ReadProcessMemory(hlprc, (const void*)(ptr + i * 0x250 + 0x188), &playersCoords[i], sizeof(Vec3), &io))
			return false;
	}

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

bool Game::SendFire(int ms)
{
	if (wasClickSend)
		return false;

	wasClickSend = true;

	INPUT down = {}; down.type = INPUT_MOUSE;
	down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &down, sizeof(INPUT));

	HANDLE t = nullptr;
	CreateTimerQueueTimer(&t, nullptr,
		[](PVOID p, BOOLEAN) {
			INPUT up = {}; up.type = INPUT_MOUSE;
			up.mi.dwFlags = MOUSEEVENTF_LEFTUP;
			SendInput(1, &up, sizeof(INPUT));
			auto* wasClickSend = static_cast<bool*>(p);
			*wasClickSend = false;
			
		}, &wasClickSend, ms, 0, WT_EXECUTEONLYONCE);

	return true;

}

PlayerView Game::GetPV()
{
	return pv;
}

Vec3 Game::GetPlayerCoords(int i)
{
	return playersCoords[i];
}

bool Game::IsGameFocused()
{
	return ProcUtils::IsMainWindowFocused(hl_pid);
}

void Game::CloseHandles()
{
	if (hlprc)
		CloseHandle(hlprc);
}