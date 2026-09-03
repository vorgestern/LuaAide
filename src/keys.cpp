
#include <LuaAide.h>
#include <string>
#include <string_view>
#include <array>
#include <iostream>
#include <algorithm>
#include <cassert>

using namespace std;

int keys(lua_State*L)
{
    LuaStack Q(L);
    if (height(Q)<1)
    {
        Q<<"Error: sortedkeys requires one argument (table)">>luaerror;
        return 0;
    }
    if (Q.typeat(-1)!=LuaType::TTABLE)
    {
        Q<<lualist<<lualistend;
        return 1;
    }
    // There is a table at the top of the stack.
    // Collect the keys in a newly created table.
    Q<<lualist<<lualistend;
    Q.swap();
    const auto Keys=Q.index(-2);
    for (LuaIterator J(Q); next(J); ++J)
    {
        // [key, item]
        Q<<LuaValue(-2)>>LuaElement({stackindex(Keys), (unsigned)J}); // add result to mappedlist.
    }
    Q<<Keys;
    return 1;
}

static int cmp1(lua_State*L)
{
    LuaStack Q(L);
    auto K1=Q.index(-2), K2=Q.index(-1);
    Q<<LuaElement({stackindex(K1), 1});
    Q<<LuaElement({stackindex(K2), 1});
    const auto ks1=Q.tostring(-2), ks2=Q.tostring(-1);
    // const auto cmp=lua_compare(Q, -2, -1, LUA_OPLT);
    Q<<(ks1<ks2?true:false);
    return 1;
}

int sortedkeys(lua_State*L)
{
    LuaStack Q(L);
    if (height(Q)<1)
    {
            Q<<"Error: sortedkeys requires one argument (table)">>luaerror;
            return 0;
    }
    if (Q.typeat(-1)!=LuaType::TTABLE)
    {
            Q<<lualist<<lualistend;
            return 1;
    }
    // There is a table at the top of the stack.
    // Collect the keys in a newly created table.
    Q<<lualist<<lualistend;
    Q.swap();
    const auto Keylist=Q.index(-2);
    for (LuaIterator J(Q); next(J); ++J)
    {
        // [key, item]
        Q<<lualist<<lualistend; // AB={key//string, key}
        auto Key=Q.index(-3), AB=Q.index(-1);

        Q<<LuaGlobalCall("tostring")<<Key>>1;
        Q>>LuaElement({stackindex(AB), 1});

        Q<<Key>>LuaElement({stackindex(AB), 2});

        Q<<AB>>LuaElement({stackindex(Keylist), (unsigned)J}); // Add AB to Keylist.

        Q.drop(1);
    }

    Q<<LuaGlobal("table")<<LuaDotCall("sort")<<Keylist<<cmp1>>0;

    if (true)
    {
        Q<<lualist<<lualistend;
        Q.swap();

        auto Result=Q.index(-2);
        for (LuaIterator J(Q); next(J); ++J)
        {
            // -2: key
            // -1: item
            Q.dup(-2);
            Q>>LuaElement({stackindex(Result), (unsigned)J});
        }

        Q<<Result;
    }
    else
    {
        Q<<Keylist;
    }
    return 1;
}

int cmp_ab(lua_State*L)
{
    LuaStack Q(L);
    auto Ta=Q.typeat(-2), Tb=Q.typeat(-1);
    if (Ta!=Tb)
    {
        Q<<((int)Ta<(int)Tb?true:false);
        return 1;
    }
    switch (Ta)
    {
        case LuaType::TBOOLEAN:
        {
            // The generic compare function does not take kindly to bools,
            // so we compare them ourselves.
            Q<<(Q.tobool(-2)?false:true);
            return 1;
        }
        // case LuaType::TNUMBER:
        // {
        //     auto a=Q.todouble(-2), b=Q.todouble(-1);
        //     Q<<(a<b?true:false);
        //     return 1;
        // }
        // case LuaType::TSTRING:
        // {
        //     const auto a=Q.tostring(-2), b=Q.tostring(-1);
        //     Q<<(a<b?true:false);
        //     return 1;
        // }
        default:
        {
            // Everything else can be handled by lua_compare.
            Q<<(lua_compare(L, -2, -1, LUA_OPLT)==1?true:false);
            return 1;
        }
    }
    return 1;
}

int sortedkeys_neu(lua_State*L)
{
    LuaStack Q(L);
    if (height(Q)<1)
    {
        Q<<"Error: sortedkeys requires one argument (table)">>luaerror;
        return 0;
    }
    if (Q.typeat(-1)!=LuaType::TTABLE)
    {
        Q<<lualist<<lualistend;
        return 1;
    }
    // There is a table at the top of the stack.
    // Collect the keys in a newly created table.
    Q<<lualist<<lualistend;
    Q.swap();
    const auto Keylist=Q.index(-2);
    for (LuaIterator J(Q); next(J); ++J)
    {
        // [key, item]
        Q<<LuaValue(-2)>>LuaElement({stackindex(Keylist), (unsigned)J}); // Add key to Keylist.
    }

    Q<<LuaGlobal("table")<<LuaDotCall("sort")<<Keylist<<cmp_ab>>0;

    Q<<Keylist;
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

class KeysEnv: public ::testing::Test
{
protected:
    LuaStack Q{};
    void SetUp() override { Q=LuaStack::New(true, panichandler); }
    void TearDown() override { Q.Close(); }
};

TEST_F(KeysEnv, keys)
{
    Q<<LuaCode("return {21,a=1,b=2,22,23,[true]=101,c=3}")>>1;
    ASSERT_EQ(1, height(Q));
    Q<<keys<<LuaValue(-2)>>1;
    ASSERT_EQ((int)LuaType::TTABLE, (int)Q.typeat(-1));
    for (LuaIterator J(Q); next(J); ++J)
    {
        auto s=Q.asstring(-1);
        cout<<(unsigned)J<<" "<<s.c_str()<<"\n";
    }
}

TEST_F(KeysEnv, sortedkeys)
{
    Q<<LuaCode("return {21,a=1,b=2,22,23,[true]=101,c=3}")>>1;
    ASSERT_EQ(1, height(Q));
    Q<<keys<<LuaValue(-2)>>1;
    ASSERT_EQ((int)LuaType::TTABLE, (int)Q.typeat(-1));
    for (LuaIterator J(Q); next(J); ++J)
    {
        auto s=Q.asstring(-1);
        cout<<(unsigned)J<<" "<<s.c_str()<<"\n";
    }
}
#endif
