#ifndef VITRITE_H
#define VITRITE_H
// Modifications by adpoliak to address issues with running pre-compiled binaries on Win7

// Prototypes
BOOL AddIconToSystemTray(HWND hWnd, UINT uID, LPTSTR lpszTip); 
BOOL RemoveIconFromSystemTray(HWND hWnd, UINT uID);
BOOL ShowPopupMenu(HWND hWnd, POINT pOint);
BOOL APIENTRY MainDlgProc(HWND hDlg, UINT Msg, UINT wParam, LONG lParam);
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow);

// Constants
#define TRAY_CALLBACK	10001

#define IDM_MEXIT		500
#define IDM_MMAIN		501

#define VITRITE_VERSION	L"1.1.1-adpoliak"


#endif	// VITRITE_H
