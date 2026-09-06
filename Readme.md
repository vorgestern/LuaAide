
# Purpose & Status
Provide a C++ substitute for Lua's C-API that is more expressive and easier to use.

- Works as documented
- Purpose more or less fulfilled
- User will occasionally use Lua API directly to fill gaps in the usecases covered by LuaAide.
- Work continues to cover more usecases.

# Usecases
+ **Embedding** Lua in a program, e.g. for configuration or as a plugin
+ **Extending** Scripts by loading binary Lua-modules written with LuaAide
+ **Exposing** C++ Types and functions to Lua scripts

## Embedding Lua
Use Lua in your application for a variety of purposes: To make it scriptable,
to implement a configuration format, to offer a plugin-interface, to use the
Lua-format for data input/output and so forth.

    #include <LuaAide.h>
    auto Q=LuaStack::New(true, nullptr);                         // Initialise Lua.
    Q<<LuaCode("local a,b=...; return a+b")<<21<<22>>1;          // Execute a piece of Lua code, passing arguments and ..
    printf("C++ code receives 21+22=%d\n", Q.toint(-1));         // .. receiving result on the stack.

    const std::vector<std::string> A {"Hoppla", "a list", "of strings"};         // Use Lua's table.concat to ..
    Q<<LuaCode(R"__(return table.concat(..., "\n"))__")<<A>>1;                   // concatenate C++ strings.
    const std::string A1(Q.tostring(-1));    // == "Hoppla\na list\nof strings"

## Extending scripts
Use C++ to create modules that can be loaded into Lua scripts, benefitting from
performance, static typing, access to C/C++ libraries etc.

