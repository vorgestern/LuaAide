
#include <LuaAide.h>
#include <iostream>
#include "helper.h"

using namespace std;
using namespace LuaAide;

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

unsigned LuaAide::version(const LuaStack&S)
{
    return static_cast<unsigned>(lua_version(S.L));
}

lua_State*LuaStack::New(bool defaultlibs, lua_CFunction errorhandler)
{
    auto*L=luaL_newstate();
    if (defaultlibs) luaL_openlibs(L);
    if (errorhandler!=nullptr) lua_atpanic(L, errorhandler);
    return L;
}

void LuaStack::Close()
{
    if (L!=nullptr)
    {
        lua_close(L);
        L=nullptr;
    }
}

bool LuaStack::check(int numpos){ return lua_checkstack(L, numpos); }

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
    remove(-3);
    return*this;
}

LuaStack&LuaStack::rotate(int wo, int num){ lua_rotate(L, wo, num); return*this; }

LuaStack&LuaStack::dup(int was){ lua_pushvalue(L, was); return*this; }

LuaStack&LuaStack::remove(int was){ lua_remove(L, was); return*this; }

LuaStack::absindex LuaStack::index(int n){ return absindex(lua_absindex(L, n)); }

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

LuaList LuaStack::operator<<(LuaListStart){ *this<<newtable; return LuaList(L); }

string LuaStack::tostring(int pos){ size_t len; const char*s=lua_tolstring(L, pos, &len); return {s, len}; }

string LuaStack::asstring(int pos)
{
    switch (typeat(pos))
    {
        case LuaType::TBOOLEAN: return lua_toboolean(L, pos)?"true":"false";
        case LuaType::TFUNCTION:
        {
            const bool native=lua_iscfunction(L, pos);
            char pad[100];
            const size_t len=snprintf(pad, sizeof(pad), "%sfunction(%p)", native?"c":"", lua_topointer(L, pos));
            return {pad, len};
        }
        case LuaType::TLIGHTUSERDATA:
        {
            char pad[100];
            const size_t len=snprintf(pad, sizeof(pad), "lightuserdata(%p)", lua_topointer(L, pos));
            return {pad, len};
        }
        case LuaType::TNIL: return "nil";
        case LuaType::TNUMBER:
        {
            char pad[100];
            snprintf(pad, sizeof(pad), "%g", lua_tonumber(L, pos));
            return pad;
        }
        case LuaType::TSTRING:
        {
            size_t len;
            const char*str=lua_tolstring(L, pos, &len);
            return {str, len};
        }
        case LuaType::TTABLE:
        {
            char pad[100];
            const size_t len=snprintf(pad, sizeof(pad), "table(%p)", lua_topointer(L, pos));
            return {pad, len};
        }
        case LuaType::TTHREAD:
        {
            char pad[100];
            const size_t len=snprintf(pad, sizeof(pad), "thread(%p)", lua_topointer(L, pos));
            return {pad, len};
        }
        case LuaType::TUSERDATA:
        {
            char pad[100];
            const size_t len=snprintf(pad, sizeof(pad), "userdata(%p)", lua_topointer(L, pos));
            return {pad, len};
        }
        default:
        case LuaType::TNONE: return "";
    }
}

string_view LuaAide::tostringview(LuaType t)
{
    static string_view names[]=
    {
        // TNONE=-1,
        "nil", // TNIL=0,
        "boolean", // TBOOLEAN,
        "lightuserdata", // TLIGHTUSERDATA,
        "number", // TNUMBER,
        "string", // TSTRING,
        "table", // TTABLE,
        "function", // TFUNCTION,
        "userdata", // TUSERDATA,
        "thread" // TTHREAD
    };
    if (t<=LuaType::TNONE) return "none";
    else if (t<=LuaType::TTHREAD) return names[static_cast<int>(t)];
    else return "none";
}

