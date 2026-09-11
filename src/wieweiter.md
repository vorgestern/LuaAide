
## Fehlerbehandlung
1. _ Lege vor lua_error eine Stringdarstellung des Stacks global ab.<br/>
     Bearbeite alle Vorkommen von lua_error() und >>luaerror.<br/>
     Beachte: lua_error leert den Stack bis auf die Fehlermeldung.

## Weiter
1. _ Steuere die Objekterstellung mit objcopy so, dass unabhängig vom Zielpfad immer der gewählte Name verwendet wird.
1. _ Fehlendes Konzept: Metatable
1. _ Fehlendes Konzept: Userdata
1. _ LuaStack<<lambda und LuaStack<<std::function<>. Ist das sinnvoll?
1. _ Was muss man tun, damit LuaStack<<myfunc<<lualist<<21<<22<<23<<lualistend einen LuaCall (myfunc)
     zurückgibt? Mindestens müssten LuaStack und LuaCall virtuelle Methoden haben.
     Das scheint mir im Moment unverhältnismäßig.
1. + Schaffe einen C++ Zugang zu formatany. Bisher gibt es nur einen für Lua.
1. _ Bilde lua_gettable(L, stackindex(Table)) ab. Es dient dem Zugriff auf ein Tabellenelement
     mit einem Schlüssel beliebigen Typs ('LuaKey'). Evtl sollte dafür der Typ absindex sichtbar
     gemacht werden. Dann kann man sowas wie pushable<absindex> LuaKey einführen.
1. _ make install
1. _ premake5
1. _ rename rotate up dig und rotate down tuck.
1. + Alternative colon call:     Q<<"21 22 23"<<LuaMethod("match")<<"(%d+) (%d+) (%d+)">>3;
1. _ Alternative list creation:  Q<<21<<22<<23<<LuaList(3);

# Transitions

    LuaStack --> LuaCall                <<lua_CFunction
                                        <<Callable
                                        <<LuaGlobalCall
                                        <<LuaClosure

     LuaStack --> LuaList               <<LuaListStart

     LuaCall --> LuaStack               >>LuaGlobal                 alle unerwünscht?
                                        >>LuaField
                                        >>LuaElement
                                        >>LuaRegValue

     LuaCall --> LuaList                --

     LuaList --> LuaCall                --

## Usecases Probleme

     Q<<lualist<<1<<2<<LuaCode(...)<<args>>1<<4<<lualistend;        klappt nicht, weil nach dem Abschluss von LuaCall kein LusList mehr vorliegt.
        Liste          Code           Stack ?
