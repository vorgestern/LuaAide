
1. x Schaffe LuaChunk ab.
1. _ Lege vor lua_error eine Stringdarstellung des Stacks global ab.
1. x Benutze luaL_loadbufferx statt lua_load.
1. _ Bearbeite alle Vorkommen von lua_error() und >>luaerror.
1. _ Beachte: lua_error leert den Stack bis auf die Fehlermeldung.
1. _ Steuere die Objekterstellung mit objcopy so, dass unabhängig vom Zielpfad immer der gewählte Name verwendet wird.
1. _ luaaide.formatany sollte genau ein Argument akzeptieren.
1. _ Fehlendes Konzept: Metatable
1. _ Fehlendes Konzept: Userdata
1. x LuaStack<<LuaList gibt einen Iterator zurück: LuaStack<<LuaList<<21<<22<<23; erzeugt eine Liste.
1. x LuaStack.tostrint(index) ==> std::string
1. _ Schaffe LuaArray ab.
1. _ LuaStack<<lambda
