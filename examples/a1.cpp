
#include <stdio.h>
#include <LuaAide.h>
#include <string>
#include <vector>

// Example 'a1' for embedding Lua:

using namespace std;

int join(lua_State*L)
{
    LuaStack Q(L);
    Q   <<LuaGlobal("table")<<LuaDotCall("concat")
        <<LuaValue(1)<<LuaUpValue(1)>>1;
        return 1;
}

int main(int argc, char*argv[])
{
    const int sel=argc>1?atoi(argv[1]):-1;

    LuaStack Q=LuaStack::New(true, nullptr);
    if (!Q) return printf("Failed to initialise Lua\n"),1;

    Q<<formatany>>LuaGlobal("formatany");

    switch (sel) {

    default:
    case -1:
    {
        fprintf(stderr, "%s <n>: run demo <n> (n=0..14).\n", argv[0]);
        return 1;
    }

    case 0:
    {
        printf("\nSimple script:\n");
        auto Script=Q<<LuaCode("print 'Dies ist ein einfaches Lua Script'");
        Script>>0; // Execute Script without arguments, expecting no return values.
        break;
    }

    case 1:
    {
        printf("\nScript with argument:\n");
        auto Script=Q<<LuaCode("arg=...; print('Dies ist ein einfaches Lua Script mit Argument', arg)");
        Script<<"Hoppla">>0; // Execute scripts with one (string) argument, expecting no results.
        break;
    }

    case 2:
    {
        printf("\nScript with arguments:\n");
        auto Script=Q<<LuaCode("args={...}; print('Dies ist ein einfaches Lua Script mit Argumenten', table.concat(args, ', '))");
        Script<<"Hü"<<"Hott"<<21<<22u<<1.56f<<5.62   >>0; // Execute Script with arguments of various types, expecting no results.
        break;
    }

    case 3:
    {
        printf("\nScript with arguments:\n");
        auto Script=Q<<LuaCode("local args,a={...},{}; for _,k in ipairs(args) do table.insert(a, tostring(k)) end; print('Dies ist ein einfaches Lua Script mit Argumenten', table.concat(a, ', '))");
        Script<<"Hü"<<"Hott"<<21<<true<<23   >>0;
        break;
    }

    case 4:
    {
        printf("\nScript with arguments:\n");
        auto C=Q<<LuaCode(R"xxx(
            local args,a={...},{}
            -- table.concat cannot handle bool!
            for _,k in ipairs(args) do table.insert(a, tostring(k)) end
            print('Dies ist ein einfaches Lua Script mit Argumenten', table.concat(a, ', '))
        )xxx");
        C<<"Hü"<<"Hott"<<21<<true<<23   >>0;
        break;
    }

    case 5:
    {
        printf("\nScript with two return values:\n");
        auto Script=Q<<LuaCode(R"xxx(
            local a,b=...
            return a+b, a-b
        )xxx");
        Script<<23<<21   >>2; // Execute script with two arguments, expecting two return values.

        // Index stack from top: -1 indexes top element, -2 second from top, ...
        // Returned values are at -2 (first) and -1 (second).
        const auto sum1=Script.toint(-2), diff1=Script.toint(-1);
        printf("Call script with 23, 21; Return values are sum=%lld, diff=%lld\n", sum1, diff1);

        // Index stack from bottom (stack is 'private')
        const auto sum=Script.toint(1), diff=Script.toint(2);
        printf("Call script with 23, 21; Return values are sum=%lld, diff=%lld\n", sum, diff);
        break;
    }

    case 6:
    {
        printf("\nScript with unknown number of return values:\n");
        auto Script=Q.clear()<<LuaCode(R"xxx(
            local drop,this=math.randomseed() -- (21, 230)
            local num=math.random(10) -- num in [1,10]
            local X={}
            for k=1,num do table.insert(X, "item_"..tostring(k)) end
            -- print("==>",table.concat(X, ","),"<==")
            return table.unpack(X)
        )xxx");
        Script>>-1; // -1 (==LUA_MULTRET): keep all results
        const int h=height(Script);
        for (int n=1; n<=h; ++n){ printf("%d/%d: %s\n", n, h, Script.tostring(-n).c_str()); }
        break;
    }

    case 7:
    {
        printf("\nIterate over list:\n");
        auto Script=Q.clear()<<LuaCode(R"xxx(
            local drop,this=math.randomseed(21, 23)
            local num=math.tointeger(...)
            local X={}
            for k=1,num do table.insert(X, "item_"..tostring(k)) end
            return X
        )xxx");
        Script<<11>>1;
        for (LuaIterator I(Q); next(I); ++I)
        {
            auto value=Q.tostring(-1);
            auto key=Q.dup(-2).tostring(-1);
            printf("[%u]: '%s' '%s'\n", (unsigned)I, key.c_str(), value.c_str());
            Q.drop(1);
        }
        break;
    }

    case 8:
    {
        printf("\nIterate over table:\n");
        auto C=Q.clear()<<LuaCode(R"xxx(
            local X={}
            X.name="LuaAide"
            X.typ="table"
            X.meta={date="2024-10-25"}
            X[1]=21
            X[2]=22
            X[4]=24
            return X
        )xxx");
        C<<11>>1;
        for (LuaIterator I(C); next(I); ++I)
        {
            auto value=C.tostring(-1);
            auto key=C.dup(-2).tostring(-1);
            printf("[%u]: %s '%s'\n", (unsigned)I, key.c_str(), value.c_str());
            C.drop(1);
        }
        break;
    }

    case 9:
    {
        printf("\nLege eine Tabelle mit benannten Feldern an:\n");
        Q.clear()<<newtable
            <<"1.2.3">>LuaField("version")
            <<"vorgestern">>LuaField("author")
            >>LuaGlobal("A");
        auto C=Q<<LuaCode(R"xxx(
            print "Felder von A:"
            for k,v in pairs(A) do print(k,v) end
        )xxx");
        C>>0;
        break;
    }

    case 10:
    {
        auto Script=Q<<LuaCode("local args={...}; for j,v in ipairs(args) do print(j,formatany(v)) end");
        Q<<lualist<<21<<22<<23;
        Q<<LuaTable()
            <<1.5>>LuaField("x")
            <<0.7>>LuaField("y")
            <<-2.1>>LuaField("z");
        Q<<vector<string> {
            "A", "B", "C"
        };
        Q<<unordered_map<string,string> {
            {"x", "21"}, {"y", "21"}, {"z", "23"}
        };
        Script>>0;
        break;
    }

    case 11:
    {
        Q   <<lualist<<21<<22<<23<<lualistend
            >>LuaGlobal("L1");
        Q<<LuaCode("print(formatany(L1))")>>0;

        Q<<LuaCode("return 21")>>1;
        auto result=Q.toint(-1);
        printf("%lld\n", result);
        break;
    }

    case 12:
    {
        // Q<<lua_error<<"This was not expected">>0;
        vector<string> A={"a", "b", "c"};
        Q<<formatany<<A>>1;
        auto str=Q.tostring(-1);
        printf("str='%s'\n", str.c_str());
        break;
    }

    case 13:
    {
        Q<<", "<<LuaClosure({join, 1})>>LuaGlobal("KommaJoin");
        Q<<"-" <<LuaClosure({join, 1})>>LuaGlobal("HyphJoin");
        Q<<LuaCode(R"__(
            local A={"a", "b", "c"}
            print(KommaJoin(A))
            print(HyphJoin(A))
        )__")>>0;
        break;
    }

    case 14:
    {
        auto Script=Q<<LuaCode("local args={...}; for j,v in ipairs(args) do print(j,formatany(v)) end");

        Q   <<LuaGlobal("string")<<LuaDotCall("format")
            <<"vector=[%s, %s, %s]"<<21<<22<<23>>1;

        Q   <<lualist<<"First"<<"Second";
        Q   <<LuaGlobal("table")<<LuaDotCall("concat")
            <<LuaValue(-2)<<"+">>1;

        Script>>0;

        break;
    }

    }

    return 0;
}
