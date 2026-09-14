
#include <LuaAide.h>

using namespace LuaAide;

bool LuaAide::next(LuaPairs&X) //!< Expect current index on the stack. Increment it and push corresponding value.
{
    if (lua_next(X.L, -2)!=0) return true;
    else
    {
        X.index=0;
        return false;
    }
}

LuaAide::LuaPairs::LuaPairs(LuaStack&S): L(S){ lua_pushnil(L); }

LuaAide::LuaPairs::~LuaPairs()
{
    // Index>0: Loop was left by break.
    if (index>0) lua_pop(L, 2);
}

LuaAide::LuaPairs::operator unsigned(){ return index; }

unsigned LuaAide::LuaPairs::operator++()
{
    lua_pop(L, 1);
    return ++index;
}
