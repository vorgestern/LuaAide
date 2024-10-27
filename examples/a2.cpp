
#include <cstdio>
#include <iostream>
#include <LuaAide.h>

using namespace std;

int democlosure(lua_State*L)
{
    LuaStack Q(L);
    Q<<LuaTable(0, 0); Q.swap()<<LuaUpValue(1); Q.swap();    // Result, Map, Arg
    LuaAbsIndex Result(Q, -3), Map(Q, -2), Arg(Q, -1);
//  cout<<Q;
    for (LuaIterator J(Q); next(J); ++J)
    {
        printf("\nRunde %u (height %u)", (unsigned)J, height(Q));
        //                                                   // Result, Map, Arg, key, value
        Q.dup(-1);
        lua_gettable(L, stackindex(Map));                    // Result, Map, Arg, key, value, Map[value]
        if (Q.hasnilat(-1))
        {
            printf("\n%u is nil", (unsigned)J);
            // Result[J]=value
            Q.drop(1);                                       // Result, Map, Arg, key, value
            Q.dup(-1);                                       // Result, Map, Arg, key, value, value
            lua_seti(L, stackindex(Result), (unsigned)J);    // Result, Map, Arg, key, value
        }
        else
        {
            printf("\n%u is not nil", (unsigned)J);
            // Result[J]=Up1[value]                          // Result, Map, Arg, key, value, Map[value]
            lua_seti(L, stackindex(Result), (unsigned)J);    // Result, Map, Arg, key, value
        }
    }                                                        // Result, Map, Arg
    Q.drop(2);                                               // Result
    return 1;
}

int main()
{
    LuaStack Q=LuaStack::New(true, nullptr);
    if (!Q) return printf("Failed to initialise Lua\n"), 1;
    if (true)
    {
        // Make Map
        Q<<LuaTable(3, 0); LuaAbsIndex Map(Q);
        Q<<21; lua_seti(Q, stackindex(Map), 1);
        Q<<22; lua_seti(Q, stackindex(Map), 2);
        Q<<23; lua_seti(Q, stackindex(Map), 3);
        // Make Closure with upvalue1==Map
        Q<<LuaClosure(democlosure, 1); LuaAbsIndex Closure1(Q);
        // Make Argument table
        Q<<LuaTable(7, 0); LuaAbsIndex Arg(Q);
        Q<<1<<1;       lua_settable(Q, stackindex(Arg));
        Q<<2<<2;       lua_settable(Q, stackindex(Arg));
        Q<<3<<3;       lua_settable(Q, stackindex(Arg));
        Q<<4<<"Hier";  lua_settable(Q, stackindex(Arg));
        Q<<5<<"Dort";  lua_settable(Q, stackindex(Arg));
        Q<<6<<"Woher"; lua_settable(Q, stackindex(Arg));
        Q<<7<<"Wohin"; lua_settable(Q, stackindex(Arg));
        Q<<8<<"Wann";  lua_settable(Q, stackindex(Arg));
        Q<<9<<"Wie";   lua_settable(Q, stackindex(Arg));
        Q<<10<<"Wozu"; lua_settable(Q, stackindex(Arg));
        lua_call(Q, 1, 1);
        cout<<"\n"<<Q<<"\n";
    }
    return 0;
}
