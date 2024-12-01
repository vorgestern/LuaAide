
@echo off

setlocal

title Helping Making "%1"

set MAKEDIR="%~dp0"
set REPOROOT=%MAKEDIR%\..\..
set gelaufen=0

ver > nul
2>nul call :%1
rem echo return mit %errorlevel%
rem echo gelaufen: %gelaufen%
if %gelaufen% neq 1 echo '%1' is not a known target.

:Alltagstest
    cd %REPOROOT%
    start /wait /b "" lua modules\alltag\Alltagstest.lua > %MAKEDIR%\Alltagstest.result
    set gelaufen=1
    goto Ende

:LuaAideTest99
    cd %REPOROOT%
    start /wait /b "" LuaAideTest.exe > %MAKEDIR%\LuaAideTest.result
    set gelaufen=1
    goto Ende

:m1test
    cd %REPOROOT%
    start /wait /b "" lua examples\m1test.lua > %MAKEDIR%\m1test.result
    set gelaufen=1
    goto Ende

:m2test
    cd %REPOROOT%
    start /wait /b "" lua examples\m2test.lua > %MAKEDIR%\m2test.result
    set gelaufen=1
    goto Ende

:Ende
