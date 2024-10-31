
#include <LuaAide.h>

using namespace std;

int keyescape(lua_State*L)
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
