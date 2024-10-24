
#include <LuaAide.h>
#include <iostream>

static void printval(lua_State*Q, int a, unsigned level=0)
{
    switch (lua_type(Q, a))
    {
        case LUA_TNIL:
        {
            fprintf(stdout, "nil");
            break;
        }
        case LUA_TBOOLEAN:
        {
            const bool f=0!=lua_toboolean(Q, a);
            fprintf(stdout, "%s", f?"true":"false");
            break;
        }
        case LUA_TNUMBER:
        {
            const double x=lua_tonumber(Q, a);
            fprintf(stdout, "%g", x);
            break;
        }
        case LUA_TSTRING:
        {
            const char*s=(char*)lua_tostring(Q, a);
            fprintf(stdout, "\"%s\"", s);
            break;
        }
        case LUA_TLIGHTUSERDATA:
        {
            const char*s=(char*)lua_touserdata(Q, a);
            fprintf(stdout, "lightuserdata(%p)", s);
            break;
        }
        case LUA_TUSERDATA:
        {
            const char*s=(char*)lua_touserdata(Q, a);
            fprintf(stdout, "userdata(%p)", s);
            break;
        }
        case LUA_TTABLE:
        {
            fprintf(stdout, "table");
            lua_pushnil(Q); // first key
            const int t=a-1;
            while (lua_next(Q, t)!=0)
            {
                // uses 'key' (at index -2) and 'value' (at index -1)
                if (lua_isnumber(Q, -2))
                {
                    const auto key=lua_tointeger(Q, -2);
                    //        printf("\n%*s[%d] is a %s:", 4*(level+1), "", key, lua_typename(Q, lua_type(Q, -1)));
                    printf("\n%*s[%lld] ", 4*(level+1), "", key);
                    printval(Q, -1, level+1);
                }
                else if (lua_isstring(Q, -2))
                {
                    const char*key=(char*)lua_tolstring(Q, -2, nullptr);
                    printf("\n%*s.%s is a %s (recursion stops)", 4*(level+1), "",
                           key, // lua_typename(Q, lua_type(Q, -2)),
                           lua_typename(Q, lua_type(Q, -1)));
                }
                // removes 'value'; keeps 'key' for next iteration
                lua_pop(Q, 1);
            }
            break;
        }
        case LUA_TFUNCTION:
        {
            if (lua_iscfunction(Q, a))
            {
                const auto cf=lua_tocfunction(Q, a);
                if (cf)   fprintf(stdout, "cfunction %p", cf);
                else fprintf(stdout, "cfunction (failed to get pointer)");
            }
            else if (lua_isfunction(Q, a))   fprintf(stdout, "lua function");
            break;
        }
        default:
        {
            fprintf(stdout, "<other type %d>", lua_type(Q, a));
            break;
        }
    }
}

static bool mypcall(LuaStack&LS, int argc, char*argv[], const char tag[])
{
    bool flag=false;
    for (int a=0; a<argc; ++a) LS<<argv[a];
    const int rc2=lua_pcall(LS, argc, LUA_MULTRET, 0);
    switch (rc2)
    {
        case LUA_OK: flag=true; break;
        case LUA_ERRRUN: std::cerr<<'\n'<<tag<<": Runtime error (LUA_ERRRUN):\n"<<LuaStackItem(LS, -1)<<'\n'; break;
        case LUA_ERRMEM: std::cerr<<'\n'<<tag<<": Memory allocation error (LUA_ERRMEM):\n"<<LuaStackItem(LS, -1)<<'\n'; break; // For such errors, Lua does not call the message handler.
        case LUA_ERRERR: std::cerr<<'\n'<<tag<<": Error handling error (LUA_ERRERR):\n"<<LuaStackItem(LS, -1)<<'\n'; break; // Error while running the message handler.

        // case LUA_ERRGCMM: std::cerr<<'\n'<<tag<<": Garbage collection error (LUA_ERRGCMM):\n"<<LuaStackItem(LS,-1)<<'\n'; break; // Error while running a __gc metamethod.
        // Dieser Fehlercode wurde in Lua 5.4 entfernt.
        // Vgl. http://www.lua.org/manual/5.4/manual.html#8.3
        // For such errors, Lua does not call the message handler.
        // (as this kind of error typically has no relation with the function being called).
        default: std::cerr<<'\n'<<tag<<": Unknown error ("<<rc2<<"):\n"<<LuaStackItem(LS, -1)<<'\n'; break;
    }
    return flag;
}

