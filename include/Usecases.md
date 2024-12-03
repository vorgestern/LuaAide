
# Usecases

## Metatables

    Named metatables                                           - Create a table that serves as metatable.
    ==================================================         - Save it in a unique place that can be accessed any time.
    luaL_newmetatable   create                                 - Assign it as metatable to a value on the stack.
    luaL_setmetatable   assign                              
    luaL_getmetatable   get/push                               Avoid direct access:
                                                               - Provide a constructor function that assigns a metatable
    Free metatables                                              to the object constructed from its arguments.
    ==================================================           Metatable is kept alive by attaching to Registry or to package.loaded[module].
    lua_setmetatable    assign from stack to stackvalue          Script can get access: getmetatable(newwidget(...)).
    lua_getmetatable    push from stackvalue                
                                                               or expose a metatable explicitly, allow a script to define
    Implied metatables                                         constructors of its own. Is this useful?
    ==================================================
    luaL_getmetafield   push metafield of a stackvalue         What if a script defines a table that is to be used as metatable by the host?
    luaL_callmeta       call metafunction of stackvalue

## Metamethods

    __tostring                      Missing in manual 2.4!     enum class LuaMetamethod {...};

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
