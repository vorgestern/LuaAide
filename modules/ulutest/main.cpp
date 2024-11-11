
#include <cstring>
#include <string_view>
#include <LuaAide.h>

using namespace std;

// Die Adressen dieser Symbole liegen in ltest.o,
// wo sie von objcopy erzeugt wurden (beim Bauen unter Linux).
// Beachte die Anpassung der Namen in syminfo (opjcopy --redefine-syms=modules/ulutest/syminfo)
extern "C" char ltest_start;
extern "C" char ltest_end;

static int mkloader(lua_State*L, const char name[], const string_view impl)
{
    LuaStack Q(L);
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
        return Q<<pad>>luaerror;
    }
}

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
