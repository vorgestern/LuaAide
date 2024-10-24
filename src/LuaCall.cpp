
#include <LuaAide.h>

LuaCall::LuaCall(lua_State*L): LuaStack(L), funcindex(L){}
LuaCall::LuaCall(LuaStack&S): LuaStack(S), funcindex(L){}
LuaCall::LuaCall(lua_State*L, unsigned start): LuaStack(L), funcindex(L, -1*(int)start){}

LuaCall&operator<<(LuaCall&S, const LuaUpValue&V)
{
    const int stackindex=lua_upvalueindex(V.index);
    lua_pushvalue(S, stackindex);
    return S;
}

static int TracebackAdder(lua_State*Q)
{
    // Dieser ErrorHandler fügt der Fehlermeldung den Stacktrace hinzu.
    // https://stackoverflow.com/questions/63570555/how-do-i-improve-lua-internal-error-messages-to-include-line-numbers
    const char*msg=lua_tostring(Q, -1);
    luaL_traceback(Q, Q, msg, 2);
    lua_remove(Q, -2); // Remove error/"msg" from stack.
    return 1; // Traceback is returned.
}

int operator>>(LuaCall&S, int numresults)
{
    const int top0=lua_gettop(S.L);
    const LuaAbsIndex now(S);
    const int numargs=stackindex(now)>=stackindex(S.funcindex)?stackindex(now)-stackindex(S.funcindex):0;
    lua_pushcfunction(S.L, TracebackAdder);             // [func, args[numargs], errorhandler]
    const int idx=-(numargs+2);
    lua_rotate(S.L, idx, 1);                            // [errorhandler, func, args[numargs]]
    const int rc=lua_pcall(S.L, numargs, numresults, idx);
    const int top1=lua_gettop(S.L);
    switch (rc)
    {
        case LUA_OK:
        {
            const int results=top1-top0+(numargs+1)-1;
            lua_remove(S.L, -results-1);
            break;
        }
        default:
        case LUA_ERRRUN:
        case LUA_ERRMEM:  // memory allocation error. For such errors, Lua does not call the message handler
        case LUA_ERRERR:  // error while running the message handler
        // case LUA_ERRGCMM: // error while running garbage collection metamethod (__gc)
        // Dieser Fehlercode wurde in Lua 5.4 entfernt.
        // Vgl. http://www.lua.org/manual/5.4/manual.html#8.3
        {
            lua_remove(S.L, -2);
            break;
        }
    }
    return rc;
}
