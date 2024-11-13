
#include <ostream>
#include <LuaAide.h>

using namespace std;

// #define LIMIT_DUMP_DEPTH 0
#define LIMIT_DUMP_DEPTH 3

namespace {

    struct Str
    {
        const char*str{nullptr};
        Str(const char*s): str(s){}
        friend ostream&operator<<(ostream&out, const Str&X)
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

    ostream&operator<<(ostream&out, const IndentedLuaStackItem&X)
    {
        LuaStack Q=State(X);
        const auto index=Index(X);
        const string Indent('\t', X.level);
        if (height(Q)<1) return out<<Indent<<"<empty>";
        else switch (Q.typeat(index))
        {
            case LuaType::TNIL: return out<<"nil";
            case LuaType::TBOOLEAN: return out<<(static_cast<bool>(X)?"true":"false");
            case LuaType::TNUMBER: return out<<static_cast<double>(X);
            case LuaType::TSTRING: return out<<'"'<<Str(static_cast<const char*>(X))<<'"';
            case LuaType::TLIGHTUSERDATA: return out<<"lightuserdata("<<lua_touserdata(Q, index)<<")";
            case LuaType::TUSERDATA: return out<<"userdata("<<lua_touserdata(Q, index)<<")";
            case LuaType::TFUNCTION:
            {
                const auto p=lua_topointer(Q, index);
                if (lua_iscfunction(Q, index))     return out<<"cfunction("<<p<<")";
                else if (lua_isfunction(Q, index)) return out<<"luafunction("<<p<<")";
                else return out<<"function(unknown type)";
            }
            case LuaType::TTABLE:
            {
#if LIMIT_DUMP_DEPTH>0
                if (X.level>LIMIT_DUMP_DEPTH) return out<<"{} output stopped at level "<<X.level;
#endif
                out<<"{";
                // Hier beginnt die Rekursion in die Tabelle.
                const int T=index<0?index-1:index;
                Q<<luanil;  // first key
                unsigned num=0;
                for (; lua_next(Q, T)!=0; ++num)
                {
                    if (num>0) out<<",";
                    // uses 'key' (at index -2) and 'value' (at index -1)
                    if (Q.hasnumberat(-2))
                    {
                        const auto key=Q.toint(-2);
                        out<<"\n\t"<<Indent<<"["<<key<<"]=";
                    }
                    else if (lua_isstring(Q, -2))
                    {
                        const char*key=(const char*)lua_tolstring(Q, -2, nullptr);
                        out<<"\n\t"<<Indent<<key<<"=";
                    }
                    out<<IndentedLuaStackItem(Q, -1, X.level+1);
                    // removes 'value'; keeps 'key' for next iteration
                    Q.drop(1);
                }
                return (num>0?out<<"\n"<<Indent:out)<<'}';
            }
            default:
            {
                return out<<"unexpected_type("<<static_cast<int>(Q.typeat(index))<<")";
            }
        }
    }

} // anon

ostream&operator<<(ostream&out, const LuaStack&X)
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

ostream&operator<<(ostream&out, const LuaStackItem&X){ return out<<IndentedLuaStackItem(X, 0); }
