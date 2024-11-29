
#include <LuaAide.h>
#include <iostream>

// Example 'a3' for embedding Lua:
// Light Userdata
// "Printing and comparing is pretty much all userdata are good for."
// Comparing allows for table lookup though.

using namespace std;

int panichandler(lua_State*L)
{
    LuaStack Q(L);
    throw runtime_error(Q.errormessage());
    return 0;
}

static int randomkey(lua_State*L)
{
    LuaStack Q(L);
    Q<<LuaLightUserData(reinterpret_cast<void*>(100+(rand()&0xf)));
    return 1;
}

int main(int argc, char*argv[])
{
    LuaStack Q=LuaStack::New(true,  panichandler);

    srand(133);

    // Create Array 'StringIndex' of 16 elements.
    // userdata StringIndex[16]
    // local StringIndex={userdata, userdata, ...}
    Q<<LuaArray(16);
    for (auto n=0; n<16; ++n)
    {
        char pad[100];
        snprintf(pad, sizeof(pad), "String %d", n);
        Q<<LuaLightUserData(reinterpret_cast<void*>(100+n))<<pad;
        lua_settable(Q, -3);
    }
    Q>>LuaGlobal("StringIndex");

    // Create function randomkey.
    Q<<randomkey>>LuaGlobal("randomkey");

    // Execute a script that makes use of function randomkey
    // to select elements of Array 'StringIndex'.
    Q<<make_pair("DemoScript", LuaCode(R"xxx(
        for j=1,20 do
            local key=randomkey();
            print(key, StringIndex[key])
        end
    )xxx"))>>0;
    return 0;
}
