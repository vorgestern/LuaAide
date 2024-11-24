
#include <cstring>
#include <string_view>
#include <LuaAide.h>

using std::string_view;
string_view chunk_ulutest();

static int mkloader(lua_State*L, const char name[], const string_view impl)
{
    LuaStack Q(L);
    if (impl.size()==0)
    {
        char pad[100];
        snprintf(pad, sizeof(pad), "Chunk is empty ('%s').", name);
        return Q<<pad>>luaerror;
    }
    const string_view errs[]=
    {
        "ok", // #define LUA_OK	0
        "yield error", // #define LUA_YIELD 1
        "runtime error", // #define LUA_ERRRUN 2
        "syntax error", // #define LUA_ERRSYNTAX 3
        "out of memory", // #define LUA_ERRMEM 4
        "unknown error", // #define LUA_ERRERR 5
    };
    if (const int rc=luaL_loadbufferx(Q, impl.data(), impl.size(), name, nullptr); rc==LUA_OK) return 1;
    else
    {
        char pad[100];
        snprintf(pad, sizeof(pad), "%s: loading '%s' failed with rc=%d.", errs[rc].data(), name, rc);
        return Q<<pad>>luaerror;
    }
}

#ifndef ULUTEST_EXPORTS
#define ULUTEST_EXPORTS
#endif

extern "C" ULUTEST_EXPORTS int luaopen_ulutest(lua_State*Q)
{
    const auto ulutest=chunk_ulutest();
    if (mkloader(Q, "ulutest", ulutest)==1) return lua_call(Q,0,1), 1;
    lua_pushliteral(Q, "ulutest cannot be loaded (internal error).");
    return lua_error(Q), 0;
}
