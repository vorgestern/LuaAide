
#include <LuaAide.h>
#include <iostream>

static void printval(lua_State*Q, int a, unsigned level=0)
{
    switch (lua_type(Q, a))
    {
        case LUA_TNIL:
        {
            fprintf(stdout, "nil");
            break;
        }
        case LUA_TBOOLEAN:
        {
            const bool f=0!=lua_toboolean(Q, a);
            fprintf(stdout, "%s", f?"true":"false");
            break;
        }
        case LUA_TNUMBER:
        {
            const double x=lua_tonumber(Q, a);
            fprintf(stdout, "%g", x);
            break;
        }
        case LUA_TSTRING:
        {
            const char*s=(char*)lua_tostring(Q, a);
            fprintf(stdout, "\"%s\"", s);
            break;
        }
        case LUA_TLIGHTUSERDATA:
        {
            const char*s=(char*)lua_touserdata(Q, a);
            fprintf(stdout, "lightuserdata(%p)", s);
            break;
        }
        case LUA_TUSERDATA:
        {
            const char*s=(char*)lua_touserdata(Q, a);
            fprintf(stdout, "userdata(%p)", s);
            break;
        }
        case LUA_TTABLE:
        {
            fprintf(stdout, "table");
            lua_pushnil(Q); // first key
            const int t=a-1;
            while (lua_next(Q, t)!=0)
            {
                // uses 'key' (at index -2) and 'value' (at index -1)
                if (lua_isnumber(Q, -2))
                {
                    const auto key=lua_tointeger(Q, -2);
                    //        printf("\n%*s[%d] is a %s:", 4*(level+1), "", key, lua_typename(Q, lua_type(Q, -1)));
                    printf("\n%*s[%lld] ", 4*(level+1), "", key);
                    printval(Q, -1, level+1);
                }
                else if (lua_isstring(Q, -2))
                {
                    const char*key=(char*)lua_tolstring(Q, -2, nullptr);
                    printf("\n%*s.%s is a %s (recursion stops)", 4*(level+1), "",
                           key, // lua_typename(Q, lua_type(Q, -2)),
                           lua_typename(Q, lua_type(Q, -1)));
                }
                // removes 'value'; keeps 'key' for next iteration
                lua_pop(Q, 1);
            }
            break;
        }
        case LUA_TFUNCTION:
        {
            if (lua_iscfunction(Q, a))
            {
                const auto cf=lua_tocfunction(Q, a);
                if (cf)   fprintf(stdout, "cfunction %p", cf);
                else fprintf(stdout, "cfunction (failed to get pointer)");
            }
            else if (lua_isfunction(Q, a))   fprintf(stdout, "lua function");
            break;
        }
        default:
        {
            fprintf(stdout, "<other type %d>", lua_type(Q, a));
            break;
        }
    }
}

static bool mypcall(LuaStack&LS, int argc, char*argv[], const char tag[])
{
    bool flag=false;
    for (int a=0; a<argc; ++a) LS<<argv[a];
    const int rc2=lua_pcall(LS, argc, LUA_MULTRET, 0);
    switch (rc2)
    {
        case LUA_OK: flag=true; break;
        case LUA_ERRRUN: std::cerr<<'\n'<<tag<<": Runtime error (LUA_ERRRUN):\n"<<LuaStackItem(LS, -1)<<'\n'; break;
        case LUA_ERRMEM: std::cerr<<'\n'<<tag<<": Memory allocation error (LUA_ERRMEM):\n"<<LuaStackItem(LS, -1)<<'\n'; break; // For such errors, Lua does not call the message handler.
        case LUA_ERRERR: std::cerr<<'\n'<<tag<<": Error handling error (LUA_ERRERR):\n"<<LuaStackItem(LS, -1)<<'\n'; break; // Error while running the message handler.

        // case LUA_ERRGCMM: std::cerr<<'\n'<<tag<<": Garbage collection error (LUA_ERRGCMM):\n"<<LuaStackItem(LS,-1)<<'\n'; break; // Error while running a __gc metamethod.
        // Dieser Fehlercode wurde in Lua 5.4 entfernt.
        // Vgl. http://www.lua.org/manual/5.4/manual.html#8.3
        // For such errors, Lua does not call the message handler.
        // (as this kind of error typically has no relation with the function being called).
        default: std::cerr<<'\n'<<tag<<": Unknown error ("<<rc2<<"):\n"<<LuaStackItem(LS, -1)<<'\n'; break;
    }
    return flag;
}

