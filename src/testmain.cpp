
#include <gtest/gtest.h>
#include <LuaAide.h>
#include <iostream>

using namespace std;

int main(int argc, char*argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

static int panichandler(lua_State*L)
{
    LuaStack Q(L);
    throw runtime_error(Q.errormessage());
    return 0;
}

class Lua: public ::testing::Test
{
protected:
    LuaStack Q{};
    void SetUp() override { Q=LuaStack::New(true, panichandler); }
    void TearDown() override { Q.Close(); }
};

TEST_F(Lua, Getmetafield)
{
    Q.clear(); Q<<"21";
    const auto t1=(LuaType)luaL_getmetafield(Q, -1, "match"); cout<<"t1="<<tostring(t1)<<"\n"; // cout<<Q<<"\n";
    Q.clear(); Q<<21;
    const auto t2=(LuaType)luaL_getmetafield(Q, -1, "match"); cout<<"t2="<<tostring(t2)<<"\n"; // cout<<Q<<"\n";

    if (0==Q<<LuaCode("return {match=function() end}")>>1)
    {
        const auto t=(LuaType)luaL_getmetafield(Q, -1, "match"); cout<<"table (1) metafield="<<tostring(t)<<"\n"; // cout<<Q<<"\n";
    }
    if (0==Q<<LuaCode("local mt={}; mt.__index=mt; return setmetatable({}, mt)")>>1)
    {
        const auto t=(LuaType)luaL_getmetafield(Q, -1, "match"); cout<<"table (2) metafield="<<tostring(t)<<"\n"; // cout<<Q<<"\n";
    }
    if (0==Q<<LuaCode("local mt={match=function() end}; mt.__index=mt; return setmetatable({}, mt)")>>1)
    {
        const auto t=(LuaType)luaL_getmetafield(Q, -1, "match"); cout<<"table (3) metafield="<<tostring(t)<<"\n"; // cout<<Q<<"\n";
    }
}

TEST_F(Lua, GetfieldOfInt)
{
    ASSERT_EQ(1, (Q.clear()<<21, height(Q)));
    // Function does not exist.
    ASSERT_THROW(lua_getfield(Q, -1, "print"), runtime_error);
}

TEST_F(Lua, GetfieldOfString)
{
    ASSERT_EQ(1, (Q.clear()<<"21", height(Q)));
    {
        // Function exists.
        const auto t=(LuaType)lua_getfield(Q, -1, "match");
        ASSERT_EQ(LuaType::TFUNCTION, t)<<"string expected to have function 'match', but is "<<tostring(t)<<"\n."<<Q<<"\n";
    }
    ASSERT_EQ(1, (Q.clear()<<"21", height(Q)));
    {
        // Function does not exist.
        const auto t1=(LuaType)lua_getfield(Q, -1, "print");
        ASSERT_EQ(LuaType::TNIL, t1)<<"string expected to not have function 'print'.\n"<<Q<<"\n";
    }
}

TEST_F(Lua, GetfieldOfTable)
{
    ASSERT_EQ(0, Q.clear()<<LuaCode("return {match=function() end}")>>1);
    {
        // Field exists directly
        const auto t=(LuaType)lua_getfield(Q, -1, "match");
        ASSERT_NE(LuaType::TNIL, t)<<"getfield should find a function directly added to a table.\n"<<Q<<"\n";
    }

    ASSERT_EQ(0, Q.clear()<<LuaCode("return {}")>>1);
    {
        // Field does not exist.
        const auto t=(LuaType)lua_getfield(Q, -1, "match");
        ASSERT_EQ(LuaType::TNIL, t)<<"getfield should not find function 'match'.\n"<<Q<<"\n";
    }
}
