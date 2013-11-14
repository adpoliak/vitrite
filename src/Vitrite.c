/*
Copyright 2002 Ryan VanMiddlesworth

This file is part of Vitrite.

Vitrite is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Vitrite is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Vitrite; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
// Modifications by adpoliak to address issues with running pre-compiled binaries on Win7

// This app requires Windows 2000 or newer
#include "resource.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <strsafe.h>
#include "vitrite.h"

// Have to declare these for the hook DLL
__declspec(dllimport) int APIENTRY InstallHook();
__declspec(dllimport) int APIENTRY RemoveHook();

// Globals
HINSTANCE  g_hInstance;
HANDLE  ghMutex;

BOOL AddIconToSystemTray(HWND hWnd, UINT uID, LPTSTR lpszTip) {
	NOTIFYICONDATA  tnid;
	tnid.cbSize = sizeof(NOTIFYICONDATA);
	tnid.hWnd = hWnd;
	tnid.uID = uID;
	tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP; 
	tnid.uCallbackMessage = TRAY_CALLBACK;
	tnid.hIcon = (HICON) LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0);

	if (!lpszTip)
		tnid.szTip[0]=0;
	else if(StringCbCopy(tnid.szTip,sizeof(tnid.szTip),lpszTip)!=S_OK)
		return FALSE;
	return(Shell_NotifyIcon(NIM_ADD, &tnid)); 
}

BOOL RemoveIconFromSystemTray(HWND hWnd, UINT uID) { 
	NOTIFYICONDATA tnid; 

	tnid.cbSize = sizeof(NOTIFYICONDATA); 
	tnid.hWnd = hWnd; 
	tnid.uID = uID; 
	tnid.uFlags = 0; 
	tnid.uCallbackMessage = 0;
	tnid.hIcon = NULL;
	tnid.szTip[0] = '\0'; 

	return(Shell_NotifyIcon(NIM_DELETE, &tnid)); 
} 

BOOL ShowPopupMenu(HWND hWnd, POINT pOint) {
	HMENU  hPopup;

	//	Create popup menu on the fly. We'll use InsertMenu or AppendMenu 
	//  to add items to it.
	hPopup = CreatePopupMenu();
	if (!hPopup) {
		MessageBox(NULL, L"CreatePopupMenu failed", L"Error", MB_OK);
		return FALSE;
	}

	AppendMenu(hPopup, MF_STRING, IDM_MMAIN, L"&About");	
	AppendMenu(hPopup, MF_STRING, IDM_MEXIT, L"E&xit");

	// We have to do some MS voodoo to make the taskbar 
	// notification menus work right. It's not my fault - 
	// Microsoft is to blame on this one.
	// See MS Knowledgebase article Q135788 for more info.
	SetForegroundWindow(hWnd);		// [MS voodoo]
	if (!TrackPopupMenuEx(hPopup, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, pOint.x, pOint.y, hWnd, NULL)) {
		MessageBox(NULL, L"TrackPopupMenu failed", L"Error", MB_OK);
		return FALSE;
	}
	PostMessage(hWnd, WM_NULL, 0, 0);	// [More MS voodoo]

	return TRUE;
}

BOOL APIENTRY MainDlgProc(HWND hDlg, UINT Msg, UINT wParam, LONG lParam) {
	POINT  pMenuPoint;
	TCHAR execpath[1024];
	TCHAR execbuf[2048];
	DWORD bufLen;
	switch(Msg) {
	case WM_INITDIALOG :
		SetWindowPos(hDlg, HWND_TOP, -100, -100, 0, 0, SWP_NOSIZE);
		return(TRUE);

	case WM_DESTROY :
		RemoveIconFromSystemTray(hDlg, IDI_ICON1);
		ReleaseMutex(ghMutex);
		PostQuitMessage(0);
		return(TRUE);

	case TRAY_CALLBACK :
		switch(lParam) {
		case WM_RBUTTONDOWN :
			GetCursorPos(&pMenuPoint);
			ShowPopupMenu(hDlg, pMenuPoint);
			return(TRUE);
		}
		return(TRUE);

	case WM_COMMAND :
		switch(LOWORD(wParam)) {
		case IDM_MEXIT :
			PostQuitMessage(0);
			return(TRUE);

		case IDM_MMAIN :
			if(execpath != NULL){
				bufLen = GetModuleFileName(NULL,execpath,sizeof(execpath)/sizeof(TCHAR));
				if(GetLastError() != ERROR_INSUFFICIENT_BUFFER && execbuf != NULL){
					if(StringCbPrintf(execbuf,sizeof(execbuf),L"res://%s/%d",execpath,(int)MAKEINTRESOURCE(IDR_HTML1)) == S_OK){
						ShellExecute(NULL,L"open",execbuf,NULL,NULL,SW_SHOW);
					}
				}
				return(TRUE);
			}
		}
		return(TRUE);
	}	
	return(FALSE);
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {	
	HWND  hWnd;
	MSG  Msg;
	TCHAR  cTrayTip[128];
	OSVERSIONINFO  OSVInfo;

	// Make sure the OS supports transparency
#pragma warning(disable:4996)
	memset(&OSVInfo, 0, sizeof(OSVInfo));
	OSVInfo.dwOSVersionInfoSize = sizeof(OSVInfo);
	if (!GetVersionEx(&OSVInfo)) {
		MessageBox(NULL, L"Call to 'GetVersionEx()' failed", L"Critical Error", MB_OK | MB_ICONSTOP);
		return 1;
	} else {
		if (OSVInfo.dwMajorVersion < 5) {
			MessageBox(NULL, L"Sorry - this program requires Windows 2000 or higher.", L"Unfortunate Error", MB_OK | MB_ICONSTOP);
			return 1;
		}
	}
#pragma warning(default:4996)

	g_hInstance = hInstance;

	// Create/check for a mutex to determine if another copy of Vitrite is already running.
	if ((ghMutex = CreateMutex(NULL, FALSE, L"_vitrite_mutex")) == NULL) {
		MessageBox(NULL, L"Unable to create mutex", L"Critical Error", MB_OK | MB_ICONSTOP);
		return 1;
	} else if (GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(NULL, L"Another copy of Vitrite is already running.", L"Warning", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}

	memset(&Msg, 0, sizeof(Msg));
	// Build dummy dialog
	hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), (UINT)NULL, (DLGPROC)MainDlgProc);

	if(StringCbPrintf(cTrayTip,sizeof(cTrayTip),L"Vitrite - %s", VITRITE_VERSION)==S_OK)
		if (!AddIconToSystemTray(hWnd, IDI_ICON1,cTrayTip)) {
			MessageBox(NULL, L"Unable to create Notification Area icon.", L"Critical Error", MB_ICONSTOP | MB_OK);
			DestroyWindow(hWnd);
			return (int)Msg.wParam;
		}

	if (InstallHook() != 0)	{
		MessageBox(NULL, L"Unable to install keyboard hook.", L"Critical Error", MB_OK | MB_ICONSTOP);
		RemoveIconFromSystemTray(hWnd, IDI_ICON1);
		DestroyWindow(hWnd);
		return (int)Msg.wParam;
	}

	while (GetMessage(&Msg, NULL, 0, 0)) {
		IsDialogMessage(hWnd, &Msg);
	}
	if (RemoveHook() != 0) {
		MessageBox(NULL, L"Unable to remove keyboard hook.", L"Critical Error", MB_OK | MB_ICONSTOP);
	}
	DestroyWindow(hWnd);

	return (int)Msg.wParam;
}
