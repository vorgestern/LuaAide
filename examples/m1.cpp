
#include <LuaAide.h>
#include <iostream>

using namespace std;

namespace { namespace Vec3 {

struct V { double x, y, z; };

static int mynew(lua_State*L)
{
    LuaStack Q(L);
    auto P=reinterpret_cast<V**>(lua_newuserdatauv(L, sizeof(V*), 0));
    Q<<LuaGlobal("mtvec3");
    lua_setmetatable(L, -2);
    if (Q.hastableat(-2))
    {
        const auto a=lua_geti(L, -2, 1)==LUA_TNUMBER?lua_tonumber(L, -1):101; lua_pop(L, 1);
        const auto b=lua_geti(L, -2, 2)==LUA_TNUMBER?lua_tonumber(L, -1):102; lua_pop(L, 1);
        const auto c=lua_geti(L, -2, 3)==LUA_TNUMBER?lua_tonumber(L, -1):103; lua_pop(L, 1);
        *P=new V {a, b, c};
    }
    else if (height(Q)>=4)
    {
        const auto a=lua_tonumber(L, 0), b=lua_tonumber(L, 1), c=lua_tonumber(L, 2);
        *P=new V {a, b, c};
    }
    else *P=new V {0, 0, 0};
    return 1;
}

static int myfinaliser(lua_State*L)
{
    LuaStack Q(L);
    if (lua_isuserdata(L, -1) && !lua_islightuserdata(L, -1))
    {
        auto*P=reinterpret_cast<V**>(lua_touserdata(L, -1));
        auto*X=*P;
        // printf("finaliser deletes %p\n", X);
        *P=nullptr;
        delete X;
    }
    return 0;
}

int mytostring(lua_State*L)
{
    LuaStack Q(L);
    if (lua_isuserdata(L, -1) && !lua_islightuserdata(L, -1))
    {
        auto*P=reinterpret_cast<V**>(lua_touserdata(Q, -1));
        auto*X=*P;
        char pad[100];
        snprintf(pad, sizeof(pad), "{%lf, %lf, %lf}", X->x, X->y, X->z);
        Q<<pad;
        return 1;
    }
    else
    {
        Q<<"mtvec3.__tostring: internal error, arg is not a 'Vec3'\n">>luaerror;
        return 0;
    }
}

int myadd(lua_State*L)
{
    LuaStack Q(L);
    if (height(Q)<2)
    {
        Q<<"mtvec3.__add: Expect two arguments at least.">>luaerror;
    }
//  cout<<"myadd: "<<Q;
    const V*A=nullptr, *B=nullptr;
    if (lua_isuserdata(L, -2) && !lua_islightuserdata(L, -2))
    {
        auto*P=reinterpret_cast<V**>(lua_touserdata(Q, -2));
        A=*P;
    }
    if (lua_isuserdata(L, -1) && !lua_islightuserdata(L, -1))
    {
        auto*P=reinterpret_cast<V**>(lua_touserdata(Q, -1));
        B=*P;
    }
//  printf("A, B %p, %p\n", A, B);
    if (A!=nullptr && B!=nullptr)
    {
        auto P=reinterpret_cast<V**>(lua_newuserdatauv(L, sizeof(V*), 0));
        Q<<LuaGlobal("mtvec3");
        lua_setmetatable(L, -2);
        *P=new V {A->x+B->x, A->y+B->y, A->z+B->z};
//      auto*p=*P;
//      printf("P=%lf, %lf, %lf\n", p->x, p->y, p->z);
        return 1;
    }
    else
    {
        Q<<"mtvec3.__add: internal error, arg is not a 'Vec3'\n">>luaerror;
        return 0;
    }
}

int mysubtract(lua_State*L)
{
    LuaStack Q(L);
    if (height(Q)<2)
    {
        Q<<"mtvec3.__sub: Expect two arguments at least.">>luaerror;
    }
//  cout<<"myadd: "<<Q;
    const V*A=nullptr, *B=nullptr;
    if (lua_isuserdata(L, -2) && !lua_islightuserdata(L, -2))
    {
        auto*P=reinterpret_cast<V**>(lua_touserdata(Q, -2));
        A=*P;
    }
    if (lua_isuserdata(L, -1) && !lua_islightuserdata(L, -1))
    {
        auto*P=reinterpret_cast<V**>(lua_touserdata(Q, -1));
        B=*P;
    }
//  printf("A, B %p, %p\n", A, B);
    if (A!=nullptr && B!=nullptr)
    {
        auto P=reinterpret_cast<V**>(lua_newuserdatauv(L, sizeof(V*), 0));
        Q<<LuaGlobal("mtvec3");
        lua_setmetatable(L, -2);
        *P=new V {A->x-B->x, A->y-B->y, A->z-B->z};
//      auto*p=*P;
//      printf("P=%lf, %lf, %lf\n", p->x, p->y, p->z);
        return 1;
    }
    else
    {
        Q<<"mtvec3.__sub: internal error, arg is not a 'Vec3'\n">>luaerror;
        return 0;
    }
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
        >>LuaGlobal("mtvec3");
    Q  <<LuaTable()
        <<"0.1">>LuaField("version")
        <<mynew>>LuaField("New");
    return 1;
}
