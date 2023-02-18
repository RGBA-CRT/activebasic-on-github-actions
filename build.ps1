ping -n 3 127.0.0.1 
start powershell ".\screenshot.ps1 test3.png"
start powershell ".\stracent.exe -f stFilter.txt .\.build\ActiveBasic\BasicCompiler.exe helloworld_rgbalib.abp ./build/helloworld_rgbalib.exe /wnd:0"
start powershell ".\screenshot.ps1 test4.png"
timping -n 3 127.0.0.1 
start powershell ".\screenshot.ps1 test5.png"
timetimping -n 10  127.0.0.1 