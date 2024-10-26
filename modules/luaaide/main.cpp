
#include <LuaAide.h>

extern "C" int luaopen_luaaide(lua_State*L)
{
    LuaStack Q(L);
    // const int h1=height(Q); printf("\nstack height 1: %d", h1);
    // for (int k=-h1; k<0; ++k){ printf("\n#%d ", k);  printval(L, k); }
    Q<<LuaTable()
        <<"1.2.3">>LuaField("version");
    // const int h2=height(Q); printf("\nstack height 2: %d\n", h2);
    return 1;
}
