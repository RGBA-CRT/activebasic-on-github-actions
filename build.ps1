ping -n 1 127.0.0.1 
start powershell -WindowStyle Minimized  ".\screenshot.ps1 test3.png"
.\.build\ActiveBasic\BasicCompiler.exe helloworld_rgbalib.abp ./build/helloworld_rgbalib.exe /wnd:0
.\.build\ActiveBasic\ProjectEditor.exe
start powershell -WindowStyle Minimized  ".\screenshot.ps1 test4.png"
ping -n 3 127.0.0.1 
start powershell -WindowStyle Minimized  ".\screenshot.ps1 test5.png"
ping -n 1  127.0.0.1 