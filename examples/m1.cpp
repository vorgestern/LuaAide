
#include <LuaAide.h>
#include <iostream>

// Handlungsbedarf:
// + Verbirg mtvec3 (LuaRegValue(tag))
// - wrap lua_newuserdatauv
// - wrap lua_setmetatable
// - wrap indizierten Zugriff auf Listenelemente
// + wrap lua_touserdata
// - Konzept für die Identifikation des Datentyps, der in userdata gekapselt ist.

using namespace std;

namespace { namespace Vec3 {

const LuaRegValue mtvec3("mtvec3");

struct V { double x, y, z; };

static double getelement(LuaStack&Q, int index, int e, const char name[])
{
    // Argument at index
    if (const auto t=Q(LuaElement(index, e)); t==LuaType::TNUMBER)
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

static V argvector(lua_State*L, int index)
{
    LuaStack  Q(L);
    if (Q.hasheavyuserdataat(index))
    {
        auto X=Q.touserpointer<V>(index);
        return*X;
    }
    if (Q.hastableat(index))
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
    auto P=reinterpret_cast<V**>(lua_newuserdatauv(L, sizeof(V*), 0));
    *P=new V {A};
    Q<<mtvec3;
    lua_setmetatable(L, -2);
    return 1;
}

static int myconstructor(LuaStack&Q, const V&arg)
{
    auto P=reinterpret_cast<V**>(lua_newuserdatauv(Q, sizeof(V*), 0));
    Q<<mtvec3;
    lua_setmetatable(Q, -2);
    *P=new V(arg);
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

static int myfinaliser(lua_State*L)
{
    LuaStack Q(L);
    if (Q.hasheavyuserdataat(-1))
    {
        auto X=Q.touserpointer<V**>(-1);
        // printf("finaliser deletes %p\n", X);
        *X=nullptr;
        delete *X;
    }
    return 0;
}

static int mytostring(lua_State*L)
{
    LuaStack Q(L);
    if (Q.hasheavyuserdataat(-1))
    {
        auto*X=Q.touserpointer<V>(-1);
        char pad[100];
        snprintf(pad, sizeof(pad), "{%g, %g, %g}", X->x, X->y, X->z);
        Q<<pad;
        return 1;
    }
    else
    {
        Q<<"mtvec3.__tostring: internal error, arg is not a 'Vec3'\n">>luaerror;
        return 0;
    }
}

static int myadd(lua_State*L)
{
    LuaStack Q(L);
    if (height(Q)<2) return Q<<"mtvec3.__add: Expect two arguments at least.">>luaerror;
    try {
        const V A=argvector(Q, -2), B=argvector(Q, -1);
        auto P=reinterpret_cast<V**>(lua_newuserdatauv(L, sizeof(V*), 0));
        Q<<mtvec3;
        lua_setmetatable(L, -2);
        *P=new V {A.x+B.x, A.y+B.y, A.z+B.z};
        return 1;
    }
    catch (const runtime_error&E) { return Q<<E.what()>>luaerror; }
}

static int mysubtract(lua_State*L)
{
    LuaStack Q(L);
    if (height(Q)<2) return Q<<"mtvec3.__sub: Expect two arguments at least.">>luaerror;
    try {
        const V A=argvector(Q, -2), B=argvector(Q, -1);
        auto P=reinterpret_cast<V**>(lua_newuserdatauv(L, sizeof(V*), 0));
        Q<<mtvec3;
        lua_setmetatable(L, -2);
        *P=new V {A.x-B.x, A.y-B.y, A.z-B.z};
        return 1;
    }
    catch (const runtime_error&E) { return Q<<E.what()>>luaerror; }
}

}}

using namespace Vec3;

extern "C" int luaopen_m1(lua_State*L)
{
    LuaStack Q(L);
    Q   <<LuaTable(0,0)
        <<myfinaliser>>LuaField("__gc")
        <<mytostring>>LuaField("__tostring")
        <<myadd>>LuaField("__add")
        <<mysubtract>>LuaField("__sub")
        >>mtvec3;
    Q  <<LuaTable()
        <<"0.1">>LuaField("version")
        <<mydemo>>LuaField("Demo")
        <<mynew>>LuaField("New");
    return 1;
}
