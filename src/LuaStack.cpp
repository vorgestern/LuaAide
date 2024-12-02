
#include <LuaAide.h>
#include <iostream>
#include <cstring>
#include <iostream>

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

string_view tostring(LuaType t)
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

string_view tostring(LuaMetaMethod m)
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
        const auto str=pad+asstring(-1);
        drop(1);
        *this<<str<<LuaClosure(errfunction, 1);
        return LuaCall(L, index(-1));
    }
}

LuaCall LuaStack::operator<<(const LuaDotCall&C)
{
    const int objectindex=-1;
    lua_getfield(L, objectindex, C.name);
    remove(objectindex-1);
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
    if (rc!=LUA_OK) *this>>luaerror;
    return LuaCall(L);
}

LuaCall LuaStack::operator<<(const pair<string_view, const LuaCode&>&X)
{
    auto [tag,C]=X;
    const int rc=luaL_loadbufferx(L, C.text, strlen(C.text), tag.data(), "t");
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
    lua_pushcclosure(L, C.closure, C.num_upvalues);
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
    lua_pushlightuserdata(L, const_cast<void*>(X.data));
    return*this;
}

LuaStack&LuaStack::operator<<(const LuaRegValue&X)
{
    *this<<LuaValue(LUA_REGISTRYINDEX)<<LuaLightUserData(X.data); // [Registry, key]
    lua_gettable(L, -2);                            // [Registry, [Registry[key]]
    remove(-2);                                     // [[Registry[key]]
    return*this;
}

LuaStack&LuaStack::operator>>(const LuaRegValue&X)
{
                                                    // [value]
    *this<<LuaValue(LUA_REGISTRYINDEX)
        <<LuaLightUserData(X.data)                  // [value, Registry, key]
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
        if (hint.empty()) luaL_typeerror(L, index, ::tostring(t).data());
        else luaL_typeerror(L, index, hint.data());
    }
}

void LuaStack::argcheck(int index, const std::function<bool(LuaStack&, int index)>&cond, std::string_view hint)
{
    if (!cond(*this, index)) luaL_error(L, "%s", hint.data());
}

