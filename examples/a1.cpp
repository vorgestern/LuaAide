
#include <stdio.h>
#include <LuaAide.h>

int main()
{
    LuaStack Q=LuaStack::New(true, nullptr);
    if (!Q) return printf("Failed to initialise Lua\n"),1;

    if (true)
    {
        printf("\nSimple script:\n");
        auto C=Q<<LuaCode("print 'Dies ist ein einfaches Lua Script'");
        C>>0;
    }

    if (true)
    {
        printf("\nScript with argument:\n");
        auto C=Q<<LuaCode("arg=...; print('Dies ist ein einfaches Lua Script mit Argument', arg)");
        C<<"Hoppla">>0;
    }

    if (true)
    {
        printf("\nScript with arguments:\n");
        auto C=Q<<LuaCode("args={...}; print('Dies ist ein einfaches Lua Script mit Argumenten', table.concat(args, ', '))");
        C<<"Hü"<<"Hott"<<21<<22u<<1.56f<<5.62   >>0;
    }

    if (true)
    {
        printf("\nScript with arguments:\n");
        auto C=Q<<LuaCode("local args,a={...},{}; for _,k in ipairs(args) do table.insert(a, tostring(k)) end; print('Dies ist ein einfaches Lua Script mit Argumenten', table.concat(a, ', '))");
        C<<"Hü"<<"Hott"<<21<<true<<23   >>0;
    }

    if (true)
    {
        printf("\nScript with arguments:\n");
        auto C=Q<<LuaCode(R"xxx(
            local args,a={...},{}
            -- table.concat cannot handle bool!
            for _,k in ipairs(args) do table.insert(a, tostring(k)) end
            print('Dies ist ein einfaches Lua Script mit Argumenten', table.concat(a, ', '))
        )xxx");
        C<<"Hü"<<"Hott"<<21<<true<<23   >>0;
    }

    if (true)
    {
        printf("\nScript with two return values:\n");
        auto C=Q<<LuaCode(R"xxx(
            local a,b=...
            return a+b, a-b
        )xxx");
        C<<23<<21   >>2;
        // Index stack from bottom (stack is 'private')
        const auto sum=C.toint(1), diff=C.toint(2);
        printf("Call script with 23, 21; Return values are sum=%lld, diff=%lld\n", sum, diff);
        // Index stack from top: -1 indexes top element, -2 second from top, ...
        const auto sum1=C.toint(-2), diff1=C.toint(-1);
        printf("Call script with 23, 21; Return values are sum=%lld, diff=%lld\n", sum1, diff1);
    }

    if (true)
    {
        printf("\nScript with unknown number of return values:\n");
        auto C=Q.clear()<<LuaCode(R"xxx(
            local drop,this=math.randomseed(21, 23)
            local num=math.tointeger(...)
            local X={}
            for k=1,num do table.insert(X, "item_"..tostring(k)) end
            return table.unpack(X)
        )xxx");
        C<<11   >>-1; // -1: keep all results
        const auto h=height(C);
        for (auto n=1; n<=h; ++n) printf("%d/%d: %s\n", n, h, C.tostring(-n));
    }

    if (true)
    {
        printf("\nIterate over list:\n");
        auto C=Q.clear()<<LuaCode(R"xxx(
            local drop,this=math.randomseed(21, 23)
            local num=math.tointeger(...)
            local X={}
            for k=1,num do table.insert(X, "item_"..tostring(k)) end
            return X
        )xxx");
        C<<11>>1;
        for (LuaIterator I(C); next(I); ++I)
        {
            const char*value=C.tostring(-1);
            const char*key=C.dup(-2).tostring(-1);
            printf("[%u]: '%s' '%s'\n", (unsigned)I, key, value);
            C.drop(1);
        }
    }

    if (true)
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
            const char*value=C.tostring(-1);
            const char*key=C.dup(-2).tostring(-1);
            printf("[%u]: %s '%s'\n", (unsigned)I, key, value);
            C.drop(1);
        }
    }

    if (true)
    {
        printf("\nLege eine Tabelle mit benannten Feldern an:\n");
        Q.clear()<<LuaTable()
            <<"1.2.3">>LuaField("version")
            <<"vorgestern">>LuaField("author")
            >>LuaGlobal("A");
        auto C=Q<<LuaCode(R"xxx(
            print "Felder von A:"
            for k,v in pairs(A) do print(k,v) end
        )xxx");
        C>>0;
    }

    return 0;
}
