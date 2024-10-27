
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
        if (Q.hasstringat(-1))
        {
            const fspath neu(Q.tostring(-1));
            if (!filesystem::exists(neu))
            {
                string meld="path does not exist: '";
                meld.append(neu.string());
                meld.append("'");
                Q<<meld.c_str();
                lua_error(L);
            }
            std::error_code ec;
            current_path(neu, ec);
            if (!ec)
            {
                return 0;
            }
            else
            {
                char pad[100];
                sprintf(pad, "cwd: system error %d", ec.value());
                Q<<pad;
                lua_error(L);
                return 0;
            }
        }
        else
        {
            char pad[100];
            sprintf(pad, "cd requires string argument (path), not %d", lua_type(L, -1));
            Q<<pad;
            lua_error(L);
            return 0;
        }
    }
    else
    {
        char pad[100];
        sprintf(pad, "cd requires argument (string path)");
        Q<<pad;
        lua_error(L);
        return 0;
    }
}
}

extern "C" int luaopen_luaaide(lua_State*L)
{
    LuaStack Q(L);
    // const int h1=height(Q); printf("\nstack height 1: %d", h1);
    // for (int k=-h1; k<0; ++k){ printf("\n#%d ", k);  printval(L, k); }
    Q<<LuaTable()
        <<"1.2.3">>LuaField("version")
        <<pwd;
    Q>>LuaField("pwd");
    Q<<cd;
    Q>>LuaField("cd");
    // const int h2=height(Q); printf("\nstack height 2: %d\n", h2);
    return 1;
}
