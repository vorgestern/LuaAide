
#include <LuaAide.h>
#include <iostream>
#include <cstring>

using namespace std;

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

static void myhandleload(LuaStack&LS, int rc1, const char tag[])
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
    const int ns=height(*this);
    if (ns>0) drop(ns);
    return*this;
}

LuaStack&LuaStack::drop(unsigned num)
{
    const int nd=(int)num;
    const int ns=height(*this);
    lua_pop(L, nd>ns?ns:nd);
    return*this;
}

LuaStack&LuaStack::swap()
{
    lua_pushvalue(L, -2);
    lua_remove(L, -3);
    return*this;
}

bool LuaStack::dofile(const char filename[], int argc, char*argv[])
{
    if (const int rc=luaL_loadfile(L, filename); rc==LUA_OK) return mypcall(*this, argc, argv, filename);
    else return myhandleload(*this, rc, filename), false;
}

bool LuaStack::dostring(const char code[], int argc, char*argv[], const char tag[])
{
    if (const int rc=luaL_loadstring(L, code); rc==LUA_OK) return mypcall(*this, argc, argv, tag);
    else return myhandleload(*this, rc, tag), false;
}

// **********************************************************************

static int errfunction(lua_State*L)
{
    LuaStack Q(L);
    Q<<LuaUpValue(1)>>luaerror;
    return 0;
}

LuaCall LuaStack::operator<<(const LuaColonCall&C)
{
    // Beispiel: Es soll der Aufruf X:Funktion(a, b, c); ausgeführt werden.
    //           C.name="Funktion"
    //           C.numargs=3
    // Auf dem Stack liegt zu Beginn: [X, a, b, c]
    const auto object=index(-1-C.numargs);
    lua_getfield(L, stackindex(object), C.name); // [X, a, b, c, Funktion]
    if (hasfunctionat(-1))
    {
        lua_insert(L, stackindex(object)); // [Funktion, X, a, b, c]
        // Dieser Konstruktor ist uns durch die Freundschaftsbeziehung zugänglich.
        // Am Index object liegt inzwischen die Funktion.
        return LuaCall(L, object);
    }
    else
    {
        char pad[100];
        snprintf(pad, sizeof(pad), "%s is not a method but ", C.name);
        const auto str=pad+stringrepr(-1);
        drop(1);
        *this<<str<<LuaClosure(errfunction, 1);
        return LuaCall(L, index(-1));
    }
}

LuaCall LuaStack::operator<<(const LuaDotCall&C)
{
    const int objectindex=-1;
    lua_getfield(L, objectindex, C.name);
    lua_remove(L, objectindex-1);
    return LuaCall(L);
}

LuaCall LuaStack::operator<<(const LuaGlobalCall&C)
{
    *this<<LuaGlobal(C.name);
    return LuaCall(L);
}

LuaCall LuaStack::operator<<(const LuaCode&C)
{
    const int rc=luaL_loadstring(L, C.text);
    if (rc!=LUA_OK) lua_error(L);
    return LuaCall(L);
}

LuaCall LuaStack::operator<<(const pair<string_view, const LuaCode&>&X)
{
    auto [tag,C]=X;
    const int rc=luaL_loadbufferx(L, C.text, strlen(C.text), tag.data(), "t");
    if (rc!=LUA_OK) lua_error(L);
    return LuaCall(L);
}

LuaCall LuaStack::operator<<(lua_CFunction X)
{
    lua_pushcfunction(L, X);
    return LuaCall(L);
}

LuaCall LuaStack::operator<<(const LuaClosure&C)
{
    lua_pushcclosure(L, C.closure, C.num_upvalues);
    return LuaCall(L);
}

LuaStack&LuaStack::operator<<(const vector<string>&X)
{
    *this<<LuaArray(X.size());
    long n=0;
    for (auto&e: X)
    {
        lua_pushlstring(L, e.c_str(), e.size());
        lua_seti(L, -2, ++n);
    }
    return*this;
}

LuaStack&LuaStack::operator<<(const unordered_map<string,string>&X)
{
    *this<<LuaArray(X.size());
    for (auto&e: X)
    {
        const auto [k,v]=e;
        lua_pushlstring(L, v.c_str(), v.size());
        lua_setfield(L, -2, k.c_str());
    }
    return*this;
}

