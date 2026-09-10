
#include <LuaAide.h>

using namespace std;
using namespace LuaAide;

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
        if (Q.hasat(LuaType::TNIL, -1))
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

// ============================================================================

#ifdef UNITTEST
#include <gtest/gtest.h>

static int panichandler(lua_State*L)
{
    LuaStack Q(L);
    throw runtime_error(Q.errormessage());
    return 0;
}

class MapEnv: public ::testing::Test
{
protected:
    LuaStack Q{};
    void SetUp() override { Q=LuaStack::New(true, panichandler); }
    void TearDown() override { Q.Close(); }
};

TEST_F(MapEnv, SimpleExample)
{
    Q<<LuaCode("return function(x) return 100+x end")>>1;
    auto mapfunc=Q.index(-1);
    Q<<lualist<<21<<22<<23<<lualistend;
    Q<<LuaAide::map<<LuaValue(-2)<<mapfunc>>1;
    ASSERT_EQ((int)LuaType::TTABLE, (int)Q.typeat(-1));
    for (LuaIterator J(Q); next(J); ++J)
    {
        EXPECT_EQ(LuaType::TNUMBER, Q.typeat(-1));
        EXPECT_EQ(20+(unsigned)J+100, Q.toint(-1));
    }
}

#endif
