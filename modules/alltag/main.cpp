
#include <LuaAide.h>
#include <iostream>
#include <filesystem>

using namespace std;
using fspath=filesystem::path;

int demofail(lua_State*L)
{
    LuaStack Q(L);
    // Dieses Skript lässt sich nicht kompilieren (fehlende Klammer).
    Q<<make_pair("Failing Demo", LuaCode(R"xxx(
        function translate(A,
            return 1
        end
    )xxx"))>>0;
    Q<<1;
    return 1;
}

#ifndef ALLTAG_EXPORTS
#define ALLTAG_EXPORTS
#endif

extern "C" ALLTAG_EXPORTS int luaopen_alltag(lua_State*L)
{
    LuaStack Q(L);
    Q<<newtable
        <<"0.1">>LuaField("version")
        <<formatany>>LuaField("formatany")
        <<keyescape>>LuaField("keyescape")
        <<demofail>>LuaField("demofail") // Produziert eine Fehlermeldung aus einem Aufruf von LuaAide.
    ;
    return 1;
}
