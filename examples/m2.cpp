
#include <LuaAide.h>
#include <chrono>
#include <thread>

// Example module m2 for extending scripts:
// Expose type 'timestamp' to Lua.
// m2test.lua and m2demo.lua demonstrate its use.

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

extern "C" int tostring(lua_State*L)
{
    LuaStack Q(L);
    if (!istimestamp(Q, 1)) return luaL_typeerror(Q, 1, "timestamp");
    const tp T1=*reinterpret_cast<tp*>(lua_touserdata(Q, 1));
    char pad[100];
    const size_t nw=snprintf(pad, sizeof(pad), "%.3fs", 0.001*(T1.time_since_epoch()/1ms));
    return Q<<string_view {pad, nw}, 1;
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

extern "C" int sleep_ms(lua_State*L)
{
    LuaStack Q(L);
    if (height(Q)<1) return Q<<"sleep_ms expects argument (ms: number)">>luaerror;
    if (Q.typeat(1)!=LuaType::TNUMBER) return luaL_typeerror(Q, 1, "number");
    const auto num=Q.todouble(1);
    this_thread::sleep_for(num*1ms);
    return 0;
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
                <<tostring>>LuaField("__tostring")
                <<tsdiff>>LuaField("__sub");
    mtpointer=lua_topointer(L, -1);
    Q       >>LuaField(mtname);

    // Return module table for 'require "m2"':
    Q   <<LuaTable()
        <<"https://github.com/vorgestern/LuaAide">>LuaField("origin")
        <<"0.1">>LuaField("version")
        <<now>>LuaField("now")
        <<sleep_ms>>LuaField("sleep_ms");
    return 1;
}
