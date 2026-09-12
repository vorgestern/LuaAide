
# Missing features
1.   Create Userdata
1.   Associate Userdata with POD-Structs
1.   Create/assign Metatables
1.   Introduce iterator equivalent to ipairs

# Features to improve
1.   Iteration with LuaIterator should check for metamethod __pairs, should be called LuaPairs too.
1.   Proper LiFo context transitions between LuaStack, LuaCall, LuaList

## Fehlerbehandlung
1. _ Lege vor lua_error eine Stringdarstellung des Stacks global ab.<br/>
     Bearbeite alle Vorkommen von lua_error() und >>luaerror.<br/>
     Beachte: lua_error leert den Stack bis auf die Fehlermeldung.

## Weiter
1. _ LuaStack<<lambda und LuaStack<<std::function<>. Ist das sinnvoll?
1. _ Bilde lua_gettable(L, stackindex(Table)) ab. Es dient dem Zugriff auf ein Tabellenelement
     mit einem Schlüssel beliebigen Typs ('LuaKey'). Evtl sollte dafür der Typ absindex sichtbar
     gemacht werden. Dann kann man sowas wie pushable<absindex> LuaKey einführen.
1. _ make install
1. _ premake5
1. _ rename rotate up dig und rotate down tuck.
1. _ Alternative list creation:  Q<<21<<22<<23<<LuaList(3);

# Transitions

    LuaStack --> LuaCall                <<Callable
                                        <<LuaGlobalCall
                                        <<LuaClosure

     LuaStack --> LuaList               <<LuaListStart

     LuaCall --> LuaStack               >>LuaGlobal                 alle unerwünscht?
                                        >>LuaField
                                        >>LuaElement
                                        >>LuaRegValue

     LuaCall --> LuaList                --

     LuaList --> LuaCall                --
