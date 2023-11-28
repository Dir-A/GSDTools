﻿#include <Windows.h>
#include <stdexcept>

#include "../../lib/RxHook/RxHook.h"
#include "../../lib/GSD/VFS.h"


static void StartHook(HMODULE hDllBase)
{
	size_t exe_base = (size_t)::GetModuleHandleW(nullptr);
	GSD::VFS::SetHook(exe_base + 0x19AB10, exe_base + 0x19AC40, exe_base + 0x08110);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		StartHook(hModule);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

extern "C" VOID __declspec(dllexport) Dir_A() {}