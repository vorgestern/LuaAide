
#include <LuaAide.h>
#include <iostream>

// Example of embedding C++ classes
// Make C++ class DemoClass available to Lua scripts.

// - defineclass creates Metatable (global 'mtdemo') and constructor (global 'newdemo')
// - Inline script creates instance and prints it.

using namespace std;
using namespace LuaAide;

int panichandler(lua_State*L)
{
    printf("Lua Panic\n");
    LuaStack Q(L);
    throw runtime_error(Q.errormessage());
    return 0;
}

struct DemoClass
{
    int a, b, c;
};
using POD=LuaUserPOD<DemoClass>;
const UV<DemoClass> NewInst;

static int mynew(lua_State*L)
{
    LuaStack Q(L);
    auto P=Q<<NewInst;
    Q<<LuaGlobal("mtdemo");
    lua_setmetatable(L, -2);
    if (Q.hasat(LuaType::TTABLE, -2))
    {
        const auto a=Q(LuaElement({-2, 1}))==LuaType::TNUMBER?Q.toint(-1):101; Q.drop(1);
        const auto b=Q(LuaElement({-2, 2}))==LuaType::TNUMBER?Q.toint(-1):102; Q.drop(1);
        const auto c=Q(LuaElement({-2, 3}))==LuaType::TNUMBER?Q.toint(-1):103; Q.drop(1);
        P=DemoClass {(int)a, (int)b, (int)c};
    }
    else P=DemoClass {1, 2, 3};
    return 1;
}

static int mytostring(lua_State*L)
{
    LuaStack Q(L);
    if (Q.hasat(LuaType::TUSERDATA, -1))
    {
        POD P;
        Q>>P;
        char pad[100];
        snprintf(pad, sizeof(pad), "{%d, %d, %d}", P.luadata->a, P.luadata->b, P.luadata->c);
        Q<<pad;
        return 1;
    }
    else
    {
        Q<<"mtdemo:tostring: internal error, arg is not a 'demo'\n">>luaerror;
        return 0;
    }
}

static void defineclass(lua_State*L)
{
    LuaStack Q(L);
    Q<<newtable
        <<mytostring>>LuaMetaMethod::tostring
        >>LuaGlobal("mtdemo");
    Q<<mynew>>LuaGlobal("newdemo");
}

int main()
{
    LuaStack Q=LuaStack::New(true, panichandler);
    defineclass(Q);

    // Use democlass from C++
#if 0
    Q<<mynew<<newtable<<55>>LuaElement({-2, 1})<<56>>LuaElement({-2,2})<<57>>LuaElement({-2,3})>>1;  // democlass {55,56,57}
#else
    Q<<lualist<<55<<56<<57<<lualistend;  // democlass {55,56,57}
    Q<<LuaCFunction(mynew)<<LuaValue(-2)>>1;
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
