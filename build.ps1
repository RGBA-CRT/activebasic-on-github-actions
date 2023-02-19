# ping -n 1 127.0.0.1 
# start powershell -WindowStyle Minimized  ".\screenshot.ps1 test3.png"
# .\.build\ActiveBasic\BasicCompiler.exe helloworld_rgbalib.abp ./build/helloworld_rgbalib.exe /wnd:0
# .\.build\ActiveBasic\ProjectEditor.exe
# start powershell -WindowStyle Minimized  ".\screenshot.ps1 test4.png"
# ping -n 3 127.0.0.1 
# start powershell -WindowStyle Minimized  ".\screenshot.ps1 test5.png"
# ping -n 1  127.0.0.1 

$code = @'
    [DllImport("user32.dll")]
     public static extern IntPtr GetForegroundWindow();
    [DllImport("user32.dll")]
    public static extern IntPtr GetWindowThreadProcessId(IntPtr hWnd, out int ProcessId);
'@

Add-Type -AssemblyName System.Windows.Forms
mkdir .\.build\ActiveBasic\UserSetting
copy *.ini .\.build\ActiveBasic\UserSetting
.\.build\ActiveBasic\ProjectEditor.exe
Start-Sleep -s 1
[System.Windows.Forms.SendKeys]::SendWait("{ENTER}")
Start-Sleep -s 1
taskkill /IM ProjectEditor.exe
Start-Sleep -s 1
.\.build\ActiveBasic\ProjectEditor.exe helloworld_rgbalib.abp
Start-Sleep -s 1
[System.Windows.Forms.SendKeys]::SendWait("{F7}")
start powershell -WindowStyle Minimized  ".\screenshot.ps1 test9.png"
# Add-Type $code -Name Utils -Namespace Win32
# $myPid = [IntPtr]::Zero;

# $hwnd = [Win32.Utils]::GetForegroundWindow()
# $hwnd_hex = $hwnd.ToString("X").PadLeft(8,'0')
# echo $hwnd
# echo $hwnd_hex
# echo "./stracent -f stFilter.txt .\.build\ActiveBasic\BasicCompiler.exe helloworld_rgbalib.abp helloworld_rgbalib.exe /wnd:$hwnd_hex /clip_compile_view"
# start powershell -WindowStyle Minimized  ".\screenshot.ps1 test5.png"
# ./stracent -f stFilter.txt .\.build\ActiveBasic\BasicCompiler.exe helloworld_rgbalib.abp helloworld_rgbalib.exe /wnd:$hwnd_hex /clip_compile_view
# start powershell -WindowStyle Minimized  ".\screenshot.ps1 test6.png"


Start-Sleep -s 1

[System.Windows.Forms.SendKeys]::SendWait("{F5}")

Start-Sleep -s 1
start powershell -WindowStyle Minimized  ".\screenshot.ps1 testA.png"