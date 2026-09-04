
#include <LuaAide.h>

using namespace std;

int LuaAide::apply(lua_State*L)
{
    LuaStack Q(L);
    if (height(Q)==0) return 0;
    if (Q.typeat(-2)!=LuaType::TTABLE) return 0;
    if (Q.typeat(-1)!=LuaType::TFUNCTION) return 0;
    Q.swap();
    auto func=Q.index(-2);
    for (LuaIterator J(Q); next(J); ++J) Q<<LuaFuncValue(stackindex(func))<<LuaValue(-2)>>0;
    return 0;
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

class ApplyEnv: public ::testing::Test
{
protected:
    LuaStack Q{};
    void SetUp() override { Q=LuaStack::New(true, panichandler); }
    void TearDown() override { Q.Close(); }
};

TEST_F(ApplyEnv, SimpleExample)
{
    Q<<0>>LuaGlobal("Akku");
    Q<<LuaCode("return function(x) Akku=Akku+x; end")>>1;
    auto func=Q.index(-1);
    Q<<lualist<<21<<22<<23<<lualistend;
    Q<<LuaAide::apply<<LuaValue(-2)<<func>>0;
    Q<<LuaGlobal("Akku");
    ASSERT_TRUE(Q.hasintat(-1));
    ASSERT_EQ(66, Q.toint(-1));
}

#endif
