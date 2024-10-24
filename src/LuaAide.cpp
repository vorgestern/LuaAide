
#include <LuaAide.h>

LuaAbsIndex::LuaAbsIndex(LuaStack&S, int index): absindex(lua_absindex(S.L,index)){}
