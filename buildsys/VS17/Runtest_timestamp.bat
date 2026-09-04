
@echo off

setlocal

title Testing module 'timestamp'

set MAKEDIR="%~dp0"
set REPOROOT=%MAKEDIR%\..\..

ver > nul

cd %REPOROOT%
lua examples/moduletest_timestamp.lua > %MAKEDIR%\moduletest_timestamp.result
