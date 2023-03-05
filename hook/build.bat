gcc -shared -o HookActiveBasicCompiler.dll HookGuiEvents.c -Wl,--kill-at -lDbghelp 
@REM gcc -shared -o HookActiveBasicCompiler.dll HookActiveBasicCompiler.c -Wl,--kill-at -lDbghelp 
gcc runABCompilerAsCLI.c -o runABCompilerAsCLI.exe -lshlwapi