void myhandleload(LuaStack&LS, int rc1, const char tag[])
{
    if (rc1==LUA_OK)
    {
        // Dieses Ergebnis wird hier nicht behandelt.
    }
    else
    {
        std::cerr<<'\n'<<tag<<": ";
        switch (rc1)
        {
            case LUA_ERRSYNTAX: std::cerr<<"Syntax Error (LUA_ERRSYNTAX): "<<LS.tostring(-1)<<'\n'; break;
            case LUA_ERRMEM:    std::cerr<<"Out of memory (LUA_ERRMEM): "<<LS.tostring(-1)<<'\n'; break;
            // case LUA_ERRGCMM:   std::cerr<<"Garbage collection error (LUA_ERRGCMM): "<<LS.tostring(-1)<<'\n'; break;
            // Dieser Fehlercode wurde in Lua 5.4 entfernt.
            // Vgl. http://www.lua.org/manual/5.4/manual.html#8.3
            // For file-related errors (e.g., it cannot open or read the file):
            case LUA_ERRFILE:   std::cerr<<"File error (LUA_ERRFILE): "<<LS.tostring(-1)<<'\n'; break;
            default:            std::cerr<<"Unknown error (loadfile() returns "<<rc1<<"): "<<LS.tostring(-1)<<'\n'; break;
        }
    }
}

unsigned version(const LuaStack&S)
{
    return static_cast<unsigned>(lua_version(S.L));
}

LuaStack&LuaStack::clear()
{
    const int ns=lua_gettop(L);
    if (ns>0) lua_pop(L, ns);
    return *this;
}

LuaStack&LuaStack::swap()
{
    lua_pushvalue(L, -2);
    lua_remove(L, -3);
    return *this;
}

bool LuaStack::dofile(const char filename[], int argc, char*argv[])
{
    bool flag=false;
    const int rc1=luaL_loadfile(L, filename);
    switch (rc1)
    {
        case LUA_OK:
        {
            flag=mypcall(*this, argc, argv, filename);
            break;
        }
        default:
        {
            myhandleload(*this, rc1, filename);
            break;
        }
    }
    return flag;
}

bool LuaStack::dostring(const char code[], int argc, char*argv[], const char tag[])
{
    bool flag=false;
    const int rc1=luaL_loadstring(L, code);
    switch (rc1)
    {
        case LUA_OK:
        {
            flag=mypcall(*this, argc, argv, tag);
            break;
        }
        default:
        {
            myhandleload(*this, rc1, tag);
            break;
        }
    }
    return flag;
}

// **********************************************************************

LuaCall operator<<(LuaStack&S, LuaColonCall&C)
{
    // Beispiel: Es soll der Aufruf X:Funktion(a, b, c); ausgeführt werden.
    //           C.name="Funktion"
    //           C.numargs=3
    // Auf dem Stack liegt zu Beginn: [X, a, b, c]
    const int objectindex=-1-C.numargs;     // -4 zeigt auf X
    // Ermittle die Elementfunktion X[name].
    lua_getfield(S.L, objectindex, C.name); // [X, a, b, c, Funktion]
    if (!lua_isnil(S.L, -1))
    {
        lua_insert(S.L, -1+objectindex); // [Funktion, X, a, b, c]
    }
    else
    {
        // Dies führt trotzdem zum Absturz, weil der Operator>> nichts davon erfährt.
        // TODO: Behebe dies.
        printf("ColonCall: Object has no field '%s' (function required).\n", C.name);
        lua_pop(S.L, 1);
    }
    return LuaCall(S.L, 1+C.numargs); // Dieser Konstruktor ist uns durch die Freundschaftsbeziehung zugänglich.
}

LuaCall operator<<(LuaStack&S, LuaDotCall&C)
{
    const int objectindex=-1;
    lua_getfield(S.L, objectindex, C.name);
    lua_remove(S.L, objectindex-1);
    return LuaCall(S);
}

LuaCall operator<<(LuaStack&S, LuaGlobalCall&C)
{
    S<<LuaGlobal(C.name);
    return LuaCall(S);
}

LuaCall operator<<(LuaStack&S, const LuaChunk&X)
{
    const int rc=luaL_loadbufferx(S.L, X.buffer, X.bufferlength, X.buffername, "bt");
    if (rc!=LUA_OK) lua_error(S);
    return LuaCall(S);
}

LuaCall operator<<(LuaStack&S, LuaCode&C)
{
    const int rc=luaL_loadstring(S, C.text);
    if (rc!=LUA_OK) lua_error(S);
    return LuaCall(S);
}

LuaCall operator<<(LuaStack&S, lua_CFunction X)
{
    lua_pushcfunction(S.L, X);
    return LuaCall(S);
}

// *******************************************************

LuaStack&operator<<(LuaStack&S, const LuaClosure&C)
{
    lua_pushcclosure(S, C.closure, C.num_upvalues);
    return S;
}

LuaStack&operator<<(LuaStack&S, const LuaUpValue&V)
{
    const int stackindex=lua_upvalueindex(V.index);
    lua_pushvalue(S, stackindex);
    return S;
}
