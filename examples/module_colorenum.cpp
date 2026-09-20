
#include <LuaAide.h>
#include <iostream>
#include <cstdint>

// Example module of extending scripts:
// Expose an opaque 'colorenum' to Lua.
// moduletest_colorenum.lua demonstrates its use.

using namespace std;
using namespace LuaAide;

enum class colortype: uint16_t {
    LCT_GREY=0,         // grayscale: 1,2,4,8,16 bit
    LCT_RGB=2,          // RGB: 8,16 bit
    LCT_PALETTE=3,      // palette: 1,2,4,8 bit
    LCT_GREY_ALPHA=4,   // grayscale with alpha: 8,16 bit
    LCT_RGBA=6          // RGB with alpha: 8,16 bit
};

namespace {

const auto mtname="mtcolortype";
const void*mtpointer=nullptr; // identify metatable via lua_topointer()
const UV<colortype,0> Inst;

bool iscolortype(lua_State*L, int index)
{
    LuaStack Q(L);
    if (!Q.hasat(LuaType::TUSERDATA, index)) return false;
    if (!lua_getmetatable(L, index)) return false;
    const void*p=lua_topointer(L, -1);
    Q.drop(1);
    return p==mtpointer;
}

int mytostring(lua_State*L)
{
    LuaStack Q(L);
    Q.argcheck(1, iscolortype, "colortype");
    auto P=Q>>Inst;
    switch (*P.luadata)
    {
        case colortype::LCT_GREY: return Q<<"LCT_GREY", 1;
        case colortype::LCT_RGB: return Q<<"LCT_RGB", 1;
        case colortype::LCT_PALETTE: return Q<<"LCT_PALETTE", 1;
        case colortype::LCT_GREY_ALPHA: return Q<<"LCT_GREY_ALPHA", 1;
        case colortype::LCT_RGBA: return Q<<"LCT_RGBA", 1;
        default: return Q<<"<unknown>", 1;
    }
}

int mynumeric(lua_State*L)
{
    LuaStack Q(L);
    Q.argcheck(1, iscolortype, "colortype");
    auto P=Q>>Inst;
    Q<<(int)*P.luadata;
    return 1;
}

} // anon

static void neu(lua_State*L, colortype value)
{
    LuaStack Q(L);
    Q<<Inst =value;
}

extern "C" int luaopen_colorenum(lua_State*L)
{
    LuaStack Q(L);

    // Create metatable.
    Q   <<LuaValue(LUA_REGISTRYINDEX)
            <<newtable
                <<"colortype">>LuaMetaMethod::name
                <<mynumeric>>LuaField("numeric")
                <<mytostring>>LuaMetaMethod::tostring;
    mtpointer=lua_topointer(L, -1);
    Q.dup(); Q>>LuaMetaMethod::index;
    Q       >>LuaField(mtname);
    Q.drop(1);

    Q<<LuaValue(LUA_REGISTRYINDEX)<<LuaField(mtname);
    auto mt=Q.index(-1);

    // Return module table for 'require "colorenum"':
    Q   <<newtable
            <<"https://github.com/vorgestern/LuaAide/examples/module_colorenum.cpp">>LuaField("origin")
            <<"0.1">>LuaField("version");

    Q       <<LuaTable();
                neu(Q, colortype::LCT_GREY); Q<<mt; lua_setmetatable(Q, -2); Q>>LuaField("LCT_GREY");
                neu(Q, colortype::LCT_RGB); Q<<mt; lua_setmetatable(Q, -2); Q>>LuaField("LCT_RGB");
                neu(Q, colortype::LCT_PALETTE); Q<<mt; lua_setmetatable(Q, -2); Q>>LuaField("LCT_PALETTE");
                neu(Q, colortype::LCT_GREY_ALPHA); Q<<mt; lua_setmetatable(Q, -2); Q>>LuaField("LCT_GREY_ALPHA");
                neu(Q, colortype::LCT_RGBA); Q<<mt; lua_setmetatable(Q, -2); Q>>LuaField("LCT_RGBA");
    Q       >>LuaField("colortype");
    return 1;
}
