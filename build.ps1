timeout 3
start powershell ".\screenshot.ps1 test3.png"
start powershell ".\stracent.exe -f stFilter.txt .\.build\ActiveBasic\BasicCompiler.exe helloworld_rgbalib.abp ./build/helloworld_rgbalib.exe /wnd:0"
start powershell ".\screenshot.ps1 test4.png"
timeout 3
start powershell ".\screenshot.ps1 test5.png"
timeout 10