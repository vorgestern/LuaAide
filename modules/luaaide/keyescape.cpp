
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
