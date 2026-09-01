
@echo off

setlocal

title Executing LuaAideTest

set MAKEDIR="%~dp0"
set REPOROOT=%MAKEDIR%\..\..

ver > nul

cd %REPOROOT%
LuaAideTest.exe > %MAKEDIR%\LuaAideTest.result
