
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
        // Make Map, Make Closure with upvalue==Map, Call Closure with Argument Arg.
        Q   <<unordered_map<string,string> {{"Hier", "Dort"}, {"wohnen", "arbeiten"}, {"Schlümpfe", "Zwerge"}}<<LuaClosure(democlosure, 1)
            <<Arg>>1;
        Q   <<unordered_map<string,string> {{"Hier", "Wo"}, {"wohnen", "schlafen"}, {"Schlümpfe", "Heinzelmännchen"}}<<LuaClosure(democlosure, 1)
            <<Arg>>1;
        cout<<"\n"<<Q<<"\n";
    }
    if (true)
    {
        Q<<LuaCode(R"xxx(
            function translate(A, M)
                local R={}
                for _,e in ipairs(A) do table.insert(R, M[e] or e) end
                return R
            end
            Closures={
                fleissig=function(A) return translate(A, {Hier="Dort", wohnen="arbeiten", ["Schlümpfe"]="Zwerge"}) end,
                faul=    function(A) return translate(A, {Hier="Wo", wohnen="schlafen", ["Schlümpfe"]="Heinzelmännchen"}) end
            }
        )xxx")>>0;
        Q<<LuaGlobal("Closures")<<LuaDotCall("fleissig")<<vector<string>{"Hier", "wohnen", "die", "Schlümpfe"}>>1;
        Q<<LuaGlobal("table")<<LuaDotCall("concat")<<LuaValue(-2)<<" ">>1;
        Q<<luaswap; Q.drop(1);
        Q<<LuaGlobalCall("print")<<LuaValue(-2)>>0;

        Q<<LuaGlobal("Closures")<<LuaDotCall("faul")<<vector<string>{"Hier", "wohnen", "die", "Schlümpfe"}>>1;
        Q<<LuaGlobal("table")<<LuaDotCall("concat")<<LuaValue(-2)<<" ">>1;
        Q<<luaswap; Q.drop(1);
        Q<<LuaGlobalCall("print")<<LuaValue(-2)>>0;
    }
    return 0;
}
