
#include <LuaAide.h>
#include <iostream>

using namespace std;

int democlosure(lua_State*L)
{
    LuaStack Q(L);
    Q<<LuaArray(0)<<LuaUpValue(1)<<luarot_3;                 // Result, Map, Arg
    auto Result=Q.index(-3), Map=Q.index(-2);
    for (LuaIterator J(Q); next(J); ++J)                     // Result, Map, Arg, key, value
    {
        Q.dup(-1);
        lua_gettable(L, stackindex(Map));                    // Result, Map, Arg, key, value, Map[value]
        // Result[J]=Map[value] or value
        if (Q.hasnilat(-1)) Q.drop(1).dup(-1);               // Result, Map, Arg, key, value, value
        lua_seti(L, stackindex(Result), (unsigned)J);        // Result, Map, Arg, key, value
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
        vector<string>Arg {"Hier", "wohnen", "die", "Schlümpfe"};
        Q<<Arg;
        // Make Map, Make Closure with upvalue==Map, Call Closure with Argument table
        Q   <<unordered_map<string,string> {{"Hier", "Dort"}, {"wohnen", "arbeiten"}, {"Schlümpfe", "Zwerge"}}<<LuaClosure(democlosure, 1)
            <<Arg>>1;
        Q   <<unordered_map<string,string> {{"Hier", "Wo"}, {"wohnen", "schlafen"}, {"Schlümpfe", "Heinzelmännchen"}}<<LuaClosure(democlosure, 1)
            <<Arg>>1;
        cout<<"\n"<<Q<<"\n";
    }
    return 0;
}
