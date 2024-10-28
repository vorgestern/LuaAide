
local X=require "luaaide"

print("luaaide version:", X.version)

if nil then
    print("pwd", X.pwd())

    X.cd "modules/luaaide"
    print("pwd 1", X.pwd())

    X.cd ".."
    print("pwd 2", X.pwd())

    -- X.stackdump();

    X.cd ".."
    print("pwd 3", X.pwd())

    X.cd "hoppla"
    print("pwd 4", X.pwd())
end

if true then
    print("1", X.formatany("abc"))
    print("2", X.formatany(21.43))
    print("3", X.formatany(21))
    print("4", X.formatany(true))
    print("5", X.formatany(false))
    print("6", X.formatany(nil))
    print("7", X.formatany(function(x) return x end))
    print("8", X.formatany({a=1, b=2, 10, 11, 12}))
end
