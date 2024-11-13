
#include <LuaAide.h>

using namespace std;

LuaCall::LuaCall(lua_State*L): LuaStack(L), funcindex(index(-1)){}
LuaCall::LuaCall(LuaStack&S): LuaStack(S), funcindex(index(-1)){}
LuaCall::LuaCall(lua_State*L, LuaAbsIndex func): LuaStack(L), funcindex(func){}

static int TracebackAdder(lua_State*Q)
{
    // Dieser ErrorHandler fügt der Fehlermeldung den Stacktrace hinzu.
    // https://stackoverflow.com/questions/63570555/how-do-i-improve-lua-internal-error-messages-to-include-line-numbers
    const char*msg=lua_tostring(Q, -1);
    luaL_traceback(Q, Q, msg, 2);
    lua_remove(Q, -2); // Remove error/"msg" from stack.
    return 1; // Traceback is returned.
}

int LuaCall::operator>>(int numresults)
{
    if (numresults<0) numresults=LUA_MULTRET;
    const auto here=index(-1);
    const auto numargs=max(stackindex(here)-stackindex(funcindex), 0);
    lua_pushcfunction(L, TracebackAdder);                               // [func, args[numargs], errorhandler]
    const auto msgh=index(-(numargs+2));
    rotate(stackindex(msgh), 1);                                        // [errorhandler, func, args[numargs]]

    const int rc=lua_pcall(L, numargs, numresults, stackindex(msgh));   // [errorhandler, results[numresults]] | [errorhandler, errorobject]
    switch (rc)
    {
        case LUA_OK:                                                    // [errorhandler, results[numresults]]
        {
            remove(stackindex(msgh));                                   // [results[numresults]]
            return rc;
        }
        default:
        case LUA_ERRRUN:
        case LUA_ERRMEM:  // memory allocation error. For such errors, Lua does not call the message handler
        case LUA_ERRERR:  // error while running the message handler
        // case LUA_ERRGCMM: // error while running garbage collection metamethod (__gc)
        // Dieser Fehlercode wurde in Lua 5.4 entfernt.
        // Vgl. http://www.lua.org/manual/5.4/manual.html#8.3
        {
                                                                        // [errorhandler, errorobject]
            remove(-2);                                                 // [errorobject]
            return rc;
        }
    }
}

int LuaCall::operator>>(std::pair<int,int>X)
{
    const int numtodrop=X.first, numresults=X.second;
    const auto now=index(-1);
    const auto numargs=stackindex(now)>=stackindex(funcindex)?stackindex(now)-stackindex(funcindex):0;
    const int msgh=-(numargs+2);
    lua_pushcfunction(L, TracebackAdder);                           // [other[numtodrop], func, args[numargs], messagehandler]
    rotate(msgh, 1);                                                // [other[numtodrop], messagehandler, func, args[numargs]]
    const int rc=lua_pcall(L, numargs, numresults, msgh);
    switch (rc)
    {
        case LUA_OK:                                                // [other[numtodrop], messagehandler, results[numresuls]]
        {
            remove(-1-numresults);                                  // [other[numtodrop], results[numresults]]
            rotate(-(numresults+numtodrop), numresults);            // [results[numresults], other[numtodrop]]
            drop(numtodrop);                                        // [results[numresults]]
            return rc;
        }
        default:
        case LUA_ERRRUN: // Laufzeitfehler
        case LUA_ERRMEM: // OutOfMemory
        case LUA_ERRERR: // Fehler im Messagehandler
        {
                                                // [other[numtodrop], messagehandler, errorobject]
            rotate(-(numtodrop+2), 1);          // [errorobject, other[numtodrop], messagehandler]
            drop(numtodrop+1);                  // [errorobject]
            return rc;
        }
    }
}

// ============================================================================

#ifdef UNITTEST
#include <gtest/gtest.h>

class CallEnv: public ::testing::Test
{
protected:
    LuaStack Q{};
    void SetUp() override { Q=LuaStack::New(true, nullptr); }
    void TearDown() override { Q.Close(); }
};

