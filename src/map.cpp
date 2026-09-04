
#include <LuaAide.h>

using namespace std;

int LuaAide::map(lua_State*L)
{
    LuaStack Q(L);
    if (height(Q)==0) return 0;
    if (Q.typeat(-2)!=LuaType::TTABLE) return 0;
    Q<<lualist<<lualistend<<luarot_3; // ==> [func {} list]
    auto func=Q.index(-3), mappedlist=Q.index(-2);
    for (LuaIterator J(Q); next(J); ++J)
    {
        // Call func(item)
        // Catch nil-results, because they are not permissible in mapping.
        Q<<LuaFuncValue(stackindex(func))<<LuaValue(-2)>>1;
        if (Q.hasnilat(-1))
        {
            auto item=Q.index(-2);
            Q<<LuaGlobal("string")<<LuaDotCall("format")<<"map: function returns nil for element %d, which is: %s"<<(unsigned)J<<item>>1;
            Q>>luaerror;
        }
        Q>>LuaElement({stackindex(mappedlist), (unsigned)J}); // add result to mappedlist.
    }
    Q<<mappedlist;
    return 1;
}
