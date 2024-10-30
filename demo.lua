
local X=require "luaaide"

if nil then
    print("luaaide version:", X.version)
end

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
--  print("1", X.formatany("abc"))
--  print("2", X.formatany(21.43))
--  print("3", X.formatany(21))
--  print("4", X.formatany(true))
--  print("5", X.formatany(false))
--  print("6", X.formatany(nil))
--  print("7", X.formatany(function(x) return x end))
--  print("8", X.formatany({a=1, b=2}))
--  print("9", X.formatany({a=1, b={101, 102, 103}}))
print(X.formatany {
    a={
        aa={
            aaa="Hier ist aaa",
            aab="Hier ist aab"
        },
        ab={
            aba="Hier ist aba",
            abb="Hier ist abb"
        }
    },
    b={21, 22, 23, 24, 25, 26, 27, 28, 29, 30, "abcdefg", "hijklmnop", "qrstuvw", "mehr", "und", "noch", "mehr", "und", "immer", "noch", "mehr", "bis", "es", "reicht"},
    c=21,
    d=function(x) return x end
})
end
