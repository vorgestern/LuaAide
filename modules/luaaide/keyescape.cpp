
#include <LuaAide.h>

using namespace std;

int keyescape(lua_State*L)
{
    LuaStack Q(L);
    size_t len=0;
    const string a=lua_tolstring(L, -1, &len);
    const auto s=a.c_str();
// printf("keyescape '%s'\n", s);
    bool issymbol=true;
    size_t required=0;
    for (size_t j=0; j<len; ++j)
    {
        const char c=s[j];
//      printf("\nj %lu c %u isalpha %d isdigit %d isprint %d\n", j, c, isalpha(c), isdigit(c), isprint(c));
        if (isalpha(c) || c=='_') ++required;
        else if (isdigit(c) && j>0) ++required;
        else
        {
            issymbol=false;
            if (isprint(c)) ++required;
            else switch (c)
            {
                case '\a':
                case '\b':
                case '\f':
                case '\n':
                case '\r':
                case '\t':
                case '\v':
                case '\\':
                case '\'':
                case '\"':
                case '\?':
                case '\0':
                case '\e': required+=2; break;
                default: required+=4; break;
            }
        }
    }
// printf("    issymbol=%d\n    required=%lu\n", issymbol, required);
    if (issymbol)
    {
        Q.dup(-1);
    }
    else
    {
        string result(required, ' ');
        auto r=result.data();
        for (size_t j=0; j<len; ++j)
        {
            const auto c=s[j];
            if (isprint(c)) *r++=c;
            else
            {
                switch (s[j])
                {
                    case '\a': *r++='\\'; *r++='a'; break;
                    case '\b': *r++='\\'; *r++='b'; break;
                    case '\f': *r++='\\'; *r++='f'; break;
                    case '\n': *r++='\\'; *r++='n'; break;
                    case '\r': *r++='\\'; *r++='r'; break;
                    case '\t': *r++='\\'; *r++='t'; break;
                    case '\v': *r++='\\'; *r++='v'; break;
                    case '\\': *r++='\\'; *r++='\\'; break;
                    case '\'': *r++='\\'; *r++='\''; break;
                    case '\"': *r++='\\'; *r++='"'; break;
                    case '\?': *r++='\\'; *r++='?'; break;
                    case '\0': *r++='\\'; *r++=0; break;
                    case '\e': *r++='\\'; *r++='e'; break;
                    default:
                    {
                        *r++='\\';
                        *r++='x';
                        *r++=(c>>4)<0xa?'1'+(c>>4):'a'+((c>>4)-10);
                        *r++=(c&0xf)<0xa?'1'+(c&0xf):'a'+((c&0xf)-10);
                        break;
                    }
                }
            }
        }
        Q<<"[\""+result+"\"]";
    }
    return 1;
}

int keyescape1(lua_State*L)
{
    LuaStack Q(L);
    if (height(Q)<1)
    {
        Q<<"keyescape: Argument (string) expected">>luaerror;
        return 0;
    }
    if (lua_type(L, -1)!=LUA_TSTRING)
    {
        Q<<"keyescape: string expected">>luaerror;
        return 0;
    }

    size_t len=0;
    const char*a=lua_tolstring(L, -1, &len);
    const string A(a, len);

    bool issymbol=true;
    size_t required=0;
    static const string_view repr2 {"\a\b\f\r\n\t\v\\\'\"\e"};
    static const string_view repr2a {"abfrntv\\\'\"e"};
    for (size_t j=0; j<len; ++j)
    {
        const char c=A[j];
        if (isalpha(c) || c=='_') ++required;
        else if (isdigit(c) && j>0) ++required;
        else
        {
            issymbol=false;
            if (isprint(c))                             required++;
            else if (c==0 || repr2.find(c)!=repr2.npos) required+=2;
            else                                        required+=4;
        }
    }
    if (issymbol)
    {
        Q.dup(-1);
        return 1;
    }
    string B(required, 0);
    for (size_t j=0, r=0; j<len && r<required; ++j)
    {
        const char c=A[j];
        if (isprint(c)) B[r++]=c;
        else if (c==0){ B[r++]='\\'; B[r++]='0'; }
        else
        {
            const auto u=repr2.find(c);
            if (u!=repr2.npos)
            {
                B[r++]='\\';
                B[r++]=repr2a[u];
            }
            else
            {
                static const string_view X {"0123456789abcdef"};
                const auto n1=(c>>4)&0xf, n2=c&0xf;
                B[r++]='\\';
                B[r++]='x';
                B[r++]=X[n1];
                B[r++]=X[n2];
            }
        }
    }
    Q<<"[\""+B+"\"]";
    return 1;
}
