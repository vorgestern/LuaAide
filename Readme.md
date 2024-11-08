
<style type="text/css">
h1 { color: red; }
table { border-collapse:collapse; }
table tr td { border: solid 1px; }
</style>

# Purpose
Substitute Lua's C-API with a C++ API that is more expressive and easier to use.

# Usecases
+ **Embedding** Lua in a program, e.g. for configuration or as a plugin
+ **Extending** Scripts by loading binary Lua-modules
+ **Exposing** C++ Types and functions to Lua scripts

# Examples

## Embedding Lua
```
    include <LuaAide.h>
    auto Q=LuaStack::New(true, nullptr);
    Q<<LuaCode("local a,b=...; return a+b")<<21<<22>>1;
    printf("C++ code receives 21+22=%d\n", Q.toint(-1));

    const std::vector<std::string> A {"Hoppla", "a list", "of strings"};
    Q<<LuaCode(R"xx(return table.concat(..., "\n"))xx")<<A>>1;
    const std::string Aconcat(Q.tostring(-1));
```

## Extending scripts
demomodule.cpp: compile/link to demomodule.so or demomodule.dll

```
    namespace {
        // These functions are implemented elsewhere in this Module:
        extern "C" int pwd(lua_Stack*);
        extern "C" int cd(lua_Stack*);
    }

    extern "C" int luaopen_demomodule(lua_State*L)
    {
        LuaStack Q(L);
        Q<<LuaTable()
            <<"0.1">>LuaField("version")
            <<pwd>>LuaField("pwd")
            <<cd>>LuaField("cd");
        return 1;
    }
```

demo.lua: use as ```lua demo.lua```

```
    local X=require "demomodule"
    print("demomodule version", X.version)
    ..
```

# Requirements
+ C++ 20
+ Lua 5.4

# How to build
## Linux
- Install requirements as you see fit. An additional requirement for Linux is objcopy (for tests).
  Check with ```make prerequisites```.
- Adapt Makefile if Lua is not at default location.
- make
- (optional) make test
- Install manually by copying libLuaAide.a and include/LuaAide.h where they belong.
  (optional: Copy luaaide.so so it's found by Lua-scripts (LUA_CPATH or LUA_CPATH_5_4))

[comment]: # ## Windows
[comment]: # - Install Visual Studio 22 and Lua 5.4
[comment]: # - Use LuaAide.sln

# Error handling
## Handling compile-errors in an application that embeds Lua
- Install a PanicHandler to translate Lua-exceptions to C++ runtime exceptions.
- Combine LuaCode with a name in a std::pair to get better error messages.

```
    #include <LuaAide.h>

    int main_throwing(lua_State*L)
    {
        LuaStack Q=L;
        // Executing this Script will fail because a parenthesis is not closed.
        Q<<make_pair("FunctioningLuaCompiletimeFailureDemo", LuaCode(R"xxx(
            function map(A, M
                local R={}
                for _,e in ipairs(A) do table.insert(R, M[e] or e) end
                return R
            end
        )xxx"))>>0;
    }

    int panichandler(lua_State*L)
    {
        LuaStack Q(L);
        throw runtime_error(Q.errormessage());
        return 0;
    }

    int main()
    {
        LuaStack Q=LuaStack::New(true, panichandler);
        try { return main_throwing(Q); }
        catch (const runtime_error&E)
        {
            printf("Runtime error:\n%s\n", E.what());
            cout<<Q<<"\n";
            return 0;
        }
    }
```

## Handling runtime-errors in an application that embeds Lua
- Throw conventional Lua-errors where applicable. Have it translated into
  a C++ runtime exception as shown above.
- Handle unexpected results from script execution by checking the return value:

```
    // This script will cause a runtime-error, because table.concat cannot handle
    // elements of type boolean.
    const auto rc=Q<<make_pair("Demo", LuaCode(R"xx(
        local a={...}
        return table.concat(a, ", ")
    )xx"))<<21<<22<<true<<false>>1;
    if (rc!=LUA_OK)
    {
        const auto message=Q.tostring(-1);
        // Use as suits your application, e.g. throw a C++ runtime-error.
    }
```

## Handling errors when Extending Scripts (i.e. in binary modules)
Throw a conventional Lua-Error, let Lua handle it:

```
    int demofunction(lua_State*L)
    {
        // Called at runtime from Lua, unhappy with arguments:
        LuaStack Q(L);
        if (height(Q)<1)
        {
            Q<<"demofunction: Argument (string) expected">>luaerror;
            return 0;
        }
        if (Q.typeat(-1)!=LUA_TSTRING)
        {
            Q<<"demofunction: string expected">>luaerror;
            return 0;
        }
        .....
    }
```

## Debugtool: Print the Stack
```
    LuaStack Q=...;
    cout<<Q;
```

[comment]: # # Sandbox to handle configuration files

[comment]: # # Similar projects