int demofunc(lua_State*L) // [n] ==> [n+1, "hoppla"]
{
    LuaStack Q(L);
    if (Q.hasintat(-1))
    {
        const int arg=Q.toint(-1);
        Q<<arg+1;
    }
    else Q<<luanil;
    Q<<"hoppla";
    return 2;
}

int demoerror(lua_State*L) // [n] ==> [n+1, "hoppla", e]
{
    LuaStack Q(L);
    if (Q.hasintat(-1))
    {
        const int arg=Q.toint(-1);
        Q<<arg+1;
    }
    else Q<<luanil;
    Q<<"hoppla";
    Q<<"demoerror">>luaerror;
    return 0;
}

TEST_F(CallEnv, CallInt)
{
                                                    ASSERT_EQ(0, height(Q));
    Q<<21<<22<<23;                                  ASSERT_EQ(3, height(Q));
    Q<<demofunc<<LuaValue(-2)>>2;                   ASSERT_EQ(5, height(Q))<<Q;
    ASSERT_TRUE(Q.hasintat(-5) && Q.toint(-5)==21)<<Q;
    ASSERT_TRUE(Q.hasintat(-4) && Q.toint(-4)==22)<<Q;
    ASSERT_TRUE(Q.hasintat(-3) && Q.toint(-3)==23)<<Q;
    ASSERT_TRUE(Q.hasintat(-2) && Q.toint(-2)==24)<<Q;
    ASSERT_TRUE(Q.hasstringat(-1))<<Q;
    ASSERT_EQ("hoppla", Q.tostring(-1))<<Q;
}

static int demo_multret(lua_State*L)
{
    LuaStack Q(L);
    Q<<121<<122<<123<<124<<125;
    return 5;
}
TEST_F(CallEnv, DISABLED_CallMultRet)
{
                                                    ASSERT_EQ(0, height(Q));
    Q<<demo_multret>>LUA_MULTRET;                   ASSERT_EQ(5, height(Q))<<Q;
    ASSERT_TRUE(Q.hasintat(-5) && Q.toint(-5)==121)<<Q;
    ASSERT_TRUE(Q.hasintat(-4) && Q.toint(-4)==122)<<Q;
    ASSERT_TRUE(Q.hasintat(-3) && Q.toint(-3)==123)<<Q;
    ASSERT_TRUE(Q.hasintat(-2) && Q.toint(-2)==124)<<Q;
    ASSERT_TRUE(Q.hasintat(-1) && Q.toint(-1)==125)<<Q;
}

TEST_F(CallEnv, CallIntError)
{
                                                    ASSERT_EQ(0, height(Q));
    Q<<21<<22<<23;                                  ASSERT_EQ(3, height(Q));
    const auto rc=Q<<demoerror<<LuaValue(-2)>>2;    ASSERT_EQ(LUA_ERRRUN, rc);
                                                    ASSERT_EQ(4, height(Q));
    ASSERT_TRUE(Q.hasintat(-4) && Q.toint(-4)==21);
    ASSERT_TRUE(Q.hasintat(-3) && Q.toint(-3)==22);
    ASSERT_TRUE(Q.hasintat(-2) && Q.toint(-2)==23);
    ASSERT_TRUE(Q.hasstringat(-1));
    ASSERT_EQ("demoerror\nstack traceback:", Q.tostring(-1));
}

TEST_F(CallEnv, CallIntErrorNonemptyStacktrace)
{
                                                                    ASSERT_EQ(0, height(Q));
    Q<<demoerror>>LuaGlobal("demoerror");
    Q<<LuaCode(R"xxx(function gehtnicht(x) return demoerror(x+100) end)xxx")>>0;
    Q<<21<<22<<23;                                                  ASSERT_EQ(3, height(Q));
    const auto rc=Q<<LuaGlobalCall("gehtnicht")<<LuaValue(-2)>>2;   ASSERT_EQ(LUA_ERRRUN, rc);
                                                                    ASSERT_EQ(4, height(Q));
    ASSERT_TRUE(Q.hasintat(-4) && Q.toint(-4)==21);
    ASSERT_TRUE(Q.hasintat(-3) && Q.toint(-3)==22);
    ASSERT_TRUE(Q.hasintat(-2) && Q.toint(-2)==23);
    ASSERT_TRUE(Q.hasstringat(-1));
    ASSERT_EQ("demoerror\nstack traceback:\n\t[string \"function gehtnicht(x) return demoerror(x+100)...\"]:1: in function 'gehtnicht'", Q.tostring(-1));
}