string LuaStack::stringrepr(int index)
{
    const auto t=lua_type(L, index);
    switch (t)
    {
        case LUA_TNIL: return "nil";
        case LUA_TBOOLEAN:{ const bool f=0!=lua_toboolean(L, index); return f?"true":"false"; }
        case LUA_TNUMBER:
        {
            const auto x=todouble(index);
            char pad[100];
            snprintf(pad, sizeof(pad), "%g", x);
            return pad;
        }
        case LUA_TSTRING:
        {
            const char*s=tostring(index);
            return s;
        }
        case LUA_TLIGHTUSERDATA:
        {
            const char*s=(char*)lua_touserdata(L, index);
            char pad[100];
            snprintf(pad, sizeof(pad), "lightuserdata(%p)", s);
            return pad;
        }
        case LUA_TUSERDATA:
        {
            const char*s=(char*)lua_touserdata(L, index);
            char pad[100];
            snprintf(pad, sizeof(pad), "userdata(%p)", s);
            return pad;
        }
        case LUA_TTABLE:
        {
            return "table {...}";
        }
        case LUA_TFUNCTION:
        {
            if (lua_iscfunction(L, index)) return "cfunction";
            else if (lua_isfunction(L, index)) return "lua function";
            else return "function(neither c nor lua)";
        }
        default:
        {
            char pad[100];
            snprintf(pad, sizeof(pad), "<undocumented type %d>", t);
            return pad;
        }
    }
}

string LuaStack::errormessage()
{
    if (height(*this)<1) return "No error message available (stack empty)";
    else if (!hasstringat(-1)) return "No error message available (not a string)";
    auto msg=tostring(-1);
    drop(1);
    return msg;
}

LuaStack&LuaStack::operator<<(const LuaLightUserData&X)
{
    lua_pushlightuserdata(L, const_cast<void*>(X.data));
    return*this;
}

LuaStack&LuaStack::operator<<(const LuaRegValue&X)
{
    *this<<LuaValue(LUA_REGISTRYINDEX)<<LuaLightUserData(X.data); // [Registry, key]
    lua_gettable(L, -2);                            // [Registry, [Registry[key]]
    lua_remove(L, -2);                              // [[Registry[key]]
    return*this;
}

