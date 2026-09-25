#pragma once
#include <windows.h>
#include <TlHelp32.h>

namespace ProcUtils
{
	bool IsGameFocused(DWORD hlPid);
	unsigned int FindClientBase(_Out_ unsigned int& hl_pid, LPCWSTR procName, LPCWSTR moduleName);

}