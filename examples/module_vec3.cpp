
#include <LuaAide.h>
#include <iostream>

// Example module for extending scripts:
// Expose type 'vec3' to Lua.
// moduletest_vec3.lua demonstrates its use.

// Handlungsbedarf:
// + Verbirg mtvec3 (LuaRegValue(tag))
// - wrap lua_setmetatable
// - wrap indizierten Zugriff auf Listenelemente
// - Konzept für die Identifikation des Datentyps, der in userdata gekapselt ist.

using namespace std;
using namespace LuaAide;

namespace { namespace Vec3 {

const LuaRegValue mtvec3("mtvec3");

struct Vec3 { double x, y, z; };

using POD=LuaUserPOD<Vec3>;
const UV<Vec3,0> NewVec;

static double getelement(LuaStack&Q, int index, int e, const char name[])
{
    // Argument at index
    if (const auto t=Q(LuaElement({index, e})); t==LuaType::TNUMBER)
    {
        auto value=Q.todouble(-1);
        Q.drop(1);
        return value;
    }
    else Q.drop(1);
    if (const auto t=lua_getfield(Q, index, name); t==LUA_TNUMBER)
    {
        auto value=Q.todouble(-1);
        Q.drop(1);
        return value;
    }
    else Q.drop(1);
    return 0;
}

static Vec3 argvector(lua_State*L, int index)
{
    LuaStack  Q(L);
    if (Q.hasat(LuaType::TUSERDATA, index))
    {
        POD P;
        Q<<LuaValue(index)>>P;
        Q.drop(1);
        return*P.luadata;
    }
    if (Q.hasat(LuaType::TTABLE, index))
    {
        const auto x=getelement(Q, index, 1, "x"),
                   y=getelement(Q, index, 2, "y"),
                   z=getelement(Q, index, 3, "z");
        return {x, y, z};
    }
    throw runtime_error("Argument is not a vector");
}

static int mydemo(lua_State*L)
{
    LuaStack Q(L);
    const auto A=argvector(Q, -1);
    Q<<NewVec =A;
    Q<<mtvec3;
    lua_setmetatable(L, -2);
    return 1;
}

static int myconstructor(LuaStack&Q, const Vec3&arg)
{
    Q<<NewVec=arg;
    Q<<mtvec3;
    lua_setmetatable(Q, -2);
    return 1;
}

static int mynew(lua_State*L)
{
    LuaStack Q(L);
    if (height(Q)==1) return myconstructor(Q, argvector(Q, -1));
    else if (height(Q)>=3)
    {
        const auto x=Q.todouble(1), y=Q.todouble(2), z=Q.todouble(3);
        return myconstructor(Q, {x,y,z});
    }
    else return myconstructor(Q, {0,0,0});
}

static int mytostring(lua_State*L)
{
    LuaStack Q(L);
    if (Q.hasat(LuaType::TUSERDATA, -1))
    {
        POD P;
        Q>>P;
        auto X=*P.luadata;
        char pad[100];
        snprintf(pad, sizeof(pad), "{%g, %g, %g}", X.x, X.y, X.z);
        Q<<pad;
        return 1;
    }
    else
    {
        Q<<"mtvec3:tostring: internal error, arg is not a 'Vec3'\n">>luaerror;
        return 0;
    }
}

static int myadd(lua_State*L)
{
    LuaStack Q(L);
    if (height(Q)<2) return Q<<"mtvec3.__add: Expect two arguments at least.">>luaerror;
    try {
        const Vec3 A=argvector(Q, -2), B=argvector(Q, -1);
        Q<<NewVec=Vec3 {A.x+B.x, A.y+B.y, A.z+B.z};
        Q<<mtvec3;
        lua_setmetatable(L, -2);
        return 1;
    }
    catch (const runtime_error&E) { return Q<<E.what()>>luaerror; }
}

static int mysubtract(lua_State*L)
{
    LuaStack Q(L);
    if (height(Q)<2) return Q<<"vec3:sub: Expect two arguments at least (self,other).">>luaerror;
    try {
        const Vec3 A=argvector(Q, -2), B=argvector(Q, -1);
        Q<<NewVec=Vec3 {A.x-B.x, A.y-B.y, A.z-B.z};
        Q<<mtvec3;
        lua_setmetatable(L, -2);
        return 1;
    }
    catch (const runtime_error&E) { return Q<<E.what()>>luaerror; }
}

}}

using namespace Vec3;

extern "C" int luaopen_vec3(lua_State*L)
{
    LuaStack Q(L);
    Q   <<newtable
        <<mytostring>>LuaMetaMethod::tostring
        <<myadd>>LuaMetaMethod::add
        <<mysubtract>>LuaMetaMethod::sub
        >>mtvec3;
    Q   <<newtable
        <<"0.1">>LuaField("version")
        <<"https://github.com/vorgestern/LuaAide/examples/module_vec3.cpp">>LuaField("origin")
        <<mydemo>>LuaField("Demo")
        <<mynew>>LuaField("New");
    return 1;
}
