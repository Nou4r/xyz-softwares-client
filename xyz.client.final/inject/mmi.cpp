#include "mmi.h"

HANDLE hToken;
int dwRetVal = RTN_OK;

int Inject(std::string dllPath, std::string targetProcess) {

	if (!GetOSInfo()) {
		return 1;
	}

	int epResult = EscalatePrivilege();

	targetProcess = "notepad.exe";

	PROCESSENTRY32 PE32{ sizeof(PROCESSENTRY32) };
	PE32.dwSize = sizeof(PE32);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE) {
		return 0;
	}

	DWORD PID = 0;
	BOOL bRet = Process32First(hSnap, &PE32);
	char yn[3];

	while (bRet) {

		//printf("process: %s\n", PE32.szExeFile);
		if (!strcmp((LPCSTR)targetProcess.c_str(), PE32.szExeFile)) {
			PID = PE32.th32ProcessID;
		}
		bRet = Process32Next(hSnap, &PE32);
	}

	CloseHandle(hSnap);

	HANDLE hProcess = OpenProcess(
		PROCESS_QUERY_INFORMATION |
		PROCESS_CREATE_THREAD |
		PROCESS_VM_OPERATION |
		PROCESS_VM_WRITE,
		FALSE, PID);

	if (!hProcess) {
		return false;
	}

	SetPrivilege(hToken, SE_DEBUG_NAME, FALSE);
	CloseHandle(hToken);
	CreateRemoteThread_Type1(dllPath.c_str(), hProcess);
	CloseHandle(hProcess);

	if (!TerminateProcess(hProcess, 0xffffffff))
	{
		DisplayError("TerminateProcess");
		dwRetVal = RTN_ERROR;
	}

	return 0;
}