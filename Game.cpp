#include "Game.h"
#include <iostream>
#include <unordered_set>

bool wasClickSend = false;
std::unordered_set<std::string> t_models = { "terror", "leet", "arctic", "guerilla" };
std::unordered_set<std::string> ct_models = { "urban", "gsg9", "sas", "gign" };

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
		else {
			offsets.InCross = client_base_addr + 0x1211f4;
			offsets.UserInSpect = client_base_addr + 0x12B394;
			offsets.userPV = client_base_addr + 0x11D470;
			offsets.playerName = 0x104;
			offsets.playerModel = 0x130;
			offsets.playerAlive = 0x17C;
			offsets.playerCoords = 0x188;
			offsets.playerStructSize = 0x250;
			offsets.playersArray = hw_base_addr + 0x9E40A;
			return true;
		}
	}

}

bool Game::OpenGameProcess()
{
	hlprc = OpenProcess(PROCESS_VM_READ, FALSE, hl_pid);

	if (!hlprc) 
		return false;

	return true;
}

bool Game::UpdateGameData()
{
	if (!ReadProcessMemory(hlprc, (LPCVOID)offsets.InCross, &in_cross, sizeof(in_cross), &io))
		return false;

	if (!ReadProcessMemory(hlprc, (LPCVOID)offsets.UserInSpect, &is_user_in_spect, sizeof(is_user_in_spect), &io))
		return false;

	if (!ReadProcessMemory(hlprc, (LPCVOID)offsets.userPV, &pv, sizeof(pv), &io))
		return false;

	unsigned int ptr = 0;

	if (!ReadProcessMemory(hlprc, (LPCVOID)offsets.playersArray, &ptr, sizeof(unsigned int), &io))
		return false;

	byte pState;

	for (int i = 0; i < 32; i++)
	{

		unsigned int base = ptr + i * offsets.playerStructSize;

		playersInfo[i].lastCoords = playersInfo[i].coords;

		if (!ReadProcessMemory(hlprc, (LPCVOID)(base + offsets.playerCoords), &(playersInfo[i].coords), sizeof(Vec3), &io))
			return false;

		unsigned char isAlive;
		bool updated = true;

		if (!ReadProcessMemory(hlprc, (LPCVOID)(base + offsets.playerAlive), &isAlive, sizeof(unsigned char), &io))
			return false;

		if (playersInfo[i].coords != playersInfo[i].lastCoords)
			playersInfo[i].lastTimePosChanged = GetTickCount64();
		else
			if (GetTickCount64()  - playersInfo[i].lastTimePosChanged > 5000)
				updated = false;
			
		playersInfo[i].isDrawable = updated && isAlive != 0;

		char buf[16];

		if (!ReadProcessMemory(hlprc, (LPCVOID)(base + offsets.playerModel), &buf, sizeof(buf), &io))
			return false;

		buf[15] = 0;
		std::string playerModel_s = std::string(buf);

		playersInfo[i].isT = t_models.count(playerModel_s);

		if (!ReadProcessMemory(hlprc, (LPCVOID)(base + offsets.playerName), &playersInfo[i].name, sizeof(playersInfo[i].name), &io))
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

PlayerInfo Game::GetPlayerInfo(int i)
{
	return playersInfo[i];
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