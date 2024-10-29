
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
    const int top0=lua_gettop(L);
    const auto now=index(-1);
    const int numargs=stackindex(now)>=stackindex(funcindex)?stackindex(now)-stackindex(funcindex):0;
    lua_pushcfunction(L, TracebackAdder);             // [func, args[numargs], errorhandler]
    const int idx=-(numargs+2);
    lua_rotate(L, idx, 1);                            // [errorhandler, func, args[numargs]]
    const int rc=lua_pcall(L, numargs, numresults, idx);
    const int top1=lua_gettop(L);
    switch (rc)
    {
        case LUA_OK:
        {
            const int results=top1-top0+(numargs+1)-1;
            lua_remove(L, -results-1);
            break;
        }
        default:
        case LUA_ERRRUN:
        case LUA_ERRMEM:  // memory allocation error. For such errors, Lua does not call the message handler
        case LUA_ERRERR:  // error while running the message handler
        // case LUA_ERRGCMM: // error while running garbage collection metamethod (__gc)
        // Dieser Fehlercode wurde in Lua 5.4 entfernt.
        // Vgl. http://www.lua.org/manual/5.4/manual.html#8.3
        {
            lua_remove(L, -2);
            break;
        }
    }
    return rc;
}

int LuaCall::operator>>(std::pair<int,int>X)
{
    const int numtodrop=X.first, numresults=X.second;
    const int top0=height(*this);
    const auto now=index(-1);
    const int numargs=stackindex(now)>=stackindex(funcindex)?stackindex(now)-stackindex(funcindex):0;
    lua_pushcfunction(L, TracebackAdder);                           // [other[numtodrop], func, args[numargs], errorhandler]
    const int idx=-(numargs+2);
    lua_rotate(L, idx, 1);                                          // [other[numtodrop], errorhandler, func, args[numargs]]
    const int rc=lua_pcall(L, numargs, numresults, idx);
    const int top1=height(*this);
    switch (rc)
    {
        case LUA_OK:                                                // [other[numtodrop], errorhandler, results[numresuls]]
        {
            const auto results=top1-top0+(numargs+1)-1;
            lua_remove(L, -results-1);                              // [other[numtodrop], results[numresults]]
            lua_rotate(L, -(numresults+numtodrop), numresults);     // [results[numresults], other[numtodrop]]
            lua_pop(L, numtodrop);                                  // [results[numresults]]
            break;
        }
        default:
        case LUA_ERRRUN:
        case LUA_ERRMEM:  // memory allocation error. For such errors, Lua does not call the message handler
        case LUA_ERRERR:  // error while running the message handler
        // case LUA_ERRGCMM: // error while running garbage collection metamethod (__gc)
        // Dieser Fehlercode wurde in Lua 5.4 entfernt.
        // Vgl. http://www.lua.org/manual/5.4/manual.html#8.3
        {
                                                // [other[numtodrop], errorhandler, errorobject]
            lua_rotate(L, -(numtodrop+2), 1);   // [errorobject, other[numtodrop], errorhandler]
            lua_pop(L, numtodrop+1);            // [errorobject]
            break;
        }
    }
    return rc;
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
    else Q<<nil;
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
    else Q<<nil;
    Q<<"hoppla";
    Q<<"demoerror">>luaerror;
    return 0;
}

TEST_F(CallEnv, CallInt)
{
                                                    ASSERT_EQ(0, height(Q));
    Q<<21<<22<<23;                                  ASSERT_EQ(3, height(Q));
    Q<<demofunc<<LuaValue(-2)>>2;                   ASSERT_EQ(5, height(Q));
    ASSERT_TRUE(Q.hasintat(-5) && Q.toint(-5)==21);
    ASSERT_TRUE(Q.hasintat(-4) && Q.toint(-4)==22);
    ASSERT_TRUE(Q.hasintat(-3) && Q.toint(-3)==23);
    ASSERT_TRUE(Q.hasintat(-2) && Q.toint(-2)==24);
    ASSERT_TRUE(Q.hasstringat(-1));
    ASSERT_STREQ("hoppla", Q.tostring(-1));
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
    ASSERT_STREQ("demoerror\nstack traceback:", Q.tostring(-1));
}

TEST_F(CallEnv, CallIntError2)
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
    ASSERT_STREQ("demoerror\nstack traceback:\n\t[string \"function gehtnicht(x) return demoerror(x+100)...\"]:1: in function 'gehtnicht'", Q.tostring(-1));
}

TEST_F(CallEnv, CallPair)
{
    ASSERT_EQ(0, height(Q));
    Q<<21<<22<<23; ASSERT_EQ(3, height(Q));
    Q<<demofunc<<LuaValue(-2)>>make_pair(3, 2); ASSERT_EQ(2, height(Q));
    ASSERT_TRUE(Q.hasintat(-2) && Q.toint(-2)==24);
    ASSERT_TRUE(Q.hasstringat(-1));
    ASSERT_STREQ("hoppla", Q.tostring(-1));
}

TEST_F(CallEnv, CallPairError)
{
                                                                ASSERT_EQ(0, height(Q));
    Q<<21<<22<<23;                                              ASSERT_EQ(3, height(Q));
    const auto rc=Q<<demoerror<<LuaValue(-2)>>make_pair(3, 2);  ASSERT_EQ(LUA_ERRRUN, rc);
                                                                ASSERT_EQ(1, height(Q));
    ASSERT_TRUE(Q.hasstringat(-1));
    ASSERT_STREQ("demoerror\nstack traceback:", Q.tostring(-1));
}

#endif
