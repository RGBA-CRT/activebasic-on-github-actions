gcc -shared -o HookActiveBasicCompiler.dll HookActiveBasicCompiler.c -Wl,--kill-at -lDbghelp 
gcc runABCompilerAsCLI.c -o runABCompilerAsCLI.exe -lshlwapi