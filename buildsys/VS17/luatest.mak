
# Nmake Makefile for Unittests
# Called from Tester.mak

REPOROOT= $(MAKEDIR)\..\..

all: Alltagstest.result LuaAideTest.result

clean:
    @del /F Alltagstest.result LuaAideTest.result

Alltagstest.result: $(REPOROOT)\alltag.dll $(REPOROOT)\modules\alltag\Alltagstest.lua
    @cd $(REPOROOT)
    @date /T > $(MAKEDIR)\Alltagstest.result
    @time /T >> $(MAKEDIR)\Alltagstest.result
    @lua modules\alltag\Alltagstest.lua >> $(MAKEDIR)\Alltagstest.result

LuaAideTest.result: $(REPOROOT)\LuaAideTest.exe
    @cd $(REPOROOT)
    @date /T > $(MAKEDIR)\LuaAideTest.result
    @time /T >> $(MAKEDIR)\LuaAideTest.result
    @LuaAideTest.exe >> $(MAKEDIR)\LuaAideTest.result

testsummary.result: Alltagstest.result LuaAideTest.result
    @date /T > $(MAKEDIR)\testsummary.result
    @time /T >> $(MAKEDIR)\testsummary.result
    @lua testsummary.lua testsummary.result Alltagstest.result LuaAideTest.result

help:
    echo "MAKEDIR=$(MAKEDIR)"
    echo "REPOROOT=$(REPOROOT)"
    echo "Targets: all clean Alltagstest.result LuaAideTest.result
