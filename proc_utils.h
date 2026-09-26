#pragma once
#include <windows.h>
#include <TlHelp32.h>

namespace ProcUtils
{
	bool IsMainWindowFocused(DWORD pid);
	unsigned int FindProccess(LPCWSTR procName);
	unsigned int FindModule(unsigned int pid, LPCWSTR moduleName);

}