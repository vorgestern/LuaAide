
#include <LuaAide.h>
#include <iostream>
#include <filesystem>

using namespace std;
using fspath=filesystem::path;

int formatany(lua_State*);
int keyescape(lua_State*);
int keyescape1(lua_State*);

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

namespace {
extern "C" int pwd(lua_State*L)
{
    LuaStack Q(L);
    auto X=filesystem::current_path().string();
    Q<<X.c_str();
    return 1;
}

extern "C" int cd(lua_State*L)
{
    LuaStack Q(L);
    if (height(Q)==1)
    {
        char pad[100];
        if (Q.hasstringat(-1))
        {
            const fspath neu(Q.tostring(-1));
            if (!filesystem::exists(neu))
            {
                const string meld="path does not exist: '"+neu.string()+"'";
                Q<<meld>>luaerror;
            }
            std::error_code ec;
            current_path(neu, ec);
            if (!ec) return 0;
            else
            {
                sprintf(pad, "cwd: system error %d", ec.value());
                Q<<pad>>luaerror;
            }
        }
        else
        {
            sprintf(pad, "cd requires string argument <path>, not %d", lua_type(L, -1));
            Q<<pad>>luaerror;
        }
    }
    else Q<<"cd requires argument (string path)">>luaerror;
    return 0;
}
}

extern "C" int luaopen_luaaide(lua_State*L)
{
    LuaStack Q(L);
    Q<<LuaTable()
        <<"0.1">>LuaField("version")
        <<pwd>>LuaField("pwd")
        <<cd>>LuaField("cd")
        <<formatany>>LuaField("formatany")
        <<keyescape1>>LuaField("keyescape")
        <<demofail>>LuaField("demofail") // Produziert eine Fehlermeldung aus einem Aufruf von LuaAide.
    ;
    return 1;
}