void myhandleload(LuaStack&LS, int rc1, const char tag[])
{
    if (rc1==LUA_OK)
    {
        // Dieses Ergebnis wird hier nicht behandelt.
    }
    else
    {
        std::cerr<<'\n'<<tag<<": ";
        switch (rc1)
        {
            case LUA_ERRSYNTAX: std::cerr<<"Syntax Error (LUA_ERRSYNTAX): "<<LS.tostring(-1)<<'\n'; break;
            case LUA_ERRMEM:    std::cerr<<"Out of memory (LUA_ERRMEM): "<<LS.tostring(-1)<<'\n'; break;
            // case LUA_ERRGCMM:   std::cerr<<"Garbage collection error (LUA_ERRGCMM): "<<LS.tostring(-1)<<'\n'; break;
            // Dieser Fehlercode wurde in Lua 5.4 entfernt.
            // Vgl. http://www.lua.org/manual/5.4/manual.html#8.3
            // For file-related errors (e.g., it cannot open or read the file):
            case LUA_ERRFILE:   std::cerr<<"File error (LUA_ERRFILE): "<<LS.tostring(-1)<<'\n'; break;
            default:            std::cerr<<"Unknown error (loadfile() returns "<<rc1<<"): "<<LS.tostring(-1)<<'\n'; break;
        }
    }
}

unsigned version(const LuaStack&S)
{
    return static_cast<unsigned>(lua_version(S.L));
}

LuaStack&LuaStack::clear()
{
    const int ns=lua_gettop(L);
    if (ns>0) lua_pop(L, ns);
    return *this;
}
LuaStack&LuaStack::drop(unsigned num)
{
    const int nd=(int)num;
    const int ns=lua_gettop(L);
    lua_pop(L, nd>ns?ns:nd);
    return*this;
}

LuaStack&LuaStack::swap()
{
    lua_pushvalue(L, -2);
    lua_remove(L, -3);
    return *this;
}

bool LuaStack::dofile(const char filename[], int argc, char*argv[])
{
    bool flag=false;
    const int rc1=luaL_loadfile(L, filename);
    switch (rc1)
    {
        case LUA_OK:
        {
            flag=mypcall(*this, argc, argv, filename);
            break;
        }
        default:
        {
            myhandleload(*this, rc1, filename);
            break;
        }
    }
    return flag;
}

bool LuaStack::dostring(const char code[], int argc, char*argv[], const char tag[])
{
    bool flag=false;
    const int rc1=luaL_loadstring(L, code);
    switch (rc1)
    {
        case LUA_OK:
        {
            flag=mypcall(*this, argc, argv, tag);
            break;
        }
        default:
        {
            myhandleload(*this, rc1, tag);
            break;
        }
    }
    return flag;
}

// **********************************************************************

LuaCall operator<<(LuaStack&S, LuaColonCall&C)
{
    // Beispiel: Es soll der Aufruf X:Funktion(a, b, c); ausgeführt werden.
    //           C.name="Funktion"
    //           C.numargs=3
    // Auf dem Stack liegt zu Beginn: [X, a, b, c]
    const int objectindex=-1-C.numargs;     // -4 zeigt auf X
    // Ermittle die Elementfunktion X[name].
    lua_getfield(S.L, objectindex, C.name); // [X, a, b, c, Funktion]
    if (!lua_isnil(S.L, -1))
    {
        lua_insert(S.L, -1+objectindex); // [Funktion, X, a, b, c]
    }
    else
    {
        // Dies führt trotzdem zum Absturz, weil der Operator>> nichts davon erfährt.
        // TODO: Behebe dies.
        printf("ColonCall: Object has no field '%s' (function required).\n", C.name);
        lua_pop(S.L, 1);
    }
    return LuaCall(S.L, 1+C.numargs); // Dieser Konstruktor ist uns durch die Freundschaftsbeziehung zugänglich.
}

LuaCall operator<<(LuaStack&S, LuaDotCall&C)
{
    const int objectindex=-1;
    lua_getfield(S.L, objectindex, C.name);
    lua_remove(S.L, objectindex-1);
    return LuaCall(S);
}

LuaCall operator<<(LuaStack&S, LuaGlobalCall&C)
{
    S<<LuaGlobal(C.name);
    return LuaCall(S);
}

LuaCall LuaStack::operator<<(const LuaChunk&X)
{
    const int rc=luaL_loadbufferx(L, X.buffer, X.bufferlength, X.buffername, "bt");
    if (rc!=LUA_OK) lua_error(L);
    return LuaCall(L);
}

