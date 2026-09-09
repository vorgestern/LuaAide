
#include <LuaAide.h>
#include <cstdlib>
#include <iostream>

using namespace std;

[[maybe_unused]] static int panichandler(lua_State*L)
{
    LuaStack Q(L);
    throw runtime_error(Q.errormessage());
    return 0;
}

// Testfälle:
// Subjekt Typ table, string, number, userdata
//             10     20      30      40

// 0: print existiert nicht
// 1: print ist ein Element, aber keine Funktion
// 2: print existiert und ist eine Funktion
// 3: print ist eine Funktion der Metatabelle
// 4: print ist ein Element der Metatabelle, aber keine Funktion

void cases_table(lua_State*L, int opt)
{
        LuaStack Q=L;
        switch (opt)
        {
                case 0:
                {
                        printf("table 0\n");
                        Q.clear();
                        Q<<LuaCode("return {}")>>1;
                        Q<<LuaMethod("print")>>0;
                        cout<<"Stack:\n"<<Q<<"\n";
                        break;
                }
                case 1:
                {
                        printf("table 1\n");
                        Q.clear();
                        Q<<LuaCode("return {print=21}")>>1;
                        Q<<LuaMethod("print")>>0;
                        cout<<"Stack:\n"<<Q<<"\n";
                        break;
                }
                case 2:
                {
                        printf("table 2\n");
                        Q.clear();
                        Q<<LuaCode("return {data='data', print=function(self) print('self.data:', self.data) end}")>>1;
                        Q<<LuaMethod("print")>>0;
                        cout<<"Stack:\n"<<Q<<"\n";
                        break;
                }
                case 3:
                {
                        printf("table 3\n");
                        Q.clear();
                        Q<<LuaCode("local mt={print=function(self) print('self.data:', self.data) end}; return setmetatable({data='data999'}, mt)")>>1;
                        Q<<LuaMethod("print")>>0;
                        cout<<"Stack:\n"<<Q<<"\n";
                        break;
                }
                case 4:
                {
                        printf("table 4\n");
                        Q.clear();
                        Q<<LuaCode("local mt={print=21}; return setmetatable({data='data999'}, mt)")>>1;
                        Q<<LuaMethod("print")>>0;
                        cout<<"Stack:\n"<<Q<<"\n";
                        break;
                }
        }
}

void cases_string(lua_State*L, int opt)
{
        LuaStack Q=L;
        switch (opt)
        {
                case 0:
                {
                        printf("string 0\n");
                        Q.clear();
                        Q<<"hallo"<<LuaMethod("print")>>0;
                        cout<<"Stack:\n"<<Q<<"\n";
                        break;
                }
        }
}

void cases_int(lua_State*L, int opt)
{
        LuaStack Q=L;
        switch (opt)
        {
                case 0:
                {
                        printf("int 0\n");
                        Q.clear();
                        Q<<21<<LuaMethod("print")>>0;
                        cout<<"Stack:\n"<<Q<<"\n";
                        break;
                }
        }
}

void cases_userdata(lua_State*L, int opt)
{
        LuaStack Q=L;
        switch (opt)
        {
                case 0:
                {
                        printf("userdata 0\n");
                        Q.clear();
                        [[maybe_unused]] auto*v=lua_newuserdatauv(Q, sizeof(void*), 0);
                        Q<<LuaMethod("print")>>0;
                        break;
                }
                case 3:
                {
                        printf("userdata 3\n");
                        Q.clear();
                        [[maybe_unused]] auto*v=lua_newuserdatauv(Q, sizeof(void*), 0);
                        const auto Userdata=Q.index(-1);
                        Q<<LuaCode("return {print=function(self) print(99+1) end}")>>1;
                        lua_setmetatable(Q, stackindex(Userdata));
                        Q<<Userdata<<LuaMethod("print")>>0;
                        break;
                }
                case 4:
                {
                        printf("userdata 4\n");
                        Q.clear();
                        [[maybe_unused]] auto*v=lua_newuserdatauv(Q, sizeof(void*), 0);
                        const auto Userdata=Q.index(-1);
                        Q<<LuaCode("return {print=21}")>>1;
                        lua_setmetatable(Q, stackindex(Userdata));
                        Q<<Userdata<<LuaMethod("print")>>0;
                        break;
                }
        }
}

int main_throwing(lua_State*L, int opt)
{
        LuaStack Q=L;

        if      (opt>=10 && opt<20) cases_table(Q, opt-10);
        else if (opt>=20 && opt<30) cases_string(Q, opt-20);
        else if (opt>=30 && opt<40) cases_string(Q, opt-30);
        else if (opt>=40 && opt<50) cases_userdata(Q, opt-40);
        else switch (opt)
        {
                case 120:
                {
                        Q.clear();
                        Q<<"hello";
                        if (const auto t1=lua_getfield(Q, -1, "print"); t1==LUA_TNIL)
                        {
                                Q<<"LuaMethod: Field 'print' is not present.";
                                lua_error(Q);
                        }
                        break;
                }

                case 1:
                {
                        Q.clear();
                        Q<<"21 22 23"<<LuaMethod("match")<<"(%d+) (%d+) (%d+)">>3;
                        cout<<"Stack:\n"<<Q<<"\n";
                        break;
                }

                case 3:
                {
                        printf("opt=3\n");
                        Q.clear();
                        Q<<21<<LuaMethod("print")>>0;
                        cout<<"Stack:\n"<<Q<<"\n";
                        break;
                }

                default:
                {
                        fprintf(stderr, "hilf1: opt=%d unknown. (Use 0..2).\n", opt);
                }
        }
        return 0;
}

int main(int argc, char*argv[])
{
        int opt=0;
        if (argc>1)
        {
                opt=atoi(argv[1]);
        }
        LuaStack Q=LuaStack::New(true, panichandler);

        try { main_throwing(Q, opt); }
        catch (const runtime_error&E)
        {
                cerr<<"caught: "<<E.what()<<"\n";
        }
        catch (...)
        {
                cerr<<"caught unexpected exception\n";
        }
        return 0;
}
