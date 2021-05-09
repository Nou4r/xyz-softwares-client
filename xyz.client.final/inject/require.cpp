#include "mmi.h"
#pragma warning(disable : 4996)

#define		WIN_NT_3_1		"Windows NT 3.1 (1993)"
#define		WIN_NT_3_5		"Windows NT 3.5 (1994)"
#define		WIN_NT_3_51		"Windows NT 3.51 (1995)"
#define		WIN_NT_4_0		"Windows NT 4.0 (1996)"
#define		WIN_NT_5_0		"Windows NT 5.0 (Windows 2000) (1997-1999)"
#define		WIN_NT_5_1		"Windows NT 5.1 (Windows XP) (2001)"
#define		WIN_NT_5_2		"Windows NT 5.2 (Windows Server 2003, Windows XP x64) (2003)"
#define		WIN_NT_6_0		"Windows NT 6.0 (Windows Vista, Windows Server 2008) (2006)"
#define		WIN_NT_6_1		"Windows NT 6.1 (Windows 7, Windows Server 2008 R2) (2009)"
#define		WIN_NT_6_2		"Windows NT 6.2 (Windows 8, Windows Server 2012) (2012)"
#define		WIN_NT_6_3		"Windows NT 6.3 (Windows 8.1, Windows Server 2012 R2) (2013)"
#define		WIN_NT_10		"Windows NT 10.0 (Windows 10, Windows Server 2016) (2015)"

bool SetPrivilege(HANDLE hToken, LPCTSTR Privilege, BOOL bEnablePrivilege) {

	TOKEN_PRIVILEGES tp;
	LUID luid;
	TOKEN_PRIVILEGES tpPrevious;
	DWORD cbPrevious = sizeof(TOKEN_PRIVILEGES);

	if (!LookupPrivilegeValue(NULL, Privilege, &luid)) return false;

	// First pass. get current privilege settings
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = 0;

	AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tp,
		sizeof(TOKEN_PRIVILEGES),
		&tpPrevious,
		&cbPrevious);

	if (GetLastError() != ERROR_SUCCESS) return false;

	// second pass. set privileges based on previous settings

	tpPrevious.PrivilegeCount = 1;
	tpPrevious.Privileges[0].Luid = luid;

	if (bEnablePrivilege) {
		tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
	}
	else {
		tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED & tpPrevious.Privileges[0].Attributes);
	}

	AdjustTokenPrivileges(
		hToken,
		FALSE,
		&tpPrevious,
		cbPrevious,
		NULL,
		NULL
	);


	if (GetLastError() != ERROR_SUCCESS) return false;

	return true;
}


void DisplayError(LPCSTR szAPI) {

	LPTSTR MessageBuffer;
	DWORD dwBufferLength;

	fprintf(stderr, "%s() error!\n", szAPI);

	if (dwBufferLength = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		GetSystemDefaultLangID(),
		(LPTSTR)&MessageBuffer,
		0,
		NULL
	)) {

		DWORD dwBytesWritten;

		// Output message string on stterr
		WriteFile(
			GetStdHandle(STD_ERROR_HANDLE),
			MessageBuffer,
			dwBufferLength,
			&dwBytesWritten,
			NULL
		);

		// Free the buffer alllocated by the system
		LocalFree(MessageBuffer);
	}

}

int EscalatePrivilege() {

	HANDLE hToken;
	int dwRetVal = RTN_OK;

	if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hToken)) {

		if (GetLastError() == ERROR_NO_TOKEN) {
			if (!ImpersonateSelf(SecurityImpersonation)) return RTN_ERROR;

			if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &hToken)) {
				DisplayError("OpenThreadToken");
				return RTN_ERROR;
			}

		}
		else {
			return RTN_ERROR;
		}
	}

	// Enable SetPrivilege()

	if (!SetPrivilege(hToken, SE_DEBUG_NAME, TRUE)) {

		DisplayError("Set Privilege");

		//close token handle
		CloseHandle(hToken);

		//indicate failure
		return RTN_ERROR;
	}

	return dwRetVal;
}

bool GetOSInfo() {

	/*
	* Windows NT 3.1 (1993)
	* Windows NT 3.5 (1994)
	* Windows NT 3.51 (1995)
	* Windows NT 4.0 (1996)
	* Windows NT 5.0 (Windows 2000) (1997-1999)
	* Windows NT 5.1 (Windows XP) (2001)
	* Windows NT 5.2 (Windows Server 2003, Windows XP x64) (2003)
	* Windows NT 6.0 (Windows Vista, Windows Server 2008) (2006)
	* Windows NT 6.1 (Windows 7, Windows Server 2008 R2) (2009)
	* Windows NT 6.2 (Windows 8, Windows Server 2012) (2012)
	* Windows NT 6.3 (Windows 8.1, Windows Server 2012 R2) (2013)
	* Windows NT 10.0 (Windows 10, Windows Server 2016) (2015)
	*/

	bool is64bit;

	is64bit = (sizeof(void*) != 4);

	OSVERSIONINFO osVersion;

	ZeroMemory(&osVersion, sizeof(OSVERSIONINFO));
	osVersion.dwOSVersionInfoSize = sizeof(osVersion);

	if (GetVersionEx(&osVersion) && osVersion.dwPlatformId == 2) {

		switch (osVersion.dwMajorVersion) {

		case 3:
			switch (osVersion.dwMinorVersion) {

			case 1:
				return true;
			case 5:
				return true;
			case 51:
				return true;
			}
			break;


		case 4:
			switch (osVersion.dwMinorVersion) {
			case 0:
				return true;
			}
			break;

		case 5:
			switch (osVersion.dwMinorVersion) {

			case 0:
				return true;
			case 1:
				return true;
			case 2:
				return true;
			}
			break;

		case 6:
			switch (osVersion.dwMinorVersion) {

			case 0:
				return true;
			case 1:
				return true;
			case 2:
				return true;
			case 3:
				return true;
			}
			break;

		case 10:
			switch (osVersion.dwMinorVersion) {
			case 0:
				return true;
			}
			break;
		}
	}

	return false;
}