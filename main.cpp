#include <windows.h>
#include <TlHelp32.h>
#include <iostream>

enum class _IN_CROSS_OBJECT : unsigned char
{
	INC_CROSS_CLEAR,
	INC_FRIEND,
	INC_ENEMY,
	INC_HOSTAGE
};


bool IsGameFocused(DWORD hlPid) {
	HWND fg = GetForegroundWindow();
	if (!fg) return false;

	DWORD fgPid = 0;
	GetWindowThreadProcessId(fg, &fgPid);
	return fgPid == hlPid;
}

unsigned int FindClientBase(_Out_ unsigned int& hl_pid)
{
	HANDLE tlh_list;
	unsigned int pid{ 0 }, cl_base{ 0 };
	tlh_list = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, -1);

	if (INVALID_HANDLE_VALUE == tlh_list) return 0;

	PROCESSENTRY32W pe{ sizeof PROCESSENTRY32W };

	
	if (Process32First(tlh_list, &pe)) {
		do {
			if (lstrcmpiW(L"hl.exe", pe.szExeFile) == 0) {
				pid = pe.th32ProcessID;
				break;
			}

		} while (Process32Next(tlh_list, &pe));
	}
	CloseHandle(tlh_list);

	hl_pid = pid;

	if (pid == 0)
		return 0;

	tlh_list = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);

	if (INVALID_HANDLE_VALUE == tlh_list) 
		return 0;

	MODULEENTRY32W me{ sizeof MODULEENTRY32W };

	Module32First(tlh_list, &me);
	do
	{
		if (lstrcmpiW(L"client.dll", me.szModule) == 0)
		{
			cl_base = reinterpret_cast<unsigned int>(me.modBaseAddr);
			break;
		}

	} while (Module32Next(tlh_list, &me));

	CloseHandle(tlh_list);

	return cl_base;
}

int main()
{
	while(true)
	{	
		unsigned int in_cross_addr{ 0 }, hl_pid{ 0 }, base_addr{ 0 }, is_user_in_spect_addr{ 0 };
		_IN_CROSS_OBJECT in_cross, prev_val = _IN_CROSS_OBJECT::INC_CROSS_CLEAR;
		unsigned char is_user_in_spect = 0;
		SIZE_T io;
		bool enabled = true;

		INPUT down = {};
		down.type = INPUT_MOUSE;
		down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

		INPUT up = {};
		up.type = INPUT_MOUSE;
		up.mi.dwFlags = MOUSEEVENTF_LEFTUP;


		while (!(base_addr = FindClientBase(hl_pid))) {
			Sleep(1000);
			std::cout << "Searching for proccess hl.exe...." << std::endl;

			if (GetAsyncKeyState(VK_F4) & 0b1)
			{
				std::cout << "Triggerbot closed by user" << std::endl;
				return 0;
			}

		}
		in_cross_addr = base_addr + 0x1211f4; 
		is_user_in_spect_addr = base_addr + 0x12B394;

		HANDLE hlprc = OpenProcess(PROCESS_VM_READ, FALSE, hl_pid);
		if (!hlprc) {
			std::cout << "OpenProcess failed: " << GetLastError() << std::endl;
			continue;
		}

		std::cout << "HL process found successfully: " << std::endl;

		while (true)
		{
			if (GetAsyncKeyState(VK_F3) & 0b1)
				enabled = !enabled;

			if (GetAsyncKeyState(VK_F4) & 0b1)
			{
				CloseHandle(hlprc);
				std::cout << "Triggerbot closed by user" << std::endl;
				return 0;
			}


			if (enabled && IsGameFocused(hl_pid))
				if (ReadProcessMemory(hlprc, (const void*)in_cross_addr, &in_cross, sizeof(in_cross), &io))
				{
					switch (in_cross) {
					case _IN_CROSS_OBJECT::INC_ENEMY:
						if (in_cross != prev_val)
							std::cout << "Enemy in the cross...." << std::endl;

						if (ReadProcessMemory(hlprc, (const void*)is_user_in_spect_addr, &is_user_in_spect, sizeof(is_user_in_spect), &io))
						{
							if (is_user_in_spect == 0)
							{
								std::cout << "firing...." << std::endl;
								SendInput(1, &down, sizeof(INPUT));
								Sleep(50);
								SendInput(1, &up, sizeof(INPUT));
								Sleep(25);
							}
						}
						break;

					case _IN_CROSS_OBJECT::INC_FRIEND:
						if (in_cross != prev_val)
							std::cout << "Friend in the cross....." << std::endl;
						break;

					case _IN_CROSS_OBJECT::INC_HOSTAGE:
						if (in_cross != prev_val)
							std::cout << "Hostage in the cross..." << std::endl;
						break;

					}

					if(in_cross != prev_val)
						prev_val = in_cross;

				}
				else
					if (!FindClientBase(hl_pid) || !hl_pid)
						break;

			Sleep(15);
		}

	}
}