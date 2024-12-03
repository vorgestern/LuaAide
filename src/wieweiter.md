
Fehlerbehandlung
1. x Schaffe LuaChunk ab.
1. _ Lege vor lua_error eine Stringdarstellung des Stacks global ab.<br/>
     Bearbeite alle Vorkommen von lua_error() und >>luaerror.<br/>
     Beachte: lua_error leert den Stack bis auf die Fehlermeldung.
1. x Benutze luaL_loadbufferx statt lua_load.

Weiter
1. _ Steuere die Objekterstellung mit objcopy so, dass unabhängig vom Zielpfad immer der gewählte Name verwendet wird.
1. _ luaaide.formatany sollte genau ein Argument akzeptieren.
1. _ Fehlendes Konzept: Metatable
1. _ Fehlendes Konzept: Userdata
1. x LuaStack<<lualist gibt einen Iterator zurück: LuaStack<<lualist<<21<<22<<23; erzeugt eine Liste.
1. x LuaStack.tostring(index) ==> std::string
1. / Schaffe LuaArray ab.
1. _ LuaStack<<lambda und LuaStack<<std::function<>. Ist das sinnvoll?
1. / Führe Literale zur Erzeugung von Objekten ein (LightUserData 0x12378lud: LuaStack.F(0x1234lud, "Hoppla"))
1. x Verlege die Implementierung von formatany von alltag nach LuaAide.
1. x Verlege die Implementierung von keyescape von alltag nach LuaAide.
1. _ Implementiere LuaColonCall analog zu anderen Funktionsaufrufen```
     Stack<<Objekt<<LuaColonCall(methodname)<<arg1<<arg2>>1;``` statt wie überliefert```
     Stack<<Objekt<<arg1<<arg2<<LuaColonCall(methodname)>>1;``` Die Anzahl
     der Argumente muss dann nicht mehr angegeben werden.
1. _ Was muss man tun, damit LuaStack<<myfunc<<lualist<<21<<22<<23<<lualistend einen LuaCall (myfunc)
     zurückgibt? Mindestens müssten LuaStack und LuaCall virtuelle Methoden haben.
     Das scheint mir im Moment unverhältnismäßig.
1. _ Schaffe einen C++ Zugang zu formatany. Bisher gibt es nur einen für Lua.