LuaStack&LuaStack::operator>>(const LuaRegValue&X)
{
                                                    // [value]
    *this<<LuaValue(LUA_REGISTRYINDEX)
        <<LuaLightUserData(X.data)                  // [value, Registry, key]
        <<luarot_3;                                 // [Registry, key, value]
    lua_settable(L, -3);                            // [Registry]
    lua_remove(L, -1);                              // []
    return*this;
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

TEST_F(StackEnv, HeightInitial)
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

TEST_F(StackEnv, SwapNeu)
{
    Q<<21<<22<<luaswap;
    ASSERT_EQ(22, Q.toint(-2));
    ASSERT_EQ(21, Q.toint(-1));
}

TEST_F(StackEnv, Rot3)
{
    ASSERT_EQ(3, static_cast<int>(luarot3));
    ASSERT_EQ(-3, static_cast<int>(luarot_3));
    Q<<21<<22<<23<<luarot3;
    ASSERT_EQ(23, Q.toint(-3))<<Q;
    ASSERT_EQ(21, Q.toint(-2))<<Q;
    ASSERT_EQ(22, Q.toint(-1))<<Q;
    Q.clear();
    Q<<21<<22<<23<<luarot_3;
    ASSERT_EQ(22, Q.toint(-3))<<Q;
    ASSERT_EQ(23, Q.toint(-2))<<Q;
    ASSERT_EQ(21, Q.toint(-1))<<Q;
}

TEST_F(StackEnv, VectorString)
{
    string s4 {"String 5 enthält ein Nullbyte hier: \0 (hat geklappt)", 53}; // Die 53 schließt das abschließende Nullbyte.
    const vector<string>X={
        "Dies ist der erste String",
        "Dies ist der zweite String",
        "Dies ist der dritte String",
        "Ab vier sparsamer",
        s4,
        "String 6",
        "String 7"
    };
    // ASSERT_EQ(53, X[4].size());
    Q<<X;
    ASSERT_EQ(1, height(Q)); ASSERT_TRUE(Q.hastableat(-1));
    lua_geti(Q, -1, 5);
    ASSERT_EQ(2, height(Q)); ASSERT_TRUE(Q.hasstringat(-1));
    size_t len;
    const char*s=lua_tolstring(Q, -1, &len);
    ASSERT_NE(nullptr, s);
    ASSERT_EQ(53, len);
    ASSERT_STREQ("String 5 enthält ein Nullbyte hier: ", s);
    s=strchr(s, 0)+1;
    ASSERT_STREQ(" (hat geklappt)", s);
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
    Q<<-21.2<<false<<"abc"<<true<<luanil;
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

TEST_F(StackEnv, LuaAbsIndex)
{
    Q<<21<<22<<23<<"hoppla";
    const auto Hoppla=Q.index(-1);
    Q<<101<<102<<103;
    ASSERT_TRUE(Q.hasstringat(stackindex(Hoppla)));
    Q.drop(3);
    ASSERT_TRUE(Q.hasstringat(stackindex(Hoppla)));
    Q.drop(1)<<true<<"hoppla woanders";
    ASSERT_TRUE(Q.hasboolat(stackindex(Hoppla)));
}

TEST_F(StackEnv, LuaCode)
{
    Q<<LuaCode("return 21");
    ASSERT_TRUE(Q.hasfunctionat(-1));
    LuaCall(Q)>>1;
    ASSERT_TRUE(Q.hasintat(-1));
    ASSERT_EQ(21, Q.toint(-1));
}

TEST_F(StackEnv, LuaGlobal)
{
    Q<<LuaCode(R"xxx(a=21 b="hoppla")xxx")>>0;
    Q.clear();
    Q<<LuaGlobal("b")<<LuaGlobal("a");
    ASSERT_TRUE(Q.hasintat(-1));
    ASSERT_EQ(21, Q.toint(-1));
    ASSERT_TRUE(Q.hasstringat(-2));
    ASSERT_STREQ("hoppla", Q.tostring(-2));
}

TEST_F(StackEnv, LuaDotCall)
{
    Q<<LuaCode(R"xxx(
        A={
            demo=function(x) return string.format("x=%s", x) end
        }
    )xxx")>>0;
    Q.clear();
    Q<<LuaGlobal("A")<<LuaDotCall("demo")<<"alpha">>1;
    ASSERT_TRUE(Q.hasstringat(-1));
    ASSERT_STREQ("x=alpha", Q.tostring(-1));
}

TEST_F(StackEnv, LuaColonCall)
{
    Q<<LuaCode(R"xxx(
        local mt={
            demo=function(self, a) return string.format("x=%s, a=%s", self.x, a) end
        }
        mt.__index=mt
        A=setmetatable({x="alpha"}, mt)
    )xxx")>>0;
    Q.clear();
    Q<<LuaGlobal("A")<<"beta"<<LuaColonCall("demo", 1)>>1;
    ASSERT_TRUE(Q.hasstringat(-1));
    ASSERT_STREQ("x=alpha, a=beta", Q.tostring(-1));
}

TEST_F(StackEnv, LuaColonCallNotAMethod)
{
    Q<<LuaCode(R"xxx(
        local mt={
            demo=function(self, a) return string.format("x=%s, a=%s", self.x, a) end
        }
        mt.__index=mt
        A=setmetatable({x="alpha"}, mt)
    )xxx")>>0;
    Q.clear();
    Q<<LuaGlobal("A")<<"beta"<<LuaColonCall("demo_nixda", 1)>>1;
    ASSERT_TRUE(Q.hasstringat(-1));
    const string errmsg=Q.tostring(-1);
    ASSERT_TRUE(errmsg.starts_with("demo_nixda is not a method but nil"));
}

TEST_F(StackEnv, LuaRegValue)
{
    static const char modname[]="demo";
    ASSERT_EQ(0, height(Q));
    Q<<212223>>LuaRegValue(modname);
    ASSERT_EQ(0, height(Q));
    Q<<LuaRegValue(modname);
    ASSERT_EQ(1, height(Q));
    ASSERT_TRUE(Q.hasintat(-1));
    ASSERT_EQ(212223, Q.toint(-1));
}

// Teststatus LuaStack:
// ====================
// + version
// + height
// - <<(ostream)
// + clear
// + swap
// + drop
// + dup
// + index (LuaAbsIndex)
// - <<LuaSwap
// + <<LuaRotate
// - <<bool
// - <<int
// - <<unsigned
// - <<const char[]
// - <<float
// - <<double
// - <<vector<string>
// - <<LuaValue
// - <<LuaUpValue
// + <<LuaGlobal
// - <<LuaNil
// - <<LuaTable
// - <<LuaLightUserData
// - <<LuaClosure
// + <<LuaCode
// - <<lua_CFunction
// + <<LuaColonCall
// + <<LuaDotCall
// - <<LuaGlobalCall
// - <<LuaArray
// + <<LuaRegValue
//
// - >>LuaField
// - >>LuaGlobal
// - >>LuaError
//
// + posvalid
// + hasnilat
// + hasstringat
// + hasboolat
// + hasintat
// + hastableat
// + hasfunctionat
// - hasthreadat
// - hasuserdataat
//
// - tostring
// - tobool
// - toint
// - todouble
//
// - dofile
// - dostring
//
// - New
// - Close
// - stringrepr

// Teststatus LuaCall:
// ===================
// -

// Teststatus Sonstige:
// ====================
// - LuaIterator

#endif

#if 0
namespace {
    struct singlechunkreader_context
    {
        const char*chunk {nullptr};
        size_t chunklen {0};
        bool gelesen {false};
    };
    const char*singlechunkreader(lua_State*L, void*context, size_t*size)
    {
        auto cx=reinterpret_cast<singlechunkreader_context*>(context);
        if (cx->gelesen) return *size=0,nullptr;
        *size=cx->chunklen;
        cx->gelesen=true;
        return cx->chunk;
    }
}
LuaCall LuaStack::operator<<(const pair<string_view, const LuaCode&>&X)
{
    auto [tag,C]=X;
    singlechunkreader_context cx {C.text, strlen(C.text), false};
    const int rc=lua_load(L, singlechunkreader, &cx, tag.data(), nullptr);
    if (rc!=LUA_OK) lua_error(L);
    return LuaCall(L);
}
#endif
