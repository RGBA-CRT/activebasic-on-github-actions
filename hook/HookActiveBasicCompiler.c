/* hook.c */
#include <stdio.h>
#include <windows.h>

#include <Dbghelp.h>  /* ImageDirectoryEntryToData */
#include <tlhelp32.h> /* CreateToolhelp32Snapshot */
#pragma comment(lib, "Dbghelp")

FARPROC orgSetWindowTextA;
FARPROC orgSetDlgItemTextA;
FARPROC orgSendMessage;
FARPROC orgPostMessage;
FARPROC orgSendDlgItemMessageA;
FARPROC orgMessageBoxA;

void modifyIATonemod(char *modname, void *origaddr, void *newaddr,
                     HMODULE hModule) {
  ULONG ulSize;
  PIMAGE_IMPORT_DESCRIPTOR pImportDesc;
  pImportDesc = ImageDirectoryEntryToData(
      hModule, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &ulSize);
  if (pImportDesc == NULL) {
    return;
  }

  /* seek the target DLL */
  while (pImportDesc->Name) {
    char *name = (char *)hModule + pImportDesc->Name;
    if (lstrcmpi(name, modname) == 0) {
      break;
    }
    pImportDesc++;
  }
  if (pImportDesc->Name == 0) {
    return;
  }

  /* modify corrensponding IAT entry */
  PIMAGE_THUNK_DATA pThunk =
      (PIMAGE_THUNK_DATA)((char *)hModule + pImportDesc->FirstThunk);
  while (pThunk->u1.Function) {
    PROC *paddr = (PROC *)&pThunk->u1.Function;
    if (*paddr == origaddr) {
      DWORD flOldProtect;
      VirtualProtect(paddr, sizeof(paddr), PAGE_EXECUTE_READWRITE,
                     &flOldProtect);
      *paddr = newaddr;
      VirtualProtect(paddr, sizeof(paddr), flOldProtect, &flOldProtect);
    }
    pThunk++;
  }
}

void modifyIAT(char *modname, void *origaddr, void *newaddr) {
  HANDLE hModuleSnap =
      CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());

  // printf("[mod] %s - %08X -> %08X \n", modname, origaddr, newaddr,
  //        GetLastError());

  MODULEENTRY32 me;
  me.dwSize = sizeof(me);

  /* modify IAT in all loaded modules */
  BOOL bModuleResult = Module32First(hModuleSnap, &me);
  while (bModuleResult) {
    modifyIATonemod(modname, origaddr, newaddr, me.hModule);
    bModuleResult = Module32Next(hModuleSnap, &me);
  }

  CloseHandle(hModuleSnap);
}

void *modifyIATbyName(char *modname, char *funcname, void *newaddr) {
  HMODULE hmodule = GetModuleHandle(modname);
  void *origadr = GetProcAddress(hmodule, funcname);
  modifyIAT(modname, origadr, newaddr);
  return origadr;
}

/* ====================================================================== */

BOOL WINAPI newSetDlgItemTextA(HWND hWnd, int nIDDlgItem, LPCTSTR lpString) {
  // printf("[%s] %X:%X <-- (%x)%s \n", __func__, hWnd, nIDDlgItem, lpString,
  //        lpString);
  size_t textSize = strlen(lpString);
  if (strncmp("Version", lpString, min(6, textSize)) == 0) {
    printf("ActiveBasic %s\n", lpString);

  } else if (strncmp("’†’f", lpString, min(6, textSize)) == 0) {
    printf("Start compile\n", lpString);
  } else if (strncmp("•Â‚¶‚é", lpString, min(6, textSize)) == 0) {
    orgPostMessage(hWnd, WM_CLOSE, 0, 0);
  }else{
    printf("ABC: %s\n", lpString);
  }
  return orgSetDlgItemTextA(hWnd, nIDDlgItem, lpString);
}

LRESULT WINAPI newSendMessage(HWND hWnd, UINT Msg, WPARAM wParam,
                              LPARAM lParam) {
  // printf("[SendMessage] %x:%x", hWnd, Msg);
  // if (wParam & 0xFF000000) {
  //   printf(" w:%08x(%8s...)", wParam, wParam);
  // } else {
  //   printf(" w:%08x", wParam);
  // }
  // if (lParam & 0xFF000000) {
  //   printf(" l:%08x(%8s...)\n", lParam, lParam);
  // } else {
  //   printf(" l:%08x\n", lParam);
  // }

  return orgSendMessage(hWnd, Msg, wParam, lParam);
}
LRESULT WINAPI newPostMessage(HWND hWnd, UINT Msg, WPARAM wParam,
                              LPARAM lParam) {
  // printf("[PostMessage] %04x:%04x", hWnd, Msg);
  // if (wParam & 0xFF000000) {
  //   printf(" w:%08x(%8s...)", wParam, wParam);
  // } else {
  //   printf(" w:%08x", wParam);
  // }
  // if (lParam & 0xFF000000) {
  //   printf(" l:%08x(%8s...)\n", lParam, lParam);
  // } else {
  //   printf(" l:%08x\n", lParam);
  // }

  return orgPostMessage(hWnd, Msg, wParam, lParam);
}
LRESULT WINAPI newSendDlgItemMessageA(HWND hWnd, int dlgItem, UINT Msg,
                                      WPARAM wParam, LPARAM lParam) {
#if 0
  printf("[SendDlgItemMessageA] %04x:%d msg=%x", hWnd, dlgItem, Msg);
  if (wParam & 0xFF000000) {
    printf(" w:%08x(%8s...)", wParam, wParam);
  } else {
    printf(" w:%08x", wParam);
  }
  if (lParam & 0xFF000000) {
    printf(" l:%08x(%8s...)\n", lParam, lParam);
  } else {
    printf(" l:%08x\n", lParam);
  }
#else
  if (lParam & 0xFF000000) {
    printf("ABC: %s\n", lParam);
  }
#endif
  return orgSendDlgItemMessageA(hWnd, dlgItem, Msg, wParam, lParam);
}

int WINAPI newMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption,
                          UINT uType) {
  printf("ABC: !!!! %s - %s\n", lpText, lpCaption);
  // orgMessageBoxA(hWnd,lpText,lpCaption, uType);
  return MB_OK;
}

void DoReplaceApi() {
  orgSetDlgItemTextA =
      modifyIATbyName("user32.dll", "SetDlgItemTextA", newSetDlgItemTextA);
  orgSendMessage =
      modifyIATbyName("user32.dll", "SendMessageA", newSendMessage);
  orgPostMessage =
      modifyIATbyName("user32.dll", "PostMessageA", newPostMessage);
  orgSendDlgItemMessageA = modifyIATbyName("user32.dll", "SendDlgItemMessageA",
                                           newSendDlgItemMessageA);
  orgMessageBoxA = modifyIATbyName("user32.dll", "MessageBoxA", newMessageBoxA);
}

#include <fcntl.h>  // _O_TEXT
#include <io.h>     // _open_osfhandle

void ConnectStdout() {
  // AllocConsole();
  // freopen("CONOUT$", "w", stdout);
  // freopen("CONIN$", "r", stdin);

  /* connect Windows STDOUT(by CreateProcess's StartupInfo.hStdOutput) */
  {
    STARTUPINFO si = {0};
    GetStartupInfo(&si);
    int fd = _open_osfhandle((intptr_t)si.hStdOutput, _O_TEXT);
    *stdout = *_fdopen(fd, "w");
    setvbuf(stdout, NULL, _IONBF, 0);
  }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      ConnectStdout();
      DoReplaceApi();

      break;
    case DLL_PROCESS_DETACH:
      break;
  }

  return TRUE;
}
