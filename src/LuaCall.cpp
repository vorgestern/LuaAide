
#include <LuaAide.h>

LuaCall::LuaCall(lua_State*L): LuaStack(L), funcindex(index(-1)){}
LuaCall::LuaCall(LuaStack&S): LuaStack(S), funcindex(index(-1)){}
LuaCall::LuaCall(lua_State*L, unsigned start): LuaStack(L), funcindex(index(-1*(int)start)){}

// LuaCall&LuaCall::operator<<(const LuaUpValue&V)
// {
//     const int stackindex=lua_upvalueindex(V.index);
//     lua_pushvalue(S, stackindex);
//     return S;
// }

static int TracebackAdder(lua_State*Q)
{
    // Dieser ErrorHandler fügt der Fehlermeldung den Stacktrace hinzu.
    // https://stackoverflow.com/questions/63570555/how-do-i-improve-lua-internal-error-messages-to-include-line-numbers
    const char*msg=lua_tostring(Q, -1);
    luaL_traceback(Q, Q, msg, 2);
    lua_remove(Q, -2); // Remove error/"msg" from stack.
    return 1; // Traceback is returned.
}

int LuaCall::operator>>(int numresults)
{
    const int top0=lua_gettop(L);
    const auto now=index(-1);
    const int numargs=stackindex(now)>=stackindex(funcindex)?stackindex(now)-stackindex(funcindex):0;
    lua_pushcfunction(L, TracebackAdder);             // [func, args[numargs], errorhandler]
    const int idx=-(numargs+2);
    lua_rotate(L, idx, 1);                            // [errorhandler, func, args[numargs]]
    const int rc=lua_pcall(L, numargs, numresults, idx);
    const int top1=lua_gettop(L);
    switch (rc)
    {
        case LUA_OK:
        {
            const int results=top1-top0+(numargs+1)-1;
            lua_remove(L, -results-1);
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
            lua_remove(L, -2);
            break;
        }
    }
    return rc;
}
