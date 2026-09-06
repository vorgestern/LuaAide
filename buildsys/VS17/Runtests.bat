
@echo off

setlocal

title Running all tests

set MAKEDIR=%~dp0
set REPOROOT=%MAKEDIR%\..\..

ver > nul

cd %REPOROOT%

.\LuaAideTest.exe > %MAKEDIR%LuaAideTest.result
lua examples\moduletest_vec3.lua > %MAKEDIR%\moduletest_vec3.result
lua examples\moduletest_timestamp.lua > %MAKEDIR%\moduletest_timestamp.result
lua examples\moduletest_colorenum.lua > %MAKEDIR%\moduletest_colorenum.result

echo summarise_tests.lua
echo %MAKEDIR%LuaAideTest.result
echo %MAKEDIR%moduletest_vec3.result
echo %MAKEDIR%moduletest_timestamp.result
echo %MAKEDIR%moduletest_colorenum.result
lua buildsys\generic\summarise_tests.lua TestSummary.lua %MAKEDIR%LuaAideTest.result %MAKEDIR%moduletest_vec3.result %MAKEDIR%moduletest_timestamp.result %MAKEDIR%moduletest_colorenum.result
lua TestSummary.lua