LuaCall operator<<(LuaStack&S, const LuaCode&C)
{
    const int rc=luaL_loadstring(S, C.text);
    if (rc!=LUA_OK) lua_error(S);
    return LuaCall(S);
}

LuaCall LuaStack::operator<<(lua_CFunction X)
{
    lua_pushcfunction(L, X);
    return LuaCall(L);
}

// *******************************************************

LuaStack&operator<<(LuaStack&S, const LuaClosure&C)
{
    lua_pushcclosure(S, C.closure, C.num_upvalues);
    return S;
}

LuaStack&operator<<(LuaStack&S, const LuaUpValue&V)
{
    const int stackindex=lua_upvalueindex(V.index);
    lua_pushvalue(S, stackindex);
    return S;
}

// ============================================================================

#ifdef UNITTEST
#include <gtest/gtest.h>

class StackEnv: public ::testing::Test
{
protected:
    LuaStack Q{};
    void SetUp() override { Q=LuaStack::New(true, nullptr); }
    void TearDown() override { Q.Close(); }
};

TEST_F(StackEnv, Version)
{
    ASSERT_EQ(504, version(Q));
}

TEST_F(StackEnv, InitialHeight)
{
    ASSERT_EQ(0, height(Q));
}

TEST_F(StackEnv, Height)
{
    Q<<21<<"Hoppla";
    ASSERT_EQ(2, height(Q));
}

TEST_F(StackEnv, Drop)
{
    Q<<21<<"Hoppla";
    ASSERT_EQ(2, height(Q));
    Q.drop(1);
    ASSERT_EQ(1, height(Q));
    Q.drop(1);
    ASSERT_EQ(0, height(Q));
    ASSERT_NO_THROW(Q.drop(1));
    ASSERT_EQ(0, height(Q));
}

TEST_F(StackEnv, Swap)
{
    Q<<21<<22;
    ASSERT_EQ(21, Q.toint(-2));
    ASSERT_EQ(22, Q.toint(-1));
    Q.swap();
    ASSERT_EQ(22, Q.toint(-2));
    ASSERT_EQ(21, Q.toint(-1));
}

TEST_F(StackEnv, Dup)
{
    Q<<21;
    ASSERT_EQ(1, height(Q));
    ASSERT_EQ(21, Q.toint(-1));
    Q.dup();
    ASSERT_EQ(2, height(Q));
    ASSERT_EQ(21, Q.toint(-1));
    ASSERT_EQ(21, Q.toint(-2));
}

TEST_F(StackEnv, DupSecond)
{
    Q<<21<<22;
    Q.dup(-2);
    ASSERT_EQ(3, height(Q));
    ASSERT_EQ(21, Q.toint(-3));
    ASSERT_EQ(22, Q.toint(-2));
    ASSERT_EQ(21, Q.toint(-1));
}

TEST_F(StackEnv, HasNilAt)
{
    Q<<21<<LuaNil()<<22;
    ASSERT_EQ(3, height(Q));
    ASSERT_FALSE(Q.hasnilat(-4));
    ASSERT_FALSE(Q.hasnilat(-3));
    ASSERT_TRUE( Q.hasnilat(-2));
    ASSERT_FALSE(Q.hasnilat(-1));
    ASSERT_FALSE(Q.hasnilat( 1));
    ASSERT_TRUE( Q.hasnilat( 2));
    ASSERT_FALSE(Q.hasnilat( 3));
    ASSERT_FALSE(Q.hasnilat( 4));
}

TEST_F(StackEnv, PosValid)
{
    Q<<21<<22<<23<<24;
    ASSERT_EQ(4, height(Q));
    EXPECT_FALSE(Q.posvalid(-5));
    EXPECT_TRUE (Q.posvalid(-4));
    EXPECT_TRUE (Q.posvalid(-3));
    EXPECT_TRUE (Q.posvalid(-2));
    EXPECT_TRUE (Q.posvalid(-1));
    EXPECT_FALSE(Q.posvalid( 0));
    EXPECT_TRUE (Q.posvalid( 1));
    EXPECT_TRUE (Q.posvalid( 2));
    EXPECT_TRUE (Q.posvalid( 3));
    EXPECT_TRUE (Q.posvalid( 4));
    EXPECT_FALSE(Q.posvalid( 5));
}

