
#include <LuaAide.h>
#include <filesystem>

using namespace std;
using fspath=filesystem::path;

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
    // const int h1=height(Q); printf("\nstack height 1: %d", h1);
    // for (int k=-h1; k<0; ++k){ printf("\n#%d ", k);  printval(L, k); }
    Q<<LuaTable()
        <<"1.2.3">>LuaField("version")
        <<pwd>>LuaField("pwd");
    Q<<cd;
    Q>>LuaField("cd");
    // const int h2=height(Q); printf("\nstack height 2: %d\n", h2);
    return 1;
}