string_view LuaAide::tostringview(LuaMetaMethod m)
{
    static const string_view names[]=
    {
        "__tostring",            // tostring,
        "__add", "__sub", "__mul", "__div", "__mod", "__pow", // add, sub, mul, div, mod, pow,
        "__unm",                 // unm,
        "__band", "__bor", "__bxor", "__bnot", "__shl", "__shr", // band, bor, bxor, bnot, shl, shr,
        "__concat",              // concat,
        "__len",                 // len,
        "__eq", "__lt", "__le",  // eq, lt, le,
        "__index", "__newindex", // index, newindex,
        "__call",                // call,
        "__gc",                  // gc,
        "__close",               // close,
        "__mode",                // mode,
        "__name"                 // name
    };
    const auto index=static_cast<unsigned>(m);
    if (m<=LuaMetaMethod::name) return names[index];
    else return "none";
}

// *********************************************************************

[[maybe_unused]] static int errfunction(lua_State*L)
{
    LuaStack Q(L);
    // Q<<LuaUpValue(1)>>luaerror;
    Q<<LuaUpValue(1);
    lua_error(L);
    return 0;
}

LuaCall LuaStack::operator<<(const LuaMethod&C)
{
    const auto t=typeat(-1);
    switch (t)
    {
        case LuaType::TTABLE:
        case LuaType::TSTRING:
        {
            const auto t=(LuaType)lua_getfield(L, -1, C.name); // [X, field]
            switch (t)
            {
                case LuaType::TFUNCTION:
                {
                    swap(); // [function, X]
                    return LuaCall(L, index(-2));
                }
                case LuaType::TNIL:
                {
                    drop(1);
                    const auto tm=(LuaType)luaL_getmetafield(L, -1, C.name);
                    switch (tm)
                    {
                        case LuaType::TFUNCTION:
                        {
                            swap(); // [X.name, X]
                            return LuaCall(L, index(-2));
                        }
                        default:
                        {
                            drop(2);
                            char pad[1000];
                            auto tfs=tostringview(tm);
                            sprintf(pad, "Attempt to call a %s value (LuaMethod '%s').", tfs.data(), C.name);
                            *this<<pad;
                            break;
                        }
                    }
                    break;
                }
                default:
                {
                    // [X, field]
                    drop(2);
                    char pad[1000];
                    auto tfs=tostringview(t);
                    sprintf(pad, "Attempt to call a %s value (LuaMethod '%s').", tfs.data(), C.name);
                    *this<<pad;
                    break;
                }
            }
            break;
        }
        default:
        {
            // [X]
            if (lua_getmetatable(L, -1))
            {
                // [X, mt]
                const auto tf=(LuaType)lua_getfield(L, -1, C.name); // [X, mt, mt.name]
                switch (tf)
                {
                    case LuaType::TFUNCTION:
                    {
                        *this<<luarot3; // [mt.name, X, mt]
                        drop(1);        // [mt.name, X]
                        return LuaCall(L, index(-2));
                    }
                    default:
                    {
                        // [X, mt, mt.name]
                        drop(3);
                        char pad[1000];
                        const auto tfs=tostringview(tf);
                        sprintf(pad, "Attempt to index a %s value (LuaMethod '%s').", tfs.data(), C.name);
                        *this<<pad;
                    }
                }
            }
            else
            {
                // [X]
                drop(1);
                char pad[1000];
                const auto ts=tostringview(t);
                sprintf(pad, "Attempt to index a %s value (LuaMethod '%s').", ts.data(), C.name);
                *this<<pad;
            }
        }
    }

    // [errmsg]
    lua_error(L);
    return LuaCall(L, index(-1));
}

LuaCall LuaStack::operator<<(const LuaDotCall&C)
{
    const int objectindex=-1;
    lua_getfield(L, objectindex, C.value.data());
    remove(objectindex-1);
    return LuaCall(L);
}

LuaCall LuaStack::operator<<(const LuaGlobalCall&C)
{
    *this<<LuaGlobal(C.value.data());
    return LuaCall(L);
}

LuaCall LuaStack::operator<<(const LuaCode&C)
{
    const int rc=luaL_loadbufferx(L, C.value.data(), C.value.size(), C.value.data(), nullptr); // Use string as name, better than nothing.
    if (rc!=LUA_OK) *this>>luaerror;
    return LuaCall(L);
}

