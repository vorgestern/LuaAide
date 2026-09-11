
#include <gtest/gtest.h>
#include <LuaAide.h>
#include <iostream>
#include "helper.h"

using namespace std;
using namespace LuaAide;

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

static bool stringat(LuaStack&Q, int index){ return Q.hasat(LuaType::TSTRING, index); }

TEST_F(StackEnv, LuaCode)
{
    Q<<LuaCode("return 21");
    ASSERT_TRUE(Q.hasat(LuaType::TFUNCTION, -1));
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
    ASSERT_TRUE(stringat(Q, -2));
    ASSERT_EQ("hoppla", Q.tostring(-2));
}

TEST_F(StackEnv, LuaElementCall)
{
    Q<<LuaCode(R"xxx(
        A={
            demo=function(x) return string.format("x=%s", x) end
        }
    )xxx")>>0;
    Q.clear();
    Q<<LuaGlobal("A")<<LuaElementCall("demo")<<"alpha">>1;
    ASSERT_TRUE(stringat(Q, -1));
    ASSERT_EQ("x=alpha", Q.tostring(-1));
}

TEST_F(StackEnv, LuaElementCallByKey)
{
    Q.clear();
    Q<<LuaCode(R"___(
        A={
            demo=function(x) return string.format("x=%s", x) end
        }
    )___")>>0;
    Q<<LuaGlobal("A")<<"demo";
    const auto table=Q.index(-2);
    Q<<MyElementFunction(table)<<"beta">>1;
    ASSERT_TRUE(stringat(Q, -1));
    ASSERT_EQ("x=beta", Q.tostring(-1));
}

TEST_F(StackEnv, LuaFuncValue)
{
    Q.clear();
    // Define a function.
    Q<<LuaCode(R"___(
        return function(a) return tostring(a)..","..tostring(a) end
    )___")>>1;
    auto func=Q.index(-1);
    // Push some random things on the stack on top of it,
    // then call the function with argument 123.
    Q<<21<<22<<23<<LuaFuncValue(func)<<123>>1;
    ASSERT_EQ(LuaType::TSTRING, Q.typeat(-1));
    ASSERT_EQ("123,123", Q.tostring(-1));
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
    ASSERT_TRUE(stringat(Q, -1));
    ASSERT_EQ("B", Q.tostring(-1));
    Q.drop(1);
    Q<<LuaElement {{-1, 4}};
    ASSERT_TRUE(stringat(Q, -1));
    ASSERT_EQ("D", Q.tostring(-1));
    Q.drop(1);
    Q<<LuaElement {{-1, 5}};
    ASSERT_TRUE(Q.hasat(LuaType::TNIL, -1));
    Q.drop(1);
    ASSERT_EQ(1, height(Q));
    ASSERT_EQ(LuaType::TTABLE, Q.typeat(-1));
    ASSERT_EQ(LuaType::TSTRING, Q(LuaElement {{-1, 1}}));
    ASSERT_EQ(2, height(Q));
    ASSERT_TRUE(stringat(Q, -1));
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
    Q<<LuaGlobal("table")<<LuaElementCall("concat")<<LuaValue(-2)<<",">>1;
    ASSERT_EQ(LuaType::TSTRING, Q.typeat(-1));
    ASSERT_EQ("121,122,123,124,125", Q.tostring(-1));
}

TEST_F(StackEnv, LuaIteratorBreak)
{
    Q<<lualist<<121<<122<<123<<124<<125;

    for (LuaIterator J(Q); next(J); ++J)
    {
        auto j=(unsigned)J;
        if (j==2) break;
        ASSERT_EQ(120+j, Q.toint(-1));
    }

    ASSERT_EQ(1, height(Q))<<"Breaking out of the loop must take the looping variables off the stack.";
    ASSERT_EQ(LuaType::TTABLE, Q.typeat(-1));
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

// ====================================================================

TEST_F(StackEnv, MethodOfString)
{
    ASSERT_EQ(0, height(Q));
    Q<<"21 22 23"<<LuaMethod("match")<<"(%d+) (%d+) (%d+)">>3;
    ASSERT_EQ("21", Q.tostring(-3));
    ASSERT_EQ("22", Q.tostring(-2));
    ASSERT_EQ("23", Q.tostring(-1));
}

TEST_F(StackEnv, MethodOfStringDoesNotExist)
{
    ASSERT_EQ(0, height(Q));
    ASSERT_THROW(Q<<"21 22 23"<<LuaMethod("print")>>0, runtime_error)<<"Nach dem Aufruf:\n"<<Q<<"\n";
}

TEST_F(StackEnv, MethodDoesNotExist)
{
    ASSERT_EQ(0, height(Q));
    try {
        Q<<21<<LuaMethod("match")<<"(%d+) (%d+) (%d+)">>3;
    }
    catch (const runtime_error&E)
    {
        cout<<"runtime error: "<<E.what()<<"\n";
    }
    cout<<Q;
}

TEST_F(StackEnv, Method1)
{
    Q<<"21";
    const auto t=(LuaType)luaL_getmetafield(Q, -1, "match");
    cout<<"t="<<tostringview(t)<<"\n";
    cout<<Q;
}

TEST_F(StackEnv, Method2)
{
    Q<<"21";
    const auto t=(LuaType)lua_getfield(Q, -1, "print"); // Überraschenderweise funktioniert lua_getfield, aber nicht luaL_getmetafield bei strings.
    cout<<"t="<<tostringview(t)<<"\n";
    cout<<Q;
}

TEST_F(StackEnv, Method3)
{
    Q<<"21";
    if (lua_getmetatable(Q, -1))
    {
        const auto t3=(LuaType)lua_getfield(Q, -2, "match"); // <== Frage den value ab, nicht direkt die Metatabelle!
        cout<<"metatable.getfield(match)==>"<<tostringview(t3)<<"\n";
    }
    else cout<<"string has no metatable\n";
    // const auto t2=(LuaType)luaL_getmetafield(Q, -2, "match");
    // cout<<"t1="<<t1<<"\n";
    // cout<<"t2="<<tostringview(t2)<<"\n";
    cout<<Q;
}

TEST_F(StackEnv, Method4)
{
    Q<<21;
    const auto t=lua_getmetatable(Q, -1);
    cout<<"t="<<t<<"\n";
    cout<<Q;
}

TEST(LuaType, ToString)
{
    EXPECT_EQ("none", tostringview(LuaType::TNONE));
    EXPECT_EQ("nil", tostringview(LuaType::TNIL));
    EXPECT_EQ("boolean", tostringview(LuaType::TBOOLEAN));
    EXPECT_EQ("lightuserdata", tostringview(LuaType::TLIGHTUSERDATA));
    EXPECT_EQ("number", tostringview(LuaType::TNUMBER));
    EXPECT_EQ("string", tostringview(LuaType::TSTRING));
    EXPECT_EQ("table", tostringview(LuaType::TTABLE));
    EXPECT_EQ("function", tostringview(LuaType::TFUNCTION));
    EXPECT_EQ("userdata", tostringview(LuaType::TUSERDATA));
    EXPECT_EQ("thread", tostringview(LuaType::TTHREAD));
    EXPECT_EQ("none", tostringview(static_cast<LuaType>(-2)));
    EXPECT_EQ("none", tostringview(static_cast<LuaType>(9)));
}
