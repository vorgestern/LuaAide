
local X=require "alltag"

local demos={

    function() print("Using module 'alltag', version:", X.version) end,

    function()
        print "Demo module 'alltag', function 'formatany'"
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
    end,

    function()
        print "Demo module 'alltag', function 'formatany', edge-cases"
        local eins={}
        local e={1}
        local K={
            21, 22, 23,
            a=31, b=32, c=33,
            [3.1415926]="pi7", [3.1415926535]="pi10",
            ["abc def"]="String mit Leerzeichen",
            ["abc\ndef"]="String mit Zeilenschaltung",
            [true]="bool (true)",
            [false]="bool (false)",
            [e]="list (1)",
            [{1, 2}]="list (1, 2)",
            [eins]="list eins"
        }
        print(X.formatany(K))
        for _,k in ipairs {1, 2, 3, "a", "b", "c",
                3.1415926, 3.1415926535, 3.141592653577,
                "abc def", "abc\ndef", e, eins, true, false} do
            print(k, K[k]);
        end
    end,

    function()
        print(X.formatany({1,2,3}, {4,5,6}))
    end,

    function()
        local A={
            k1="This is a string of considerable length",
            k2=21,
            k3={
                "This is a string of considerable length,",
                "but wait, ...",
                "there is more!"
            }
        }
        A.k4=table.concat(A.k3, " ")
        print(X.formatany(A, {4,5,6}, A, B, A, "more", 21.3, A))
    end,

}

local arg=...
local was=math.tointeger(arg)

if was and demos[was] then demos[was]()
elseif was then print(string.format("There is no demo %s, try 1..%d.", was, #demos))
elseif arg then print(string.format("Argument '%s' not accepted, try 1..%d.", arg, #demos))
else print("This script requires an argument 1.."..#demos)
end
