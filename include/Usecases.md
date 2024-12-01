
# Usecases

## Metatabellen

    Benannte Metatabelle
    ==================================================
    luaL_newmetatable   erzeugen 
    luaL_setmetatable   zuordnen
    luaL_getmetatable   holen/pushen

    Freie Metatabelle
    ==================================================
    lua_setmetatable    vom Stack zum Stack zuordnen
    lua_getmetatable    vom Stack pushen

    Objektbezogen
    ==================================================
    luaL_getmetafield   Metafeld eines Werts pushen
    luaL_callmeta       Aufruf einer bekannten Bethode

## Metamethoden

    __add   __sub   __mul           Mathematische Operationen
    __div   __mod   __pow
    __unm

    __band   __bor    __bxor        Binäroperationen
    __bnot   __shl    __shr

    __concat                        x:concat(y) == x..y

    __len                           x:len()

    __eq    __lt    __le            Vergleiche

    __index
    __newindex

    __call                          x(...) == __call(x, ...)

    __gc
    __close                         x:close(errobject)
    __mode                          k,v,kv weak keys or values
    __name                          Typname

## Entwurf
    Benutzung von Metamethoden
    Q<<Value<<Lmm::add<<21>>1;          Value+21
    Q<<Value<<Lmm::band<<0xff>>1;       Value&0xff
    Q<<Value<<Lmm::concat<<"msec">>1;   Value.."msec"
    Q<<Value<<Lmm::len>>1;              Value:len()
    Q<<Value<<Lmm::eq<<5>>1;            Value==5
    Q<<Value<<Lmm::index<<7>>1;         Value[7]
    Q<<Value<<Lmm::newindex<<5<<21;     Value[5]=21
    Q<<Value<<Lmm::call<<21<<22>1;      Value(21,22)
    Q<<Value<<Lmm::gc>>1;               ?
    Q<<Value<<Lmm::close>>1;
    Q<<Value<<Lmm::mode;                k|v|kv
    Q<<Value<<Lmm::name;                Typname

    Q<<Value[Lmm::add]<<21>>1;          Value+21
    Q<<Value[Lmm::band]0xff>>1;         Value&0xff
        Hier sollte ggfs eine Dummymethode auf den Stack gelegt werden,
        die beim Aufruf eine Fehlermeldung erzeugt.

    Definition von Metamethoden
    Q<<LuaTable<<function>>Lmm::add;