LuaCall LuaStack::operator<<(const LuaFuncValue F)
{
    *this<<LuaValue(F.value);
    auto a=index(-1);
    return LuaCall(L, a);
}

LuaCall LuaStack::operator<<(const pair<string_view, const LuaCode&>&X)
{
    auto [tag,C]=X;
    const int rc=luaL_loadbufferx(L, C.value.data(), C.value.size(), tag.data(), nullptr);
    if (rc!=LUA_OK) *this>>luaerror;
    return LuaCall(L);
}

LuaCall LuaStack::operator<<(lua_CFunction X)
{
    lua_pushcfunction(L, X);
    return LuaCall(L);
}

LuaCall LuaStack::operator<<(const LuaClosure&C)
{
    lua_pushcclosure(L, C.value.first, C.value.second);
    return LuaCall(L);
}

LuaStack&LuaStack::operator<<(const vector<string>&X)
{
    *this<<LuaArray {X.size()};
    long n=0;
    for (auto&e: X)
    {
        *this<<e;
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
        *this<<v;
        lua_setfield(L, -2, k.c_str());
    }
    return*this;
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
    lua_pushlightuserdata(L, const_cast<void*>(X.value));
    return*this;
}

LuaStack&LuaStack::operator<<(const LuaRegValue&X)
{
    *this<<LuaValue(LUA_REGISTRYINDEX)<<LuaLightUserData(X.value); // [Registry, key]
    lua_gettable(L, -2);                            // [Registry, [Registry[key]]
    remove(-2);                                     // [[Registry[key]]
    return*this;
}

LuaStack&LuaStack::operator>>(const LuaRegValue&X)
{
                                                    // [value]
    *this<<LuaValue(LUA_REGISTRYINDEX)
        <<LuaLightUserData(X.value)                 // [value, Registry, key]
        <<luarot_3;                                 // [Registry, key, value]
    lua_settable(L, -3);                            // [Registry]
    drop(1);                                        // []
    return*this;
}

LuaStack&LuaStack::operator<<(LuaRotate X)
{
    auto index=static_cast<int>(X);
    if (index<0) lua_rotate(L, index, -1);
    else if (index>0) lua_rotate(L, -index, 1);
    return*this;
}

LuaStack&LuaStack::operator<<(LuaSwap)
{
    lua_rotate(L, -2, 1);
    return*this;
}

void LuaStack::argcheck(int index, LuaType t, std::string_view hint)
{
    if (typeat(index)!=t)
    {
        if (hint.empty()) luaL_typeerror(L, index, tostringview(t).data());
        else luaL_typeerror(L, index, hint.data());
    }
}

void LuaStack::argcheck(int index, const std::function<bool(LuaStack&, int index)>&cond, std::string_view hint)
{
    if (!cond(*this, index)) luaL_error(L, "%s", hint.data());
}

LuaCall LuaStack::operator[](LuaMetaMethod m)
{
    const auto t=static_cast<LuaType>(luaL_getmetafield(L, -1, ::tostringview(m).data()));
    if (t==LuaType::TNIL)
    {
        // Hier sollte eine Dummymethode auf den Stack gelegt werden, die beim Aufruf eine Fehlermeldung erzeugt.
        // cout<<"Metamethod "<<::tostring(m)<<": TNIL\n";
    }
    swap();
    return LuaCall(L, index(-2));
}

// ============================================================================

#ifdef UNITTEST
#include <gtest/gtest.h>

bool nilat(LuaStack&Q, int index){ return Q.hasat(LuaType::TNIL, index); }
bool boolat(LuaStack&Q, int index){ return Q.hasat(LuaType::TBOOLEAN, index); }
bool funcat(LuaStack&Q, int index){ return Q.hasat(LuaType::TFUNCTION, index); }

static int panichandler(lua_State*L)
{
    LuaStack Q(L);
    throw runtime_error(Q.errormessage());
    return 0;
}

