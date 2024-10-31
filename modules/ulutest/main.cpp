
#include <cstring>
#include <string_view>
#include <lua.hpp>

using namespace std;

extern "C" char ltest_start;
extern "C" char ltest_end;

static int mkloader(lua_State*Q, const char name[], const string_view impl)
{
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
        sprintf(pad, "%s: loading '%s' failed with rc=%d.", errs[rc].data(), name, rc);
        lua_pushstring(Q, pad);
        return lua_error(Q),0;
    }
}

static int mkloader(lua_State*Q, lua_CFunction impl)
{
    return lua_pushcfunction(Q, impl),1;
}

#if 0
static int searcher(lua_State*Q)
{
    if (!lua_isstring(Q, -1))
    {
        lua_pushliteral(Q, "Module-searcher for 'LuaToXML' requires string argument.");
        return lua_error(Q),0;
    }
    const char*name=lua_tostring(Q, -1);
    printf("searcher %s\n", name);
    if (0!=strncmp(name, "LuaToXML", 8))
    {
        // Diese Fehlermeldung ist an die der generischen Searcher angelehnt.
        lua_pushfstring(Q, "no module '%s' in module 'LuaToXML'.", name);
        return 1;
    }
    if (0==strcmp(name, "ulutest.init")) return mkloader(Q, name, ulutest.init);
    return 0;
}

static void install_searcher(lua_State*Q, lua_CFunction s)
{
    if (const int typ=lua_getglobal(Q, "package"); typ!=LUA_TTABLE) { lua_remove(Q, -1); return; }
    if (const int typ=lua_getfield(Q, -1, "searchers"); typ!=LUA_TTABLE) { lua_remove(Q, -1); return; }
    const auto neuindex=lua_rawlen(Q, -1)+1;
    lua_pushcfunction(Q, s);
    lua_seti(Q, -2, neuindex);
}
#endif

#ifdef WIN32
#define EXTERN __declspec(dllexport)
#else
#define EXTERN
#endif

extern "C" EXTERN int luaopen_ulutest(lua_State*Q)
{
    const string_view ulutest(&ltest_start, &ltest_end-&ltest_start);
    if (mkloader(Q, "ulutest", ulutest)==1) return lua_call(Q,0,1), 1;
    lua_pushliteral(Q, "ulutest cannot be loaded (internal error).");
    return lua_error(Q),0;
}