LuaCall LuaStack::operator[](LuaMetaMethod m)
{
    const auto t=static_cast<LuaType>(luaL_getmetafield(L, -1, ::tostring(m).data()));
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

TEST_F(StackEnv, HasThreadAt)
{
    lua_pushthread(Q);
    ASSERT_EQ(1, height(Q));
    ASSERT_TRUE(Q.hasthreadat(-1));
}

TEST_F(StackEnv, HasUserdataAt1)
{
    Q<<LuaLightUserData(0);
    ASSERT_EQ(1, height(Q));
    ASSERT_TRUE(Q.hasuserdataat(-1));
}

TEST_F(StackEnv, HasUserdataAt2)
{
    lua_newuserdatauv(Q, sizeof(void*), 0);
    ASSERT_EQ(1, height(Q));
    ASSERT_TRUE(Q.hasuserdataat(-1));
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
    ASSERT_EQ("hoppla", Q.tostring(-2));
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
    ASSERT_EQ("x=alpha", Q.tostring(-1));
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
    ASSERT_EQ("x=alpha, a=beta", Q.tostring(-1));
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

TEST_F(StackEnv, LuaElement)
{
    ASSERT_EQ(0, height(Q));
    Q<<vector<string> {"A", "B", "C", "D"};
    ASSERT_EQ(1, height(Q));
    Q<<LuaElement {{-1, 2}};
    ASSERT_TRUE(Q.hasstringat(-1));
    ASSERT_EQ("B", Q.tostring(-1));
    Q.drop(1);
    Q<<LuaElement {{-1, 4}};
    ASSERT_TRUE(Q.hasstringat(-1));
    ASSERT_EQ("D", Q.tostring(-1));
    Q.drop(1);
    Q<<LuaElement {{-1, 5}};
    ASSERT_TRUE(Q.hasnilat(-1));
    Q.drop(1);
    ASSERT_EQ(1, height(Q));
    ASSERT_TRUE(Q.hastableat(-1));
    ASSERT_EQ(LuaType::TSTRING, Q(LuaElement {{-1, 1}}));
    ASSERT_EQ(2, height(Q));
    ASSERT_TRUE(Q.hasstringat(-1));
    Q.drop(1);
}

TEST_F(StackEnv, LuaElementSet)
{
    ASSERT_EQ(0, height(Q));
    Q<<LuaArray(10);
    ASSERT_EQ(1, height(Q));
    Q   <<"hoppla">>LuaElement {{-2, 5}}
        <<21>>LuaElement {{-2, 1}}
        <<22>>LuaElement {{-2, 2}}
        <<23>>LuaElement {{-2, 3}};
    // [21,22,23,nil,"hoppla"]
    ASSERT_EQ(1, height(Q));

    lua_len(Q, -1);
    ASSERT_EQ(2, height(Q));
    ASSERT_EQ(5, Q.toint(-1));
    Q.drop(1);

    Q<<LuaElement {{-1, 3}};
    ASSERT_EQ(2, height(Q));
    ASSERT_EQ(LuaType::TNUMBER, Q.typeat(-1));
    ASSERT_EQ(23, Q.toint(-1));
    Q.drop(1);

    Q<<LuaElement {{-1, 5}};
    ASSERT_EQ(2, height(Q));
    ASSERT_EQ(LuaType::TSTRING, Q.typeat(-1));
    ASSERT_EQ("hoppla", Q.tostring(-1));
    Q.drop(1);
    ASSERT_EQ(1, height(Q));
}

TEST_F(StackEnv, LuaList)
{
    ASSERT_EQ(0, height(Q));
    Q<<lualist<<21<<22<<23;
    ASSERT_EQ(1, height(Q));
    ASSERT_EQ(LuaType::TTABLE, Q.typeat(-1));

    Q<<LuaElement {{-1, 2}};
    ASSERT_EQ(2, height(Q));
    ASSERT_EQ(LuaType::TNUMBER, Q.typeat(-1));
    ASSERT_EQ(22, Q.toint(-1));
}

TEST_F(StackEnv, LuaIterator)
{
    Q<<lualist<<121<<122<<123<<124<<125;

    for (LuaIterator J(Q); next(J); ++J)
    {
        auto j=(unsigned)J;
        ASSERT_EQ(120+j, Q.toint(-1));
    }

    ASSERT_EQ(1, height(Q));
    ASSERT_EQ(LuaType::TTABLE, Q.typeat(-1));
    Q<<LuaGlobal("table")<<LuaDotCall("concat")<<LuaValue(-2)<<",">>1;
    ASSERT_EQ(LuaType::TSTRING, Q.typeat(-1));
    ASSERT_EQ("121,122,123,124,125", Q.tostring(-1));
}

TEST_F(StackEnv, AsString)
{
    Q<<true;
    ASSERT_EQ(LuaType::TBOOLEAN, Q.typeat(-1))<<Q;
    ASSERT_EQ("true", Q.asstring(-1))<<Q;
    Q.drop(1);

    void*p=nullptr;

    Q<<lua_error;
    ASSERT_EQ(LuaType::TFUNCTION, Q.typeat(-1));
    ASSERT_EQ(1, sscanf(Q.asstring(-1).c_str(), "cfunction(%p)", &p))<<Q;
    Q.drop(1);

    Q<<LuaLightUserData((void*)lua_error);
    ASSERT_EQ(LuaType::TLIGHTUSERDATA, Q.typeat(-1));
    ASSERT_EQ(1, sscanf(Q.asstring(-1).c_str(), "lightuserdata(%p)", &p))<<Q;
    Q.drop(1);

    Q<<luanil;
    ASSERT_EQ(LuaType::TNIL, Q.typeat(-1))<<Q;
    ASSERT_EQ("nil", Q.asstring(-1))<<Q;
    Q.drop(1);

    Q<<3.1415926;
    ASSERT_EQ(LuaType::TNUMBER, Q.typeat(-1))<<Q;
    ASSERT_EQ("3.14159", Q.asstring(-1))<<Q;
    Q.drop(1);

    Q<<"hoppla";
    ASSERT_EQ(LuaType::TSTRING, Q.typeat(-1))<<Q;
    ASSERT_EQ("hoppla", Q.asstring(-1))<<Q;
    Q.drop(1);

    Q<<newtable;
    ASSERT_EQ(LuaType::TTABLE, Q.typeat(-1))<<Q;
    ASSERT_EQ(1, sscanf(Q.asstring(-1).c_str(), "table(%p)", &p))<<Q;
    Q.drop(1);

    lua_pushthread(Q);
    ASSERT_EQ(LuaType::TTHREAD, Q.typeat(-1))<<Q;
    ASSERT_EQ(1, sscanf(Q.asstring(-1).c_str(), "thread(%p)", &p))<<Q;
    Q.drop(1);

    lua_newuserdatauv(Q, sizeof(void*), 0);
    ASSERT_EQ(LuaType::TUSERDATA, Q.typeat(-1))<<Q;
    ASSERT_EQ(1, sscanf(Q.asstring(-1).c_str(), "userdata(%p)", &p))<<Q;
    Q.drop(1);
}

TEST_F(StackEnv, ArgCheckTypeThrow)
{
    Q<<"abc";
    EXPECT_THROW(Q.argcheck(-1, LuaType::TFUNCTION, "function"), std::runtime_error);
}

TEST_F(StackEnv, ArgCheckTypeNoThrow)
{
    Q<<"abc";
    EXPECT_NO_THROW(Q.argcheck(-1, LuaType::TSTRING, {}));
}

TEST_F(StackEnv, ArgCheckCondThrow)
{
    Q<<"abc";
    auto cond=[](LuaStack&, int)->bool{ return false; };
    EXPECT_THROW(Q.argcheck(-1, cond, "fail whenever"), std::runtime_error);
    // EXPECT_NO_THROW(Q.argcheck(-1, cond, "Das kann nicht klappen."));
}

TEST_F(StackEnv, ArgCheckCondNoThrow)
{
    Q<<"abc";
    auto cond=[](LuaStack&, int)->bool{ return true; };
    EXPECT_NO_THROW(Q.argcheck(-1, cond, "success guaranteed"));
}

TEST_F(StackEnv, FieldAssignment)
{
    ASSERT_EQ(0, height(Q));
    Q<<newtable<<"alpha">=21;
    Q<<"beta">=22;
    Q<<"gamma">=23;
    ASSERT_EQ(1, height(Q))<<Q;
    Q<<LuaCode("x=...; return x.alpha, x.beta, x.gamma")<<LuaValue(-2)>>3;
    ASSERT_TRUE(Q.hasintat(-3))<<Q; ASSERT_EQ(21, Q.toint(-3));
    ASSERT_TRUE(Q.hasintat(-2))<<Q; ASSERT_EQ(22, Q.toint(-2));
    ASSERT_TRUE(Q.hasintat(-1))<<Q; ASSERT_EQ(23, Q.toint(-1));
}

TEST_F(StackEnv, FieldAssignmentF)
{
    ASSERT_EQ(0, height(Q));
    Q<<newtable; Q.F("alpha",21).F("beta",22).F("gamma",23);
    ASSERT_EQ(1, height(Q))<<Q;
    Q<<LuaCode("x=...; return x.alpha, x.beta, x.gamma")<<LuaValue(-2)>>3;
    ASSERT_TRUE(Q.hasintat(-3))<<Q; ASSERT_EQ(21, Q.toint(-3));
    ASSERT_TRUE(Q.hasintat(-2))<<Q; ASSERT_EQ(22, Q.toint(-2));
    ASSERT_TRUE(Q.hasintat(-1))<<Q; ASSERT_EQ(23, Q.toint(-1));
}

TEST_F(StackEnv, FieldAssignmentF1)
{
    ASSERT_EQ(0, height(Q));
    LuaLightUserData luvkey((void*)0x12345678);
    Q<<newtable; Q.F("alpha",21).F("beta",22).F(luvkey,23);
    ASSERT_EQ(1, height(Q))<<Q;
    Q<<LuaCode("X,luvkey=...; return X.alpha, X.beta, X[luvkey]")<<LuaValue(-2)<<luvkey>>3;
    ASSERT_TRUE(Q.hasintat(-3))<<Q; ASSERT_EQ(21, Q.toint(-3));
    ASSERT_TRUE(Q.hasintat(-2))<<Q; ASSERT_EQ(22, Q.toint(-2));
    ASSERT_TRUE(Q.hasintat(-1))<<Q; ASSERT_EQ(23, Q.toint(-1));
}

TEST(LuaType, ToString)
{
    EXPECT_EQ("none", tostring(LuaType::TNONE));
    EXPECT_EQ("nil", tostring(LuaType::TNIL));
    EXPECT_EQ("boolean", tostring(LuaType::TBOOLEAN));
    EXPECT_EQ("lightuserdata", tostring(LuaType::TLIGHTUSERDATA));
    EXPECT_EQ("number", tostring(LuaType::TNUMBER));
    EXPECT_EQ("string", tostring(LuaType::TSTRING));
    EXPECT_EQ("table", tostring(LuaType::TTABLE));
    EXPECT_EQ("function", tostring(LuaType::TFUNCTION));
    EXPECT_EQ("userdata", tostring(LuaType::TUSERDATA));
    EXPECT_EQ("thread", tostring(LuaType::TTHREAD));
    EXPECT_EQ("none", tostring(static_cast<LuaType>(-2)));
    EXPECT_EQ("none", tostring(static_cast<LuaType>(9)));
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
// + <<LuaColonCall
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
// + hasnilat
// + hasstringat
// + hasboolat
// + hasintat
// + hastableat
// + hasfunctionat
// + hasthreadat
// + hasuserdataat
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
