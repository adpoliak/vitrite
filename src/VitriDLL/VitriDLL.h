#ifndef VITRIDLL_H
#define VITRIDLL_H
// Modifications by adpoliak to address issues with running pre-compiled binaries on Win7

// Prototypes
LRESULT CALLBACK KbHookProc(int nCode, WPARAM wParam, LPARAM lParam);
__declspec(dllexport) int APIENTRY InstallHook();
__declspec(dllexport) int APIENTRY RemoveHook();
BOOL APIENTRY DllMain(HINSTANCE hMod, DWORD  fdwReason, LPVOID lpvReserved);

#endif	// VITRIDLL_H