
#include <LuaAide.h>
// #include <iostream>
#include <cassert>

using namespace std;
using namespace LuaAide;

int LuaAide::keymap(lua_State*L)
{
    LuaStack Q(L);                                               // [Table, func]
    if (Q.hasat(LuaType::TNIL, 1)) return 0;

    Q<<lualist<<lualistend<<luarot_3;                            // [func, Result, Table]
    const auto func=Q.index(-3), Result=Q.index(-2);

    for (LuaIterator J(Q); next(J); ++J)
    {
        auto k=Q.index(-2), v=Q.index(-1);
        Q<<k<<LuaFuncValue(func)<<v<<k>>1;
        lua_settable(Q, stackindex(Result));
    }
    Q<<Result;
    return 1;
}

int LuaAide::applypairs(lua_State*L)
{
    LuaStack Q(L);                                               // [Table, proc]
    if (Q.hasat(LuaType::TNIL, 1)) return 0;
    Q.swap();                                                    // [proc, Table]
    auto proc=Q.index(-2);
    for (LuaIterator J(Q); next(J); ++J)
    {
        // [proc, Table, k, v]
        const auto k=Q.index(-2), v=Q.index(-1);
        Q<<LuaFuncValue(proc)<<k<<v>>0;
    }
    return 0;
}

int LuaAide::findfirst(lua_State*L)
{
    LuaStack Q(L);                                               // [Table, pred]
    if (Q.hasat(LuaType::TNIL, 1)) return 0;
    if (height(Q)==1) Q<<idfunc;
    Q<<luanil<<luarot3;                                          // [nil, Table, pred]
    Q<<luanil<<luarot3;                                          // [nil, nil, Table, pred]
    Q.swap();                                                    // [nil, nil, pred, Table]
    const auto kr=Q.index(-4), vr=Q.index(-3), pred=Q.index(-2);
    bool found=false;
    for (LuaIterator J(Q); next(J); ++J)
    {
        // [nil, nil, pred, Table, k, v]
        auto k=Q.index(-2), v=Q.index(-1);
        Q<<LuaFuncValue(pred)<<v>>1;                             // [nil, nil, pred, Table, k, v, pred(v)]
        if (Q.hasat(LuaType::TNIL, -1) || (Q.hasat(LuaType::TBOOLEAN, -1) && !Q.tobool(-1)))
        {
            Q.drop(1);
            continue;
        }
        Q.drop(1);
        Q<<k; lua_replace(Q, stackindex(kr));
        Q<<v; lua_replace(Q, stackindex(vr));
        found=true;
        break;
    }
    if (found)
    {
        Q<<vr<<kr;
        return 2;
    }
    else return 0;
}

int LuaAide::contains(lua_State*L)
{
    LuaStack Q(L);                                               // [List, item]
    if (height(Q)!=2) return  0;
    if (Q.hasat(LuaType::TNIL, -2)) return 0;
    Q<<luanil; // [List, item, result]
    Q<<luarot_3; // [item, result, List]
    const auto item=Q.index(-3), result=Q.index(-2);
    bool found=false;
    for (LuaIterator J(Q); next(J) && !found; ++J)
    {
        // [item, result, List, k, v]
        if (lua_compare(Q, -1, stackindex(item), LUA_OPEQ))
        {
            found=true;
            Q<<LuaValue(-2);
            lua_replace(Q, stackindex(result));
        }
//      else cout<<" neq\n";
    }
    if (found)
    {
        assert(height(Q)==3);
        Q.drop(1);
        return 1;
    }
    else return 0;
}
