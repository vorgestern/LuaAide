
#include <LuaAide.h>

using namespace std;

int keyescape(lua_State*);

static tuple<int, int, int>keynum(lua_State*L)
{
    LuaStack Q(L);
    int num_index=0, num_key=0, max_index=-1;
    for (LuaIterator J(Q); next(J); ++J)
    {
        // [key, value]
        if (Q.hasintat(-2))
        {
            ++num_index;
            const int index=Q.toint(-2);
            if (index>max_index) max_index=index;
        }
        else ++num_key;
    }
    return {num_key, num_index, max_index};
}

const string quot="\"";
const auto brklen=120u;

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
        case LUA_TNIL:
        {
            if (result.size()>0) result.back().append("nil");
            else result.push_back("nil");
            return;
        }
        case LUA_TBOOLEAN:
        {
            const auto f=Q.tobool(-1);
            const char*str=f?"true":"false";
            if (result.size()>0) result.back().append(str);
            else result.push_back(str);
            return;
        }
        case LUA_TNUMBER:
        {
            char pad[100];
            if (Q.hasintat(-1)) snprintf(pad, sizeof(pad), "%lld", Q.toint(-1));
            else snprintf(pad, sizeof(pad), "%g", Q.todouble(-1));
            if (result.size()>0) result.back().append(pad);
            else result.push_back(pad);
            return;
        }
        case LUA_TSTRING:
        {
            if (result.size()>0) result.back().append(quot+Q.tostring(-1)+quot);
            else result.push_back(quot+Q.tostring(-1)+quot);
            return;
        }
        case LUA_TLIGHTUSERDATA:
        {
            char pad[100];
            sprintf(pad, "lightuserdata(%p)", lua_touserdata(Q, -1));
            if (result.size()>0) result.back().append(pad);
            else result.push_back(pad);
            return;
        }
        case LUA_TUSERDATA:
        {
            char pad[100];
            sprintf(pad, "userdata(0x%p)", lua_touserdata(Q, -1));
            if (result.size()>0) result.back().append(pad);
            else result.push_back(pad);
            return;
        }
        case LUA_TFUNCTION:
        {
            const auto repr=lua_iscfunction(Q, -1)?"cfunction":lua_isfunction(Q, -1)?"luafunction":"function(unknown type)";
            if (result.size()>0) result.back().append(repr);
            else result.push_back(repr);
            return;
        }
        case LUA_TTABLE:
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
            else if (num_key==0 && num_index>0 && max_index==num_index)
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
                        const string repr=Q.stringrepr(-1);
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
            sprintf(pad, "unexpected_type(%d)", t);
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
    format1(L, result, 0, 0);
    Q.drop(1)<<"return "<<LuaGlobal("table")<<LuaDotCall("concat")<<result<<"\n">>1;
    lua_concat(Q, 2);
    return 1;
}
