// back_to_the_desktop.cpp : Defines the entry point for the console application.
//

#include <WinSDKVer.h>

#define _WIN32_WINNT _WIN32_WINNT_WINXP
#define PSAPI_VERSION 1

#include <SDKDDKVer.h>

#include <stdio.h>
#include <tchar.h>

#include <windows.h>
#include <psapi.h>
#include <shlwapi.h>
#include <conio.h>

#include <set>
using namespace std;



HDC hdcDesktop;

set<DWORD> ignoreProcess;

BOOL CALLBACK EnumWindowsProc(
	__in  HWND hwnd,
	__in  LPARAM lParam
	) {
	
	TCHAR title[1024];
	ZeroMemory(title, sizeof(title));

	GetWindowText(hwnd, title, sizeof(title) / sizeof(TCHAR));
	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);

	if (ignoreProcess.find(pid) != ignoreProcess.end())
		return true;

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION  , false, pid);
	TCHAR processname[1024];
	ZeroMemory(processname, sizeof(processname));
	TCHAR* exename = processname;
	
	if ( hProcess != NULL) {
		HRESULT x = GetProcessImageFileName(hProcess, processname, sizeof(processname) / sizeof(TCHAR));
	
		exename = PathFindFileName(processname);
		CloseHandle(hProcess);
	} else {
		_tcscpy_s(processname, sizeof(processname) / sizeof(TCHAR), _T("unknown"));
	}

	WINDOWPLACEMENT windowState;

	GetWindowPlacement(hwnd, &windowState);

	WINDOWINFO windowInfo;
	GetWindowInfo(hwnd, &windowInfo);

	RECT* position = &(windowState.rcNormalPosition);
	
	if (
		!RectVisible(hdcDesktop, position) &&
		position->left != position->right &&
		position->bottom != position->top &&
		((windowInfo.dwExStyle & (WS_EX_NOACTIVATE | WS_EX_APPWINDOW) == WS_EX_NOACTIVATE) ||
		windowInfo.dwExStyle & WS_EX_TOOLWINDOW)
		
		) {
		_tprintf(_T("\n%d %s \"%s\" at (%d,%d) size (%d,%d)\n"), pid, exename, title
			, position->left, position->top
			, position->right - position->left
			, position->bottom - position->top);

		_tprintf(_T("(S)kip/(F)ix/Skip (P)rocess"));
		int input = _getch();
		switch(tolower(input)) {
		case 's':
			break;
		case 'f':
			MoveWindow(hwnd, 100,100, position->right - position->left, position->bottom - position->top, true);
			break;
		case 'p':
			ignoreProcess.insert(pid);
			break;
		}

	}
	
	return true;
}


int _tmain(int argc, _TCHAR* argv[])
{

	HWND hwndDesktop = GetDesktopWindow();
	hdcDesktop = GetDC(hwndDesktop);
	
	EnumWindows(EnumWindowsProc, NULL);

	ReleaseDC(hwndDesktop, hdcDesktop);
	_tprintf(_T("\nAll done!"));
	_getch();
	return 0;
}

