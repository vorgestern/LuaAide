
#include <LuaAide.h>
#include <string>
#include <string_view>
#include <array>

using namespace std;

static tuple<size_t, size_t, long long>keynum(lua_State*L)
{
    LuaStack Q(L);
    size_t num_index=0, num_key=0;
    long long max_index=-1;
    for (LuaIterator J(Q); next(J); ++J)
    {
        // [key, value]
        if (Q.hasintat(-2))
        {
            ++num_index;
            const auto index=Q.toint(-2);
            if (index>max_index) max_index=index;
        }
        else ++num_key;
    }
    return {num_key, num_index, max_index};
}

const string quot="\"";
const string kk1="[[", kk2="]]";
const auto brklen=120u;

static const size_t compute_bracketlevel(string_view X)
{
    const array<string,12> A={
        "]]",
        "]=]",
        "]==]",
        "]===]",
        "]====]",
        "]=====]",
        "]======]",
        "]=======]",
        "]========]",
        "]=========]",
        "]==========]",
        "]===========]"
    };
    for (auto j=0u; j<A.size(); ++j)
    {
        const auto wo=X.find(A[j]);
        if (wo==X.npos) return j;
    }
    return A.size();
}

static void format1(lua_State*L, vector<string>&result, int level, int usedlevel)
{
    //  Zum Unterschied zwischen level und usedlevel:
    //  'Level' ist exakt der Hierarchielevel.
    //  'Usedlevel' wird nur inkrementiert, wenn tatächlich Ausgaben in neue Zeilen gemacht werden:
    //  {                                 inkrementiert usedlevel bei jedem Rekursionsschritt
    //      a={
    //          aa={
    //          }
    //      },
    //  }
    //  -----------------------
    //  {1, 2, {}, 3,                     inkrementiert usedlevel nur einmal
    //      {11, {}, 12, 13,
    //          {
    //          }
    //      }
    //  }
    LuaStack Q(L);
    if (!Q.check(5))
    {
        string memo="--[[Recursion stopped (Lua is out of stack space).]]";
        if (result.size()>0) result.back().append(memo);
        else result.push_back(memo);
    }
    string indent(4*level, ' ');
    const auto t=Q.typeat(-1);
    switch (t)
    {
        case LuaType::TNIL:
        {
            if (result.size()>0) result.back().append("nil");
            else result.push_back("nil");
            return;
        }
        case LuaType::TBOOLEAN:
        {
            const auto f=Q.tobool(-1);
            const char*str=f?"true":"false";
            if (result.size()>0) result.back().append(str);
            else result.push_back(str);
            return;
        }
        case LuaType::TNUMBER:
        {
            char pad[100];
            if (Q.hasintat(-1)) snprintf(pad, sizeof(pad), "%lld", Q.toint(-1));
            else snprintf(pad, sizeof(pad), "%g", Q.todouble(-1));
            if (result.size()>0) result.back().append(pad);
            else result.push_back(pad);
            return;
        }
        case LuaType::TSTRING:
        {
            auto s=Q.tostring(-1);
            auto neu=[](string s)->string
            {
                if (s.find_first_of('\n')!=s.npos || s.find_first_of('"')!=s.npos)
                {
                    const auto level=compute_bracketlevel(s);
                    if (level>0)
                    {
                        const string eq(level, '=');
                        return "["+eq+"["+s+"]"+eq+"]";
                    }
                    else return kk1+s+kk2;
                }
                else return quot+s+quot;
            }(s);
            if (result.size()>0) result.back().append(neu);
            else result.push_back(neu);
            return;
        }
        case LuaType::TLIGHTUSERDATA:
        {
            char pad[100];
            snprintf(pad, sizeof(pad), "lightuserdata(%p)", lua_touserdata(Q, -1));
            if (result.size()>0) result.back().append(pad);
            else result.push_back(pad);
            return;
        }
        case LuaType::TUSERDATA:
        {
            char pad[100];
            snprintf(pad, sizeof(pad), "userdata(0x%p)", lua_touserdata(Q, -1));
            if (result.size()>0) result.back().append(pad);
            else result.push_back(pad);
            return;
        }
        case LuaType::TFUNCTION:
        {
            const auto repr=lua_iscfunction(Q, -1)?"cfunction":lua_isfunction(Q, -1)?"luafunction":"function(unknown type)";
            if (result.size()>0) result.back().append(repr);
            else result.push_back(repr);
            return;
        }
        case LuaType::TTABLE:
        {
            const auto [num_key, num_index, max_index]=keynum(Q);
            // Hier beginnt die Rekursion in die Tabelle.
            const string indent1(4*(level+1), ' ');
            if (num_key==0 && num_index==0)
            {
                // Leere Tabelle
                if (result.size()>0) result.back().append("{}");
                else result.push_back("{}");
            }
            else if (num_key==0 && num_index>0 && max_index>0 && static_cast<size_t>(max_index)==num_index)
            {
                // Vektor, Liste
                if (result.size()>0) result.back().append("{");
                else result.push_back("{");
                enum {cont, brk} contstate=cont;
                string indent_cont(4*(usedlevel+1), ' ');
                for (LuaIterator I(Q); next(I); ++I)
                {
                    if ((unsigned)I>1)
                    {
                        if (contstate==cont) result.back().append(", ");
                        else
                        {
                            result.back().append(",");
                            result.push_back(indent_cont);
                        }
                    }
                    format1(L, result, level+1, usedlevel);
                    contstate=(result.back().size()>brklen)?brk:cont;
                }
                result.back().append("}");
            }
            else
            {
                // Allgemeine Tabelle
                if (result.size()>0) result.back().append("{");
                else result.push_back("{");
                for (LuaIterator I(Q); next(I); ++I)
                {
                    if ((unsigned)I>1) result.back().append(",");
                    const auto jkey=Q.index(-2); // , jvalue=Q.index(-1);
                    if (Q.hasintat(-2))
                    {
                        char pad[100];
                        snprintf(pad, sizeof(pad), "%lld", Q.toint(stackindex(jkey)));
                        result.push_back(indent1+"["+pad+"]=");
                    }
                    else if (Q.hasnumberat(-2))
                    {
                        Q<<LuaValue(stackindex(jkey));
                        const string a=Q.tostring(-1);
                        Q.drop(1);
                        result.push_back(indent1+"["+a+"]=");
                    }
                    else if (Q.hasstringat(-2))
                    {
                        Q<<keyescape<<LuaValue(stackindex(jkey))>>1;
                        const string a=Q.tostring(-1);
                        Q.drop(1);
                        result.push_back(indent1+a+"=");
                    }
                    else
                    {
                        Q<<LuaGlobalCall("tostring")<<LuaValue(stackindex(jkey))>>1;
                        const string repr=Q.asstring(-1);
                        result.push_back(indent1+"["+repr+"]=");
                        Q.drop(1);
                    }
                    format1(L, result, level+1, usedlevel+1);
                }
                result.push_back(indent+"}");
            }
            return;
        }
        default:
        {
            char pad[100];
            snprintf(pad, sizeof(pad), "unexpected_type(%d)", static_cast<int>(t));
            if (result.size()>0) result.back().append(pad);
            else result.push_back(pad);
            return;
        }
    }
}

