
#include <LuaAide.h>
#include <chrono>

// Example module m2: Embedding timestamps as userdata
// (used by examples/m2test.lua):

using namespace std;
using namespace std::chrono_literals;
using highresclk=chrono::high_resolution_clock;
using tp=highresclk::time_point;

namespace {

const auto mtname="timestamp_highres";
const void*mtpointer=nullptr; // identify metatable via lua_topointer()

static bool istimestamp(lua_State*L, int index)
{
    LuaStack Q(L);
    if (Q.typeat(index)!=LuaType::TUSERDATA) return false;
    if (!lua_getmetatable(L, index)) return false;
    const void*p=lua_topointer(L, -1);
    lua_pop(L,1);
    return p==mtpointer;
}

extern "C" int tsdiff(lua_State*L)
{
    if (!istimestamp(L, 1)) return luaL_typeerror(L, 1, "timestamp");
    if (!istimestamp(L, 2)) return luaL_typeerror(L, 2, "timestamp");
    const tp T1=*reinterpret_cast<tp*>(lua_touserdata(L, 1)),
             T2=*reinterpret_cast<tp*>(lua_touserdata(L, 2));
    auto d=(T1-T2)/1ms;
    lua_pushinteger(L, d);
    return 1;
}

extern "C" int now(lua_State*L)
{
    LuaStack Q(L);
    auto*jetzt=reinterpret_cast<tp*>(lua_newuserdatauv(L, sizeof(tp), 0));    // [userdata]
    Q<<LuaValue(LUA_REGISTRYINDEX)<<LuaField(mtname); Q.remove(-2);           // [userdata, metatable]
    lua_setmetatable(L, -2);
    *jetzt=highresclk::now();
    return 1;
}

} // anon

extern "C" int luaopen_m2(lua_State*L)
{
    LuaStack Q(L);

    // Create metatable for timestamps.
    // Set Registry[mtname]={__name="timestamp", timestamp=function}:
    Q   <<LuaValue(LUA_REGISTRYINDEX)
            <<LuaTable()
                <<"timestamp">>LuaField("__name")
                <<tsdiff>>LuaField("__sub");
    mtpointer=lua_topointer(L, -1);
    Q       >>LuaField(mtname);

    // Return module table for 'require "m2"':
    Q   <<LuaTable()
        <<"https://github.com/vorgestern/LuaAide">>LuaField("origin")
        <<"0.1">>LuaField("version")
        <<now>>LuaField("now");
    return 1;
}
