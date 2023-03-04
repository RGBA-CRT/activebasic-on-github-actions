/* hook.c */
#include <windows.h>

#include <Dbghelp.h> /* ImageDirectoryEntryToData */
#include <stdio.h>
#include <tlhelp32.h> /* CreateToolhelp32Snapshot */
#pragma comment(lib, "Dbghelp")

FARPROC origShellAboutA;
FARPROC orgHeapReAlloc;
FARPROC orgHeapAlloc;
FARPROC orgSetWindowTextA;
FARPROC orgSetDlgItemTextA;
FARPROC orgSendMessage;
FARPROC orgPostMessage;
FARPROC orgDefDlgProcA;
FARPROC orgTextOutA;
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

  printf("[mod] %s - %08X -> %08X \n", modname, origaddr, newaddr,
         GetLastError());

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

int WINAPI newShellAboutA(HWND hWnd, LPCTSTR szApp, LPCTSTR szOtherStuff,
                          HICON hIcon) {
  printf("newShellAboutA %s\n", szOtherStuff);
  return origShellAboutA(hWnd, "ALL YOUR BASE ARE BELONG TO US", szOtherStuff,
                         hIcon);
}

LPVOID WINAPI newHeapReAlloc(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem,
                             SIZE_T dwBytes) {
  dwBytes *= 10;
  dwFlags = HEAP_ZERO_MEMORY;
  LPVOID ret = orgHeapReAlloc(hHeap, dwFlags, lpMem, dwBytes);

  printf("[ReAlloc] Area:%08X %08X bytes Flag:%X ", lpMem, dwBytes, dwFlags);
  if (ret == lpMem)
    printf("\n");
  else if (ret == NULL)
    printf("==> failed!!!!!!!!!!!!!!!!!!\n");
  else
    printf(" ==> moved:%08X\n", ret);
  fflush(stdout);

  return ret;
}

LPVOID WINAPI newHeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes) {
  dwBytes *= 10;
  LPVOID ret = orgHeapAlloc(hHeap, dwFlags, dwBytes);
  printf("[new] Area:%08X %08X bytes\n", ret, dwBytes);
  return ret;
}

BOOL WINAPI newSetWindowText(HWND hWnd, LPCTSTR lpString) {
  printf("[SetWindowText] %X <-- (%x)%s \n", hWnd, lpString, lpString);
}

BOOL WINAPI newSetDlgItemTextA(HWND hWnd, int nIDDlgItem, LPCTSTR lpString) {
  printf("[%s] %X:%X <-- (%x)%s \n", __func__, hWnd, nIDDlgItem, lpString,
         lpString);
  orgSetDlgItemTextA(hWnd, nIDDlgItem, lpString);
  if (strcmp("•Â‚¶‚é", lpString) == 0) {
    printf("•Â‚¶‚é‚Ý‚Â‚¯‚½\n");
     orgPostMessage(hWnd, WM_CLOSE, 0, 0);
  }
}

LRESULT WINAPI newSendMessage(HWND hWnd, UINT Msg, WPARAM wParam,
                              LPARAM lParam) {
  printf("[SendMessage] %x:%x", hWnd, Msg);
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

  return orgSendMessage(hWnd, Msg, wParam, lParam);
}
LRESULT WINAPI newPostMessage(HWND hWnd, UINT Msg, WPARAM wParam,
                              LPARAM lParam) {
  printf("[PostMessage] %04x:%04x", hWnd, Msg);
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

  return orgPostMessage(hWnd, Msg, wParam, lParam);
}
LRESULT WINAPI newSendDlgItemMessageA(HWND hWnd, int dlgItem, UINT Msg,
                                      WPARAM wParam, LPARAM lParam) {
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

  return orgSendDlgItemMessageA(hWnd, dlgItem, Msg, wParam, lParam);
}
LRESULT WINAPI newDefDlgProcA(HWND hWnd, UINT Msg, WPARAM wParam,
                              LPARAM lParam) {
  printf("[DefDlgProcA] %04x:%04x", hWnd, Msg);
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

  return orgDefDlgProcA(hWnd, Msg, wParam, lParam);
}
LRESULT WINAPI newTextOutA(HDC hdc, int x, int y, LPCSTR lpString, int c) {
  printf("[TextOut] (%d,%d) %s", x, y, lpString);
  return orgTextOutA(hdc, x, y, lpString, c);
}
int WINAPI newMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption,
                          UINT uType) {
  printf("[MessageBox] %s - %s\n", lpText, lpCaption);
  // orgMessageBoxA(hWnd,lpText,lpCaption, uType);
  return MB_OK;
}

#include <io.h> // _open_osfhandle
#include <fcntl.h> // _O_TEXT
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
  HMODULE shell32 = GetModuleHandle("shell32");
  HMODULE kernel32 = GetModuleHandle("kernel32");
  HMODULE user32 = GetModuleHandle("user32");

  origShellAboutA = GetProcAddress(shell32, "ShellAboutA");
  orgHeapReAlloc = GetProcAddress(kernel32, "HeapReAlloc");
  orgHeapAlloc = GetProcAddress(kernel32, "HeapAlloc");
  orgSetWindowTextA = GetProcAddress(user32, "SetWindowTextA");

  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      // AllocConsole();
      // freopen("CONOUT$", "w", stdout);
      // //	freopen ( "c:\\log.log", "w", stdout );
      // freopen("CONIN$", "r", stdin);
      /* connect Windows STDOUT(by CreateProcess's StartupInfo.hStdOutput) */
      {
        STARTUPINFO si = {0};
        GetStartupInfo(&si);
        printf("getstdout=%d, si.out=%d, title=%s\n", GetStdHandle(STD_OUTPUT_HANDLE), si.hStdOutput,si.lpTitle);
        WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), "PIPEWRITE\n", 10,NULL,NULL);
        int fd = _open_osfhandle((intptr_t)si.hStdOutput, _O_TEXT);
        // FILE* fpt =_fdopen(fd, "w");
        // fprintf(fpt, "printftest\n");
        *stdout = *_fdopen( fd, "w" );
        setvbuf( stdout, NULL, _IONBF, 0 );
      }
      printf("DLL INJECTOR HOOK.DLL\n");

      // modifyIAT("shell32.dll", origShellAboutA, newShellAboutA);
      // modifyIAT("kernel32.dll", orgHeapReAlloc, newHeapReAlloc);
      // modifyIAT("kernel32.dll", orgHeapAlloc, newHeapAlloc);
      modifyIAT("user32.dll", orgSetWindowTextA, newSetWindowText);
      orgSetDlgItemTextA =
          modifyIATbyName("user32.dll", "SetDlgItemTextA", newSetDlgItemTextA);
      orgSendMessage =
          modifyIATbyName("user32.dll", "SendMessageA", newSendMessage);
      orgPostMessage =
          modifyIATbyName("user32.dll", "PostMessageA", newPostMessage);
      orgSendDlgItemMessageA = modifyIATbyName(
          "user32.dll", "SendDlgItemMessageA", newSendDlgItemMessageA);
      orgDefDlgProcA =
          modifyIATbyName("user32.dll", "DefWindowProc", newDefDlgProcA);
      orgTextOutA = modifyIATbyName("gdi32.dll", "TextOutA", newTextOutA);
      orgMessageBoxA =
          modifyIATbyName("user32.dll", "MessageBoxA", newMessageBoxA);

      break;
    case DLL_PROCESS_DETACH:
      modifyIAT("shell32.dll", newShellAboutA, origShellAboutA);
      // origMessageBox(0,"stop","",0);
      // getchar();
      break;
  }

  return TRUE;
}
