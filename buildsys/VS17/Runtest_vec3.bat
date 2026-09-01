
@echo off

setlocal

title Testing module 'vec3'

set MAKEDIR="%~dp0"
set REPOROOT=%MAKEDIR%\..\..

ver > nul

cd %REPOROOT%
lua examples/moduletest_vec3.lua > %MAKEDIR%\moduletest_vec3.result
