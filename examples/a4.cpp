
#include <LuaAide.h>
#include <iostream>

// Example 'a4' for embedding Lua:
// Userdata
// Make C++ class DemoClass available to Lua scripts.

// - defineclass creates Metatable (global 'mtdemo')
//   and Constructor (global 'newdemo')
// - Inline script create instance and prints it.

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
        const auto a=Q(LuaElement(-2, 1))==LuaType::TNUMBER?Q.toint(-1):101; Q.drop(1);
        const auto b=Q(LuaElement(-2, 2))==LuaType::TNUMBER?Q.toint(-1):102; Q.drop(1);
        const auto c=Q(LuaElement(-2, 3))==LuaType::TNUMBER?Q.toint(-1):103; Q.drop(1);
        *P=new DemoClass {(int)a, (int)b, (int)c};
    }
    else *P=new DemoClass {1, 2, 3};
    return 1;
}

static int myfinaliser(lua_State*L)
{
    LuaStack Q(L);
    if (Q.hasheavyuserdataat(-1))
    {
        auto X=Q.touserdata<DemoClass**>(-1);
        printf("finaliser deletes %p\n", *X);
        delete *X;
        *X=nullptr;
    }
    return 0;
}

static int mytostring(lua_State*L)
{
    LuaStack Q(L);
    if (Q.hasheavyuserdataat(-1))
    {
        auto X=Q.touserpointer<DemoClass>(-1);
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

    // Use democlass from C++
#if 0
    Q<<mynew<<LuaTable()<<55>>LuaElement(-2, 1)<<56>>LuaElement(-2,2)<<57>>LuaElement(-2,3)>>1;  // democlass {55,56,57}
#else
    Q<<lualist<<55<<56<<57<<lualistend;  // democlass {55,56,57}
    Q<<mynew<<LuaValue(-2)>>1;
    Q.remove(-2);
#endif
    // Save string representation in 'demostring'.
    Q[LuaMetaMethod::tostring]>>1;
    Q>>LuaGlobal("demostring");
    Q.drop(1);

    const auto rc=Q<<LuaCode(R"xxx(
        if true then
            local A=newdemo {31,32,33}
            print(string.format("A=%s",A))
            -- for k,v in ipairs(A) do print(k,v) end
        end
        collectgarbage()
        print "demostring:"
        print(demostring)
    )xxx")>>0;
    printf("\n==============\nScript executed, rc=%d.\n", rc);
    if (rc!=0) cout<<Q;
    return 0;
}
