
#include <LuaAide.h>
#include <iostream>

// a3: LuaLightUserData

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

    Q<<LuaArray(16);
    for (auto n=0; n<16; ++n)
    {
        char pad[100];
        sprintf(pad, "String %d", n);
        Q<<LuaLightUserData(reinterpret_cast<void*>(100+n))<<pad;
        lua_settable(Q, -3);
    }
    Q>>LuaGlobal("StringIndex");
    Q<<randomkey>>LuaGlobal("randomkey");

    Q<<make_pair("DemoScript", LuaCode(R"xxx(
        for j=1,20 do
            local key=randomkey();
            print(key, StringIndex[key])
        end
    )xxx"))>>0;
    return 0;
}
