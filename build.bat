@echo off

setlocal

set DIR=%~dp0
set SLN=%DIR%src\diana_14_0.sln
set DEVENV="%VS140COMNTOOLS%..\IDE\devenv.com"

%DEVENV% %SLN% /Build "Release|Win32" || goto :eof
%DEVENV% %SLN% /Build "Release|x64" || goto :eof
%DEVENV% %SLN% /Build "Debug|Win32" || goto :eof
%DEVENV% %SLN% /Build "Debug|x64" || goto :eof

echo ok
endlocal
