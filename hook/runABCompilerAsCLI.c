/* injector.c */
#include <stdio.h>

#include <shlwapi.h>
#include <windows.h>

int main(int argc, char *argv[]) {
  char dllpath[MAX_PATH] = {0};
  GetModuleFileName(NULL, dllpath, sizeof(dllpath));
  PathRemoveFileSpec(dllpath);
  strncat(dllpath, "\\HookActiveBasicCompiler.dll", sizeof(dllpath) - 1);
  printf("dllpath=%s\n", dllpath);

  SECURITY_ATTRIBUTES sa = {0};
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = TRUE;  //継承可能にする
  HANDLE hReadHandle, hWriteHadle;
  if (!CreatePipe(&hReadHandle, &hWriteHadle, &sa, 0)) {
    printf("CreatePipe err GLE=%d\n",GetLastError());
    return -1;
  }

  STARTUPINFO tStartupInfo = {0};
  tStartupInfo.cb = sizeof(tStartupInfo);
  tStartupInfo.hStdError = hWriteHadle;
  tStartupInfo.hStdOutput = hWriteHadle;
  tStartupInfo.dwFlags = STARTF_USESTDHANDLES;
  // tStartupInfo.dwFlags |= STARTF_USESHOWWINDOW
  tStartupInfo.wShowWindow = SW_HIDE;

  PROCESS_INFORMATION tProcessInfomation = {0};
  BOOL bResult = CreateProcess(
      "BasicCompiler.exe",
      "BasicCompiler.exe helloworld_rgbalib.abp out.exe /wnd:0", NULL,
      NULL, TRUE, CREATE_SUSPENDED, NULL, NULL, &tStartupInfo,
      &tProcessInfomation);
  HANDLE hProcess = tProcessInfomation.hProcess;
  printf("createprocess =%d GLE=%d\n", bResult, GetLastError());
  if (hProcess == INVALID_HANDLE_VALUE) printf("process open error\n");
  void *datamemory = VirtualAllocEx(hProcess, NULL, sizeof(dllpath), MEM_COMMIT,
                                    PAGE_READWRITE);
  WriteProcessMemory(hProcess, datamemory, (void *)dllpath, sizeof(dllpath),
                     NULL);

  HMODULE kernel32 = GetModuleHandle("kernel32");
  FARPROC loadlibrary = GetProcAddress(kernel32, "LoadLibraryA");
  HANDLE hThread =
      CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadlibrary,
                         datamemory, 0, NULL);
  if (!hThread) {
    /* 32 bit (WOW64) -> 64 bit (Native) won't work */
    char errmsg[512];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0, errmsg,
                  sizeof(errmsg), NULL);
    printf("%s\n GLE=%d", errmsg, GetLastError());
    return 1;
  }
  WaitForSingleObject(hThread, INFINITE);

  CloseHandle(hThread);
  VirtualFreeEx(hProcess, datamemory, sizeof(dllpath), MEM_RELEASE);

  ResumeThread(tProcessInfomation.hThread);

  while (1) {
    DWORD dwAvailSize = 0;

    // printf(".\n");

    if (!PeekNamedPipe(hReadHandle, NULL, 0, NULL, &dwAvailSize, NULL)) {
      continue;
    }

    if (dwAvailSize > 0) {
      DWORD dwRead = 0;
      char buf[128];
      DWORD readSize = dwAvailSize < sizeof(buf) ? dwAvailSize : sizeof(buf);
      // printf("R %d %d %d\n", readSize, dwAvailSize, sizeof(buf));
      ReadFile(hReadHandle, buf, readSize, &dwRead, NULL);

      // printf("Re\n");
      fwrite(buf, dwRead, 1, stdout);
    }

    if (WAIT_OBJECT_0 == WaitForSingleObject(hProcess, 0)) {
      break;
    }
  }
  // WaitForSingleObject(hProcess, INFINITE);

  return 0;
}