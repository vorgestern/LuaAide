
#include <LuaAide.h>
#include <iostream>

using namespace std;

int panichandler(lua_State*L)
{
    LuaStack Q(L);
    // printf("Lua panics, %d items on Stack\n", height(Q));
    throw runtime_error(Q.errormessage());
    return 0;
}

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

int main_throwing(lua_State*L)
{
    LuaStack Q=L;
    if (!Q) return printf("Failed to initialise Lua\n"), 1;
    if (false)
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
        Q<<21<<22<<23;
        Q<<make_pair("Closuredemo", LuaCode(R"xxx(
            function translate(A, M
                local R={}
                for _,e in ipairs(A) do table.insert(R, M[e] or e) end
                return R
            end
            Closures={
                fleissig=function(A) return translate(A, {Hier="Dort", wohnen="arbeiten", ["Schlümpfe"]="Zwerge"}) end,
                faul=function(A)
                    error "faul!"
                    return translate(A, {Hier="Wo", wohnen="schlafen", ["Schlümpfe"]="Heinzelmännchen"})
                end
            }
        )xxx"))>>0;
        if (const auto rc=Q<<LuaGlobal("Closures")<<LuaDotCall("fleissig")<<vector<string>{"Hier", "wohnen", "die", "Schlümpfe"}>>1; rc==0)
        {
            Q<<LuaGlobal("table")<<LuaDotCall("concat")<<LuaValue(-2)<<" ">>make_pair(1, 1);
            Q<<LuaGlobalCall("print")<<LuaValue(-2)>>make_pair(1, 0);
        }
        else return printf("%s\n", Q.errormessage().c_str()), 1;
        if (const auto rc=Q<<LuaGlobal("Closures")<<LuaDotCall("faul")<<vector<string>{"Hier", "wohnen", "die", "Schlümpfe"}>>1; rc==0)
        {
            Q<<LuaGlobal("table")<<LuaDotCall("concat")<<LuaValue(-2)<<" ">>make_pair(1, 1);
            Q<<LuaGlobalCall("print")<<LuaValue(-2)>>make_pair(1, 0);
        }
        else return printf("%s\n", Q.errormessage().c_str()), 1;
    }
    return 0;
}

int main()
{
    LuaStack Q=LuaStack::New(true, panichandler);
    try { return main_throwing(Q); }
    catch (const runtime_error&E)
    {
        printf("Runtime error:\n%s\n", E.what());
        cout<<Q<<"\n";
        return 0;
    }
}
