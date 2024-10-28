
#include <LuaAide.h>
#include <iostream>
#include <filesystem>

using namespace std;

#if 0

// #define LIMIT_DUMP_DEPTH 0
#define LIMIT_DUMP_DEPTH 3

namespace {

    struct Indent
    {
        unsigned level{0};
        Indent(unsigned lev): level(lev){}
        friend std::ostream&operator<<(std::ostream&out, const Indent&X)
        {
            for (unsigned n=0; n<X.level; ++n) out<<'\t';
            return out;
        }
    };

    struct Str
    {
        const char*str{nullptr};
        Str(const char*s): str(s){}
        friend std::ostream&operator<<(std::ostream&out, const Str&X)
        {
            if (X.str!=nullptr)
            {
                for (const char*s=X.str; *s!=0; ++s) switch (*s)
                {
                case '\n': out<<"\\n"; break;
                case '\r': out<<"\\r"; break;
                case '\t': out<<"\\t"; break;
                default: out<<*s;
                }
                return out;
            }
            else return out<<"<nullptr>";
        }
    };

    struct IndentedLuaStackItem: public LuaStackItem
    {
        unsigned level{0};
        IndentedLuaStackItem(lua_State*Q, int index, unsigned level1=0): LuaStackItem(Q, index), level(level1){}
        IndentedLuaStackItem(const LuaStackItem&X, unsigned level1): LuaStackItem(X), level(level1){}
    };

    std::ostream&operator<<(std::ostream&out, const IndentedLuaStackItem&X)
    {
        const auto Q=State(X);
        const auto index=Index(X);
        const Indent I(X.level);
        if (lua_gettop(Q)<1) return out<<I<<"<empty>";
        else switch (lua_type(Q, index))
        {
            case LUA_TNIL: return out<<"nil";
            case LUA_TBOOLEAN: return out<<(static_cast<bool>(X)?"true":"false");
            case LUA_TNUMBER: return out<<static_cast<double>(X);
            case LUA_TSTRING: return out<<'"'<<Str(static_cast<const char*>(X))<<'"';
            case LUA_TLIGHTUSERDATA: return out<<"lightuserdata("<<lua_touserdata(Q, index)<<")";
            case LUA_TUSERDATA: return out<<"userdata("<<lua_touserdata(Q, index)<<")";
            case LUA_TFUNCTION:
            {
                if (lua_iscfunction(Q, index))
                {
                    const auto cf=lua_tocfunction(Q, index);
                    if (cf) return out<<"cfunction("<<cf<<")";
                    else    return out<<"cfunction(failed to get pointer)";
                }
                else if (lua_isfunction(Q, index)) return out<<"luafunction";
                else return out<<"function(unknown type)";
            }
            case LUA_TTABLE:
            {
#if LIMIT_DUMP_DEPTH>0
                if (X.level>LIMIT_DUMP_DEPTH) return out<<"{} output stopped at level "<<X.level;
#endif
                out<<"{";
                // Hier beginnt die Rekursion in die Tabelle.
                const int T=index<0?index-1:index;
                lua_pushnil(Q);  // first key
                unsigned num=0;
                for (; lua_next(Q, T)!=0; ++num)
                {
                    if (num>0) out<<",";
                    // uses 'key' (at index -2) and 'value' (at index -1)
                    if (lua_isnumber(Q, -2))
                    {
                        const auto key=lua_tointeger(Q, -2);
                        out<<"\n\t"<<I<<"["<<key<<"]=";
                    }
                    else if (lua_isstring(Q, -2))
                    {
                        const char*key=(const char*)lua_tolstring(Q, -2, nullptr);
                        out<<"\n\t"<<I<<key<<"=";
                    }
                    out<<IndentedLuaStackItem(Q, -1, X.level+1);
                    // removes 'value'; keeps 'key' for next iteration
                    lua_pop(Q, 1);
                }
                return (num>0?out<<"\n"<<I:out)<<'}';
            }
            default:
            {
                return out<<"unexpected_type("<<lua_type(Q, index)<<")";
            }
        }
    }

} // anon

std::ostream&operator<<(std::ostream&out, const LuaStack&X)
{
    const int h=height(X);
    out<<"Stack("<<h<<")={";
    for (int n=-1; n>=-h; --n)
    {
        lua_State*Q=X;
        out<<(n<-1?",":"")<<"\n\t["<<n<<"]="<<IndentedLuaStackItem(Q, n, (unsigned)1);
    }
    return out<<"\n}";
}

#endif

static void format1(lua_State*L, vector<string>&result, int level)
{
    LuaStack Q(L);
    string indent(4*level, ' ');
    const auto t=lua_type(Q, -1);
    switch (t)
    {
        case LUA_TNIL: result.push_back(indent+"nil"); return;
        case LUA_TBOOLEAN:
        {
            const auto f=Q.tobool(-1);
            result.push_back(indent+(f?"true":"false"));
            return;
        }
        case LUA_TNUMBER:
        {
            char pad[100];
            sprintf(pad, "%g", Q.todouble(-1));
            result.push_back(indent+pad);
            return;
        }
        case LUA_TSTRING:
        {
            result.push_back(indent+Q.tostring(-1));
            return;
        }
        case LUA_TLIGHTUSERDATA:
        {
            char pad[100];
            sprintf(pad, "lightuserdata(%p)", lua_touserdata(Q, -1));
            result.push_back(indent+pad);
            return;
        }
        case LUA_TUSERDATA:
        {
            char pad[100];
            sprintf(pad, "userdata(0x%p)", lua_touserdata(Q, -1));
            result.push_back(indent+pad);
            return;
        }
        case LUA_TFUNCTION:
        {
                 if (lua_iscfunction(Q, -1)) result.push_back(indent+"cfunction");
            else if (lua_isfunction(Q, -1)) result.push_back(indent+"luafunction");
            else result.push_back(indent+"function(unknown type)");
            return;
        }
        case LUA_TTABLE:
        {
            result.push_back(indent+"{");
#if 0
            // Hier beginnt die Rekursion in die Tabelle.
            const int T=-2; // index<0?index-1:index;
            lua_pushnil(Q);  // first key
            unsigned num=0;
            for (; lua_next(Q, T)!=0; ++num)
            {
                // if (num>0) out<<",";
                // uses 'key' (at index -2) and 'value' (at index -1)
                if (lua_isnumber(Q, -2))
                {
                    const auto key=lua_tointeger(Q, -2);
                    out<<"\n\t"<<I<<"["<<key<<"]=";
                }
                else if (lua_isstring(Q, -2))
                {
                    const char*key=(const char*)lua_tolstring(Q, -2, nullptr);
                    out<<"\n\t"<<I<<key<<"=";
                }
                out<<IndentedLuaStackItem(Q, -1, X.level+1);
                // removes 'value'; keeps 'key' for next iteration
                lua_pop(Q, 1);
            }
#endif
            result.push_back(indent+"}");
            // (num>0?out<<"\n"<<I:out)<<'}';
            return;
        }
        default:
        {
            char pad[100];
            sprintf(pad, "unexpected_type(%d)", t);
            result.push_back(indent+pad);
            return;
        }
    }
}

int formatany(lua_State*L)
{
    LuaStack Q(L);
    vector<string>result;
    format1(L, result, 0);
    Q<<LuaGlobal("table")<<LuaDotCall("concat")<<result<<"\n">>1;
    return 1;
}