int formatany(lua_State*L)
{
    LuaStack Q(L);
    vector<string>result;
    const auto numargs=height(Q);
    if (numargs==0) result.push_back("nil");
    else for (auto n=1u; n<=numargs; ++n)
    {
        Q<<LuaValue(n);
        format1(L, result, 0, 0);
        Q.drop(1);
        if (n<numargs && result.size()>0) result.back().append(", ");
        else if (n<numargs) result.push_back(", ");
    }
    Q.drop(1)<<"return "<<LuaGlobal("table")<<LuaDotCall("concat")<<result<<"\n">>1;
    lua_concat(Q, 2);
    return 1;
}

// ============================================================================

#ifdef UNITTEST
#include <gtest/gtest.h>

class FormatAnyEnv: public ::testing::Test
{
protected:
    LuaStack Q1 {};
    void SetUp() override { Q1=LuaStack::New(true, nullptr); }
    void TearDown() override { Q1.Close(); }
};

TEST_F(FormatAnyEnv, String1Regular)
{
    auto Q=Q1<<formatany;                           ASSERT_EQ(1, height(Q));
    Q<<"abc-def";                                   ASSERT_EQ(2, height(Q));
    Q>>1;                                           ASSERT_EQ(1, height(Q))<<Q;
    ASSERT_EQ("return \"abc-def\"", Q.tostring(-1))<<Q;
}
TEST_F(FormatAnyEnv, String2Newline)
{
    auto Q=Q1<<formatany;                           ASSERT_EQ(1, height(Q));
    Q<<"abc\ndef";                                  ASSERT_EQ(2, height(Q));
    Q>>1;                                           ASSERT_EQ(1, height(Q))<<Q;
    ASSERT_EQ("return [[abc\ndef]]", Q.tostring(-1))<<Q;
}
TEST_F(FormatAnyEnv, String3Quote)
{
    auto Q=Q1<<formatany;                           ASSERT_EQ(1, height(Q));
    Q<<"ab \"cd\" ef";                              ASSERT_EQ(2, height(Q));
    Q>>1;                                           ASSERT_EQ(1, height(Q))<<Q;
    ASSERT_EQ("return [[ab \"cd\" ef]]", Q.tostring(-1))<<Q;
}

TEST_F(FormatAnyEnv, BracketLevel)
{
    ASSERT_EQ(0, compute_bracketlevel("[ [\n] ]"));
    ASSERT_EQ(0, compute_bracketlevel("[=[\n]=]"));
    ASSERT_EQ(1, compute_bracketlevel("[[1\n]]"));
    ASSERT_EQ(1, compute_bracketlevel("[==[[[1\n]]==]"));
    ASSERT_EQ(2, compute_bracketlevel("[=[[\n]]=]"));
}

TEST_F(FormatAnyEnv, String4Bracket1)
{
    auto Q=Q1<<formatany;                           ASSERT_EQ(1, height(Q));
    Q<<"print '\n]]\n'";                            ASSERT_EQ(2, height(Q));
    Q>>1;                                           ASSERT_EQ(1, height(Q))<<Q;
    ASSERT_EQ("return [=[print '\n]]\n']=]", Q.tostring(-1))<<Q;
}

TEST_F(FormatAnyEnv, String4Bracket2)
{
    auto Q=Q1<<formatany;                           ASSERT_EQ(1, height(Q));
    Q<<"print '\n]=]\n'";                           ASSERT_EQ(2, height(Q));
    Q>>1;                                           ASSERT_EQ(1, height(Q))<<Q;
    ASSERT_EQ("return [[print '\n]=]\n']]", Q.tostring(-1))<<Q;
}

TEST_F(FormatAnyEnv, String4Bracket3)
{
    auto Q=Q1<<formatany;                           ASSERT_EQ(1, height(Q));
    Q<<"print '\n]=]\n]]'";                         ASSERT_EQ(2, height(Q));
    Q>>1;                                           ASSERT_EQ(1, height(Q))<<Q;
    ASSERT_EQ(R"__(return [==[print '
]=]
]]']==])__", Q.tostring(-1))<<Q;
}

#endif
