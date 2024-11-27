
#include <lua.hpp>
#include <iostream>
#include <chrono>

using namespace std;
using namespace std::chrono_literals;
using highresclk=chrono::high_resolution_clock;
using tp=highresclk::time_point;

namespace {

const void*mtpointer=nullptr; // identify metatable via lua_topointer()

extern "C" int tsdiff(lua_State*L)
{
    auto istimestamp=[L](int index) {
        if (lua_type(L, index)!=LUA_TUSERDATA) return false;
        if (!lua_getmetatable(L, index)) return false;
        const void*p=lua_topointer(L, -1);
        lua_pop(L,1);
        return p==mtpointer;
    };
    if (!istimestamp(1))
    {
        lua_pushliteral(L, "tsdiff(tb, ta) expects two timestamps (created with tshighres), but tb is not a timestamp.");
        return lua_error(L), 0;
    }
    if (!istimestamp(2))
    {
        lua_pushliteral(L, "tsdiff(tb, ta) expects two timestamps (created with tshighres), but ta is not a timestamp.");
        return lua_error(L), 0;
    }
    const tp T1=*reinterpret_cast<tp*>(lua_touserdata(L, 1)),
             T2=*reinterpret_cast<tp*>(lua_touserdata(L, 2));
    auto d=(T1-T2)/1ms;
    lua_pushnumber(L, d);
    return 1;
}

static void mygetmetatable_timestamp(lua_State*L)
{
    const auto mtname="timestamp_highres";
    // printf("\n\n    mygetmetatable_timestamp [%d].", lua_gettop(L));
    lua_pushvalue(L, LUA_REGISTRYINDEX);                                // Registry
    // printf("\n    Registry [%d]", lua_gettop(L));
    const auto t1=lua_getfield(L, -1, mtname);      // Register mt|nil
    // printf("\n    Registry.%s: Typ %d [%d]", mtname, t1, lua_gettop(L));
    if (t1==LUA_TNIL)
    {
        lua_pop(L, 1);                              // Registry
        // printf("\n    Registry.%s ist nil, erzeuge also die Tabelle [%d].\n", mtname, lua_gettop(L));
        lua_createtable(L, 0, 2);                   // Reg {}
        mtpointer=lua_topointer(L, -1);
        // printf("\nmtpointer %p\n", mtpointer);
            lua_pushliteral(L, "timestamp");        // Reg {} ".."
            lua_setfield(L, -2, "__name");          // Reg {__name=".."}
            lua_pushcfunction(L, tsdiff);           // Reg {...} tsdiff
            lua_setfield(L, -2, "__sub");           // Reg {...m __sub=tsdiff}
        lua_setfield(L, -2, mtname);                // Reg
        const auto t2=lua_getfield(L, -1, mtname);  // Reg mt|nil
        if (t2!=LUA_TTABLE)
        {
            lua_pushstring(L, "Kann keine Tabelle erzeugen.");
            lua_error(L);
        }
        lua_remove(L, -2);  // mt
    }
    else if (t1!=LUA_TTABLE)
    {
        lua_pushfstring(L, "Registry.%s ist keine Tabelle sondern ein %d", mtname, t1);
        lua_error(L);
    }
    else
    {
        lua_remove(L, -2); // mt
    }
}

extern "C" int timestamp_hres(lua_State*L)
{
    auto*jetzt=reinterpret_cast<tp*>(lua_newuserdatauv(L, sizeof(tp), 0));
    mygetmetatable_timestamp(L);
    lua_setmetatable(L, -2);
    *jetzt=highresclk::now();
    return 1;
}

} // anon

extern "C" int luaopen_m2(lua_State*L)
{
    lua_pushcfunction(L, timestamp_hres);     // ts
    lua_setglobal(L, "tshighres");            // --
    lua_getglobal(L, "tshighres");            // ts
    return 1;
}
