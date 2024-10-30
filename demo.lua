
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

if nil then
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
        b={{}, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, "abcdefg", "hijklmnop", "qrstuvw", "mehr", "und", "noch", "mehr", "und", "immer", "noch", "mehr", "bis", "es", "reicht"},
        c=21,
        d=function(x) return x end,
        e={
            {
                {
                    {
                        {
                            {
                                {{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{}, {{{{{{{{{}}}}}}}}}}}}}}}},{}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
                            }
                        }
                    }
                }
            }
        }
    })
end

if true then
    local eins={}
    local K={
        21, 22, 23,
        a=31, b=32, c=33,
        [3.1415926]="pi7", [3.1415926535]="pi10",
        ["abc def"]="String mit Leerzeichen",
        ["abc\ndef"]="String mit Zeilenschaltung",
        [true]="bool (true)",
        [false]="bool (false)",
        [{1}]="list (1)",
        [{1, 2}]="list (1, 2)",
        [eins]="list eins"
    }
    print(X.formatany(K))
    for _,k in ipairs {1, 2, 3, "a", "b", "c",
            3.1415926, 3.1415926535, 3.141592653577,
            "abc def", "abc\ndef", {1}, eins, true, false} do
        print(k, K[k]);
    end
end
