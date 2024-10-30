
#include <LuaAide.h>

using namespace std;

tuple<int, int, int>keynum(lua_State*L)
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

static void format1(lua_State*L, vector<string>&result, int level)
{
    LuaStack Q(L);
    string indent(4*level, ' ');
    const auto t=lua_type(Q, -1);
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
            sprintf(pad, "%g", Q.todouble(-1));
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
            char pad[100];
            sprintf(pad, " --[[%d %d %d]]", num_key, num_index, max_index);
            // result.back().append(pad);
            // Hier beginnt die Rekursion in die Tabelle.
            const string indent1(4*(level+1), ' ');
            if (num_key==0 && num_index==0)
            {
                if (result.size()>0) result.back().append("{}");
                else result.push_back("{}");
            }
            else if (num_key==0 && num_index>0 && max_index==num_index)
            {
                if (result.size()>0) result.back().append("{");
                else result.push_back("{");
                const string indent_cont(4*(level+1), ' ');
                enum {cont, brk} contstate=cont;
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
                    format1(L, result, level+1);
                    contstate=(result.back().size()>brklen)?brk:cont;
                }
                result.back().append("}");
            }
            else
            {
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
                    else if (Q.hasstringat(-2))
                    {
                        result.push_back(indent1+Q.tostring(stackindex(jkey))+"=");
                    }
                    else
                    {
                        result.push_back(indent1+"[???]=");
                    }
                    format1(L, result, level+1);
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
    format1(L, result, 0);
    Q.drop(1)<<"return "<<LuaGlobal("table")<<LuaDotCall("concat")<<result<<"\n">>1;
    lua_concat(Q, 2);
    return 1;
}