Example demomodule.cpp: compile/link to demomodule.so or demomodule.dll

    namespace {
        // These functions are implemented elsewhere in this module.
        // They are exposed to the Lua runtime in luaopen_demomodule.
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

Use demomodule from a script:

    local demo=require "demomodule"
    print("demomodule version", demo.version)
    ..

## Exposing C/C++ Types
Take advantage of C++ strong typing by implementing specific types in C++
and exposing them to Lua as modules.

Several examples are included in the examples folder:
module_vec3.cpp, module_timestamp.lua, module_colorenum.cpp

# Requirements
+ C++ 20
+ Lua 5.4

# How to build

## First clone including submodules ..

    git clone --recurse-submodules --remote-submodules https://github.com/vorgestern/luaaide.git

## .. then on Linux
- Adapt Makefile if Lua is not at default location.
- Run ```make```
- Run ```make test``` (*optional*)
  This executes ./LuaAideTest (tests of the static library) and test scripts for the example modules.
- Install LuaAide manually by copying libLuaAide.a and include/LuaAide.h where they belong.

## .. else on Windows
- Edit buildsys/VS17/Lua.props to point to your Lua-Installation:
  * **AdditionalIncludeDirectories**: Include the directory that contains lua.hpp.
  * **AdditionalLibraryDirectories**: Include the directory where the import library is located.
  * **AdditionalDependencies**: Include the import library for Lua 5.4.
- LuaAide.lib (Release|Win32) will be built in the root directory, others under buildsys/VS17.
  Select a different Configuration|Platform in buildsys/VS17/LuaAide.props.
- Build with Visual Studio 2022 (VS17) or later by launching buildsys/VS17/LuaAide.sln

# How to use

In these examples, it is assumed that Q ist an instance of LuaStack, e.g. from `LuaStack Q(L)`
or `auto Q=LuaStack::New(true, nullptr);`.

## Creating lists and tables

    using namespace std;
    Q<<lualist<<21<<22<<23;                         // Pushes {21, 22, 23} on to the stack.

    Q<<LuaTable()<<1.5>>LuaField("x")               // Pushes {x=1.5, y=0.7, z=-2.1} on to the stack.
                <<0.7>>LuaField("y")
                <<-2.1>>LuaField("z");

    Q<<vector<string> {"A", "B", "C"};              // Pushes {"A", "B", "C"} on the stack.

    Q<<unordered_map<string,string> {               // Pushes {x="21", y="22", z="23"} on to the stack.
        {"x", "21"}, {"y", "21"}, {"z", "23"}
    };

### Storing values in global Lua variables

    Q   <<lualist<<21<<22<<23<<lualistend
        >>LuaGlobal("L1");                          // L1={21, 22, 23} is a global variable.

## Accessing data on the Lua stack from C++

    Q<<LuaCode("return 21")>>1;                     // Execute a script to push one result.
    auto result=Q.toint(-1);                        // Read value on top of the stack as integer.
                                                    // Result will be 21.

## Calling Lua functions

    using namespace std;
    Q<<"This was not expected">>luaerror;           // Equiv. of 'error "This was not expected"'

    vector<string> A={"a", "b", "c"};
    Q<<formatany<<A>>1;                             // formatany is part of LuaAide.
    auto str=Q.tostring(-1);                        // It converts (nearly) any value to a string.

## Creating closures

    int join(lua_State*L)                                   // demofunction: table.concat with separator in upvalue
    {
        LuaStack Q(L);
        Q   <<LuaGlobal("table")<<LuaDotCall("concat")      // local arg=...
            <<LuaValue(1)<<LuaUpValue(1)>>1;                // return table.concat(arg, up1)
        return 1;
    }
    Q<<", "<<LuaClosure({join, 1})>>LuaGlobal("KommaJoin"); // Create Closure that joins with comma.
    Q<<"-" <<LuaClosure({join, 1})>>LuaGlobal("HyphJoin");  // Create Closure that joins with hyphens.

    Q<<LuaCode(R"__(                                        // Execute demo script
        local A={"a", "b", "c"}
        print(KommaJoin(A))                                 // prints "a, b, c"
        print(HyphJoin(A))                                  // prints "a-b-c"
    )__")>>0;

## Calling functions from the Lua runtime

    Q<<LuaGlobal("string")<<LuaDotCall("format")    // Pushes "vector=[21, 22, 23]" on to the stack.
     <<"vector=[%s, %s, %s]"<<21<<22<<23>>1;

    Q<<lualist<<"First"<<"Second";                  // Pushes {"First", "Second"} on to the stack.
    Q<<LuaGlobal("table")<<LuaDotCall("concat")
     <<LuaValue(-2)<<"+">>1;                        // Calls table.concat on the list, i.e. pushes
                                                    // "First+Second" on to the stack.

## Error handling
### Handling compile-errors in an application that embeds Lua
- Install a PanicHandler to translate Lua-exceptions to C++ runtime exceptions.
- Combine LuaCode with a name in a std::pair to get better error messages.
    ```
    #include <LuaAide.h>
    using namespace std;

    int main_throwing(lua_State*L)
    {
        LuaStack Q=L;
        // Executing this Script will fail because a parenthesis is not closed.
        Q<<make_pair("FunctioningLuaCompiletimeFailureDemo", LuaCode(R"___(
            function map(A, M
                local R={}
                for _,e in ipairs(A) do table.insert(R, M[e] or e) end
                return R
            end
        )___"))>>0;
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

### Handling runtime-errors in an application that embeds Lua
- Throw conventional Lua-errors where applicable. Have them translated to
  a C++ runtime exception as shown above.
- Handle unexpected results from script execution by checking the return value:

    ```
    // This script will cause a runtime-error, because table.concat cannot handle
    // elements of type boolean.
    const auto rc=Q<<make_pair("Demo", LuaCode(R"___(
        local a={...}
        return table.concat(a, ", ")
    )___"))<<21<<22<<true<<false>>1;
    if (rc!=LUA_OK)
    {
        const auto message=Q.tostring(-1);
        // Use as suits your application, e.g. throw a C++ runtime-error.
    }
    ```

### Handling errors when Extending Scripts (i.e. in binary modules)
Throw a conventional Lua-Error, let Lua handle it:

    ```
    int demofunction(lua_State*L)
    {
        // Called at runtime from Lua, unhappy with arguments:
        LuaStack Q(L);
        if (height(Q)<1) return Q<<"demofunction: Argument (string) expected">>luaerror;
        if (Q.typeat(-1)!=LuaType::TSTRING) return Q<<"demofunction: string expected">>luaerror;
        .....
    }
    ```

### Debugtool: Print the Stack

    LuaStack Q=...;
    cout<<Q;
