
#include <LuaAide.h>
#include <iostream>

// Demo LuaUserData

using namespace std;

int panichandler(lua_State*L)
{
    printf("Lua Panik\n");
    LuaStack Q(L);
    throw runtime_error(Q.errormessage());
    return 0;
}

struct DemoClass
{
    int a, b, c;
};

static int mynew(lua_State*L)
{
    LuaStack Q(L);
    auto P=reinterpret_cast<DemoClass**>(lua_newuserdatauv(L, sizeof(DemoClass*), 0));
    Q<<LuaGlobal("mtdemo");
    lua_setmetatable(L, -2);
    if (Q.hastableat(-2))
    {
        const int a=Q(LuaElement(-2, 1))==LuaType::TNUMBER?Q.toint(-1):101; Q.drop(1);
        const int b=Q(LuaElement(-2, 2))==LuaType::TNUMBER?Q.toint(-1):102; Q.drop(1);
        const int c=Q(LuaElement(-2, 3))==LuaType::TNUMBER?Q.toint(-1):103; Q.drop(1);
        *P=new DemoClass {a, b, c};
    }
    else *P=new DemoClass {1, 2, 3};
    return 1;
}

static int myfinaliser(lua_State*L)
{
    LuaStack Q(L);
    if (lua_isuserdata(L, -1) && !lua_islightuserdata(L, -1))
    {
        auto*P=reinterpret_cast<DemoClass**>(lua_touserdata(L, -1));
        auto*X=*P;
        printf("finaliser deletes %p\n", X);
        *P=nullptr;
        delete X;
    }
    return 0;
}

static int mytostring(lua_State*L)
{
    LuaStack Q(L);
    if (lua_isuserdata(L, -1) && !lua_islightuserdata(L, -1))
    {
        auto*P=reinterpret_cast<DemoClass**>(lua_touserdata(Q, -1));
        auto*X=*P;
        char pad[100];
        snprintf(pad, sizeof(pad), "{%d, %d, %d}", X->a, X->b, X->c);
        Q<<pad;
        return 1;
    }
    else
    {
        Q<<"mtdemo.__tostring: internal error, arg is not a 'demo'\n">>luaerror;
        return 0;
    }
}

static void defineclass(lua_State*L)
{
    LuaStack Q(L);
    Q<<LuaTable(0,0)
        <<myfinaliser>>LuaField("__gc")
        <<mytostring>>LuaField("__tostring")
        >>LuaGlobal("mtdemo");
    Q<<mynew>>LuaGlobal("newdemo");
}

int main(int argc, char*argv[])
{
    LuaStack Q=LuaStack::New(true, panichandler);
    defineclass(Q);
    const auto rc=Q<<LuaCode(R"xxx(
        if true then
            local A=newdemo {31,32,33}
            print(string.format("A=%s",A))
            -- for k,v in ipairs(A) do print(k,v) end
        end
        collectgarbage()
    )xxx")>>0;
    printf("\n==============\nScript executed, rc=%d.\n", rc);
    if (rc!=0) cout<<Q;
    return 0;
}