class StackEnv: public ::testing::Test
{
protected:
    LuaStack Q{};
    void SetUp() override { Q=LuaStack::New(true, panichandler); }
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
    ASSERT_EQ(LuaType::TSTRING, Q(LuaElement {{-1, 5}}));
    ASSERT_EQ(2, height(Q));
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
    ASSERT_FALSE(nilat(Q, -4));
    ASSERT_FALSE(nilat(Q, -3));
    ASSERT_TRUE( nilat(Q, -2));
    ASSERT_FALSE(nilat(Q, -1));
    ASSERT_FALSE(nilat(Q,  1));
    ASSERT_TRUE( nilat(Q,  2));
    ASSERT_FALSE(nilat(Q,  3));
    ASSERT_FALSE(nilat(Q,  4));
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
    EXPECT_FALSE(boolat(Q, -6));
    EXPECT_FALSE(boolat(Q, -5));
    EXPECT_TRUE (boolat(Q, -4));
    EXPECT_FALSE(boolat(Q, -3));
    EXPECT_TRUE (boolat(Q, -2));
    EXPECT_FALSE(boolat(Q, -1));
    EXPECT_FALSE(boolat(Q,  0));
    EXPECT_FALSE(boolat(Q,  1));
    EXPECT_TRUE (boolat(Q,  2));
    EXPECT_FALSE(boolat(Q,  3));
    EXPECT_TRUE (boolat(Q,  4));
    EXPECT_FALSE(boolat(Q,  5));
    EXPECT_FALSE(boolat(Q,  6));
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
    Q<<true<<newtable<<true<<true;
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
    EXPECT_FALSE(funcat(Q, -5));
    EXPECT_FALSE(funcat(Q, -4));
    EXPECT_TRUE( funcat(Q, -3));
    EXPECT_FALSE(funcat(Q, -2));
    EXPECT_FALSE(funcat(Q, -1));
    EXPECT_FALSE(funcat(Q,  0));
    EXPECT_FALSE(funcat(Q,  1));
    EXPECT_TRUE( funcat(Q,  2));
    EXPECT_FALSE(funcat(Q,  3));
    EXPECT_FALSE(funcat(Q,  4));
    EXPECT_FALSE(funcat(Q,  5));
}

TEST_F(StackEnv, HasThreadAt)
{
    lua_pushthread(Q);
    ASSERT_EQ(1, height(Q));
    ASSERT_TRUE(Q.hasat(LuaType::TTHREAD, -1));
}

TEST_F(StackEnv, HasLightUserdataAt1)
{
    Q<<LuaLightUserData(0);
    ASSERT_EQ(1, height(Q));
    ASSERT_TRUE(Q.hasat(LuaType::TLIGHTUSERDATA, -1));
}

TEST_F(StackEnv, HasUserdataAt2)
{
    lua_newuserdatauv(Q, sizeof(void*), 0);
    ASSERT_EQ(1, height(Q));
    ASSERT_TRUE(Q.hasat(LuaType::TUSERDATA, -1));
}

TEST_F(StackEnv, LuaStackAbsindex)
{
    Q<<21<<22<<23<<"hoppla";
    const auto Hoppla=Q.index(-1);
    Q<<101<<102<<103;
    ASSERT_TRUE(Q.hasstringat(stackindex(Hoppla)));
    Q.drop(3);
    ASSERT_TRUE(Q.hasstringat(stackindex(Hoppla)));
    Q.drop(1)<<true<<"hoppla woanders";
    ASSERT_TRUE(boolat(Q, stackindex(Hoppla)));
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
// + index (LuaStack::absindex)
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
// + <<LuaMethod
// + <<LuaDotCall
// - <<LuaGlobalCall
// - <<LuaArray
// + <<LuaRegValue
// + <<LuaElement
//
// - >>LuaField
// - >>LuaGlobal
// - >>LuaError
//
// + posvalid
// + hasstringat
// + hasintat
// + hastableat
//
// - tostring
// - tobool
// - toint
// - todouble
//
// + asstring
//
// - dofile
// - dostring
//
// - New
// - Close
// - stringrepr
//
// + argcheck

// Teststatus LuaCall:
// ===================
// -

// Teststatus LuaList:
// ===================
// + Konstrktor
// + push int

// Teststatus Sonstige:
// ====================
// + LuaIterator

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
    if (rc!=LUA_OK) *this>>luaerror;
    return LuaCall(L);
}
#endif
