
@echo off

setlocal

title Testing module 'colorenum'

set MAKEDIR="%~dp0"
set REPOROOT=%MAKEDIR%\..\..

ver > nul

cd %REPOROOT%
lua examples/moduletest_colorenum.lua > %MAKEDIR%\moduletest_colorenum.result
