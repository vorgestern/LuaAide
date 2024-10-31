
#include <gtest/gtest.h>
#include <LuaAide.h>

using namespace std;

class luaaide: public ::testing::Test
{
protected:
    LuaStack Q{};
    void SetUp() override { Q=LuaStack::New(true, nullptr); }
    void TearDown() override { Q.Close(); }
};

TEST_F(luaaide, Version)
{
    Q<<LuaCode(R"xxx(
        local X=require "luaaide"
        return X.version
    )xxx")>>1;
    ASSERT_EQ(1, height(Q));
    ASSERT_TRUE(Q.hasstringat(-1));
    ASSERT_EQ("0.1", Q.stringrepr(-1));
}

TEST_F(luaaide, Keyescape)
{
    const auto rc=Q<<LuaCode(R"xxx(
        local X=require "luaaide"
        local Cases={
            {7,  "[\"abc\\adef\"]"},
            {8,  "[\"abc\\bdef\"]"},
            {9,  "[\"abc\\tdef\"]"},
            {10, "[\"abc\\ndef\"]"},
            {12, "[\"abc\\fdef\"]"},
            {13, "[\"abc\\rdef\"]"},
            {27, "[\"abc\\edef\"]"},
            {46, "[\"abc.def\"]"}
        }
        local num=0
        for c,case in ipairs(Cases) do
            local s=string.format("abc%cdef", case[1])
            local r=X.keyescape(s)
            if r==case[2] then num=num+1
            else
                print(string.format("Abweichung case %d: (char %d) %s != %s\n", c, case[1], r, case[2]))
            end
        end
        return #Cases, num
    )xxx")>>2;
    if (rc!=0)
    {
        cout<<"Scriptausfuehrung abgeschlossen mit rc="<<rc<<"\n";
        if (height(Q)>0 && Q.hasstringat(-1))
        {
            const string err=Q.tostring(-1);
            cout<<err<<"\n";
        }
    }
    ASSERT_EQ(0, rc);
    ASSERT_EQ(2, height(Q));
    ASSERT_TRUE(Q.hasnumberat(-2)); const auto numcases=Q.toint(-2);
    ASSERT_TRUE(Q.hasnumberat(-1)); const auto numsuccess=Q.toint(-1);
    ASSERT_EQ(numcases, numsuccess);
}

int main(int argc, char*argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
