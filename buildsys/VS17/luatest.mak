
# Nmake Makefile for Unittests
# Called from Tester.mak

REPOROOT= $(MAKEDIR)\..\..

all: alltag.result LuaAideTest.result

clean:
    @del /F alltag.test

alltag.result: $(REPOROOT)\alltag.dll $(REPOROOT)\modules\alltag\test.lua
    @cd $(REPOROOT)
    @date /T > $(MAKEDIR)\alltag.result
    @time /T >> $(MAKEDIR)\alltag.result
    @lua modules\alltag\test.lua >> $(MAKEDIR)\alltag.result

LuaAideTest.result: $(REPOROOT)\LuaAideTest.exe
    @cd $(REPOROOT)
    @date /T > $(MAKEDIR)\LuaAideTest.result
    @time /T >> $(MAKEDIR)\LuaAideTest.result
    @LuaAideTest.exe >> $(MAKEDIR)\LuaAideTest.result

testsummary.result: alltag.result LuaAideTest.result
    @date /T > $(MAKEDIR)\testsummary.result
    @time /T >> $(MAKEDIR)\testsummary.result
    @lua testsummary.lua testsummary.result alltag.result LuaAideTest.result

help:
    echo "MAKEDIR=$(MAKEDIR)"
    echo "REPOROOT=$(REPOROOT)"
    echo "Targets: all clean alltag.test