TEST_F(CallEnv, CallPairErrorNonemptyStacktrace)
{
                                                            ASSERT_EQ(0, height(Q));
    Q<<demoerror>>LuaGlobal("demoerror");                   ASSERT_EQ(0, height(Q));
    Q<<make_pair("myscript", LuaCode(R"xxx(
        function gehtnicht(x)
            return demoerror(x+100)
        end
    )xxx"))>>0;                                             ASSERT_EQ(0, height(Q));
    const auto rc=Q<<LuaGlobalCall("gehtnicht")<<21>>2;     ASSERT_EQ(LUA_ERRRUN, rc);
                                                            ASSERT_EQ(1, height(Q));
    ASSERT_TRUE(Q.hasstringat(-1));
    ASSERT_EQ("demoerror\nstack traceback:\n\t[string \"myscript\"]:3: in function 'gehtnicht'", Q.tostring(-1));
}

TEST_F(CallEnv, CallPair)
{
    ASSERT_EQ(0, height(Q));
    Q<<21<<22<<23; ASSERT_EQ(3, height(Q));
    Q<<demofunc<<LuaValue(-2)>>make_pair(3, 2); ASSERT_EQ(2, height(Q));
    ASSERT_TRUE(Q.hasintat(-2) && Q.toint(-2)==24);
    ASSERT_TRUE(Q.hasstringat(-1));
    ASSERT_EQ("hoppla", Q.tostring(-1));
}

TEST_F(CallEnv, CallPairError)
{
                                                                ASSERT_EQ(0, height(Q));
    Q<<21<<22<<23;                                              ASSERT_EQ(3, height(Q));
    const auto rc=Q<<demoerror<<LuaValue(-2)>>make_pair(3, 2);  ASSERT_EQ(LUA_ERRRUN, rc);
                                                                ASSERT_EQ(1, height(Q));
    ASSERT_TRUE(Q.hasstringat(-1));
    ASSERT_EQ("demoerror\nstack traceback:", Q.tostring(-1));
}

// =====================================================================

int demo_studie(lua_State*L)
{
    // []  ==> [nil, "hoppla"]
    // [n] ==> [n+1, "hoppla"]
    LuaStack Q(L);
    if (Q.hasintat(-1))
    {
        const int arg=Q.toint(-1);
        Q<<arg+1;
    }
    else Q<<luanil;
    Q<<"hoppla"<<"mehr";
    return 3;
}
TEST_F(CallEnv, Studie)
{
    Q<<demo_studie;
    const auto func=Q.index(-1);                                        EXPECT_EQ(1, stackindex(func));
    Q<<21<<22<<23;
    const auto here=Q.index(-1);                                        EXPECT_EQ(4, stackindex(here));
    const int numargs=stackindex(here)>=stackindex(func)?stackindex(here)-stackindex(func):0;
    EXPECT_EQ(3, numargs);
    cout<<"1 "<<Q<<"\n";

    lua_pushcfunction(Q, TracebackAdder);
    cout<<"2 "<<Q<<"\n";

    lua_rotate(Q, -(numargs+2), -(numargs+1));
    // const int msgh=-(numargs+2);
    const auto msgh=Q.index(-(numargs+2));
    cout<<"3 "<<Q<<" msgh="<<stackindex(msgh)<<"\n";

    const int rc=lua_pcall(Q, numargs, LUA_MULTRET, stackindex(msgh));
    cout<<"rc="<<rc<<" "<<Q<<"\n";

    lua_remove(Q, stackindex(msgh));
    cout<<"remove: "<<Q<<"\n";
}

#endif
