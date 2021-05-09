#include "mmi.h"
#include "..\menus.h"

bool CreateRemoteThread_Type1(LPCSTR DllPath, HANDLE hProcess) {

	LPVOID LoadLibAddr = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

	if (!LoadLibAddr) {
		return false;
	}

	LPVOID pDllPath = VirtualAllocEx(hProcess, 0, strlen(DllPath), MEM_COMMIT, PAGE_READWRITE);

	if (!pDllPath) {
		return false;
	}

	BOOL Written = WriteProcessMemory(hProcess, pDllPath, (LPVOID)DllPath, strlen(DllPath), NULL);

	if (!Written) {
		return false;
	}

	HANDLE hThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddr, pDllPath, 0, NULL);

	if (!hThread) {
		return false;
	}

	WaitForSingleObject(hThread, INFINITE);

	if (VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE)) {

	}

	CloseHandle(hThread);

	m_Menu->loadingType = _xor_("Injection success!");
	Sleep(1000);
	m_Menu->loadingType = _xor_("Closing...");
	Sleep(1000);
	m_Menu->AppOpen = false;

	return true;
}