TEST_F(StackEnv, HasStringAt)
{
    // Beachte dass String und Number gegenseitig akzeptiert werden,
    // weil die Konvertierbarkeit gewährleistet ist.
    Q<<true<<"abc"<<true<<true;
    ASSERT_EQ(4, height(Q));
    EXPECT_FALSE(Q.hasstringat(-5));
    EXPECT_FALSE(Q.hasstringat(-4));
    EXPECT_TRUE( Q.hasstringat(-3));
    EXPECT_FALSE(Q.hasstringat(-2));
    EXPECT_FALSE(Q.hasstringat(-1));
    EXPECT_FALSE(Q.hasstringat( 0));
    EXPECT_FALSE(Q.hasstringat( 1));
    EXPECT_TRUE( Q.hasstringat( 2));
    EXPECT_FALSE(Q.hasstringat( 3));
    EXPECT_FALSE(Q.hasstringat( 4));
    EXPECT_FALSE(Q.hasstringat( 5));
}

TEST_F(StackEnv, HasBoolAt)
{
    Q<<-21.2<<false<<"abc"<<true<<nil;
    ASSERT_EQ(5, height(Q));
    EXPECT_FALSE(Q.hasboolat(-6));
    EXPECT_FALSE(Q.hasboolat(-5));
    EXPECT_TRUE (Q.hasboolat(-4));
    EXPECT_FALSE(Q.hasboolat(-3));
    EXPECT_TRUE (Q.hasboolat(-2));
    EXPECT_FALSE(Q.hasboolat(-1));
    EXPECT_FALSE(Q.hasboolat( 0));
    EXPECT_FALSE(Q.hasboolat( 1));
    EXPECT_TRUE (Q.hasboolat( 2));
    EXPECT_FALSE(Q.hasboolat( 3));
    EXPECT_TRUE (Q.hasboolat( 4));
    EXPECT_FALSE(Q.hasboolat( 5));
    EXPECT_FALSE(Q.hasboolat( 6));
}

TEST_F(StackEnv, HasIntAt)
{
    Q<<true<<21<<true<<true;
    ASSERT_EQ(4, height(Q));
    EXPECT_FALSE(Q.hasintat(-5));
    EXPECT_FALSE(Q.hasintat(-4));
    EXPECT_TRUE( Q.hasintat(-3));
    EXPECT_FALSE(Q.hasintat(-2));
    EXPECT_FALSE(Q.hasintat(-1));
    EXPECT_FALSE(Q.hasintat( 0));
    EXPECT_FALSE(Q.hasintat( 1));
    EXPECT_TRUE( Q.hasintat( 2));
    EXPECT_FALSE(Q.hasintat( 3));
    EXPECT_FALSE(Q.hasintat( 4));
    EXPECT_FALSE(Q.hasintat( 5));
}

TEST_F(StackEnv, HasTableAt)
{
    Q<<true<<LuaTable()<<true<<true;
    ASSERT_EQ(4, height(Q));
    EXPECT_FALSE(Q.hastableat(-5));
    EXPECT_FALSE(Q.hastableat(-4));
    EXPECT_TRUE( Q.hastableat(-3));
    EXPECT_FALSE(Q.hastableat(-2));
    EXPECT_FALSE(Q.hastableat(-1));
    EXPECT_FALSE(Q.hastableat( 0));
    EXPECT_FALSE(Q.hastableat( 1));
    EXPECT_TRUE( Q.hastableat( 2));
    EXPECT_FALSE(Q.hastableat( 3));
    EXPECT_FALSE(Q.hastableat( 4));
    EXPECT_FALSE(Q.hastableat( 5));
}

#if 0
static int dummyfunc(lua_State*){ return 0; }

TEST_F(StackEnv, HasFunctionAt)
{
    Q<<true<<dummyfunc<<true<<true;
    ASSERT_EQ(4, height(Q));
    EXPECT_FALSE(Q.hasfunctionat(-5));
    EXPECT_FALSE(Q.hasfunctionat(-4));
    EXPECT_TRUE( Q.hasfunctionat(-3));
    EXPECT_FALSE(Q.hasfunctionat(-2));
    EXPECT_FALSE(Q.hasfunctionat(-1));
    EXPECT_FALSE(Q.hasfunctionat( 0));
    EXPECT_FALSE(Q.hasfunctionat( 1));
    EXPECT_TRUE( Q.hasfunctionat( 2));
    EXPECT_FALSE(Q.hasfunctionat( 3));
    EXPECT_FALSE(Q.hasfunctionat( 4));
    EXPECT_FALSE(Q.hasfunctionat( 5));
}
#endif

#endif
