
# Usecases

## Metatables

    Named metatables
    ==================================================
    luaL_newmetatable   create 
    luaL_setmetatable   assign
    luaL_getmetatable   get/push

    Free metatables
    ==================================================
    lua_setmetatable    assign from stack to stackvalue
    lua_getmetatable    push from stackvalue

    Implied metatables
    ==================================================
    luaL_getmetafield   push metafield of a stackvalue
    luaL_callmeta       call metafunction of stackvalue

## Metamethods

    __tostring                      Missing in manual 2.4!

    __add   __sub   __mul           Mathematical operations
    __div   __mod   __pow
    __unm

    __band   __bor    __bxor        Binary operations
    __bnot   __shl    __shr

    __concat                        x:concat(y) == x..y

    __len                           x:len()

    __eq    __lt    __le            Comparisons

    __index
    __newindex

    __call                          x(...) == __call(x, ...)

    __gc
    __close                         x:close(errobject)
    __mode                          k,v,kv weak keys or values
    __name                          Type name

## Entwurf
Using Metamethods

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
    Q<<Value<<Lmm::name;                Type name

    Q<<Value[Lmm::add]<<21>>1;          Value+21
    Q<<Value[Lmm::band]<<0xff>>1;       Value&0xff
        On error operator[] shold push function to issue error message

Definition of Metamethods

    Q<<LuaTable<<function>>Lmm::add;

Creation of arbitrary tables

    // verbose
    Q<<LuaTable()<<21>>LuaField("x")<<22>>LuaField("y")<<23>>LuaField("z")>>LuaGlobal("mytable");

    // not chainable (operator precendence)
    Q<<LuaTable()<<21>="x";
    Q<<22>="y";
    Q<<23>="z";
    Q>>LuaGlobal("mytable");

    // chainable after restart, verbose
    Q<<LuaTable();
    Q.F("key1", 101).F("version", "0.1").F("print", myprint);
