#include "proc_utils.h"

namespace ProcUtils
{

	bool IsMainWindowFocused(DWORD pid) {
		HWND fg = GetForegroundWindow();
		if (!fg) return false;

		DWORD fgPid = 0;
		GetWindowThreadProcessId(fg, &fgPid);
		return fgPid == pid;
	}

	unsigned int FindProccess(LPCWSTR procName)
	{
		HANDLE tlh_list;
		unsigned int pid{ 0 }, cl_base{ 0 };
		tlh_list = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, -1);

		if (INVALID_HANDLE_VALUE == tlh_list) return 0;

		PROCESSENTRY32W pe{ sizeof PROCESSENTRY32W };


		if (Process32First(tlh_list, &pe)) {
			do {
				if (lstrcmpiW(procName, pe.szExeFile) == 0) {
					pid = pe.th32ProcessID;
					break;
				}

			} while (Process32Next(tlh_list, &pe));
		}
		CloseHandle(tlh_list);

		return pid;
	}

	unsigned int FindModule(unsigned int pid, LPCWSTR moduleName)
	{

		if (pid == 0)
			return 0;

		HANDLE tlh_list = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);

		if (INVALID_HANDLE_VALUE == tlh_list)
			return 0;

		MODULEENTRY32W me{ sizeof MODULEENTRY32W };

		unsigned int moduleBase = 0;

		Module32First(tlh_list, &me);
		do
		{
			if (lstrcmpiW(moduleName, me.szModule) == 0)
			{
				moduleBase = reinterpret_cast<unsigned int>(me.modBaseAddr);
				break;
			}

		} while (Module32Next(tlh_list, &me));

		CloseHandle(tlh_list);

		return moduleBase;
	}
}