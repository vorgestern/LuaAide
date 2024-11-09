
print "\n******** test luaaide ********"

local ok,luaaide=pcall(require, "luaaide")

if not ok then
    error("\n\tThis is a test suite for 'luaaide'."..
    "\n\tHowever, require 'luaaide' failed."..
    "\n\tInstall and build it from luaaide.git.")
end

local ok,ULU=pcall(require, "ulutest")

if not ok then
    error("\n\tThis is a Unit Test implemented with 'ulutest'."..
    "\n\tHowever, require 'ulutest' failed."..
    "\n\tInstall it from luaaide.git.")
end

local TT=ULU.TT

local function setup1(c) return string.format("abc%cdef", c) end

ULU.RUN(

{
    name="Version",
    TT("version present", function(T)
        T:ASSERT_EQ(type(luaaide.version), "string")
    end),
    TT("version sufficient", function(T)
        local maj,min=luaaide.version:match "^(%d+)%.(%d+)"
        local Maj,Min=math.tointeger(maj),math.tointeger(min)
        T:ASSERT(100*Maj+Min>=1) -- Version mindestens 0.1
    end)
},

{
    name="Functions present",
    TT("pwd present", function(T)
        T:ASSERT_EQ(type(luaaide.pwd), "function")
    end),
    TT("cd present", function(T)
        T:ASSERT_EQ(type(luaaide.cd), "function")
    end),
    TT("formatany present", function(T)
        T:ASSERT_EQ(type(luaaide.formatany), "function")
    end),
    TT("keyescape present", function(T)
        T:ASSERT_EQ(type(luaaide.keyescape), "function")
    end),
},

{
    name="keyescape",
    TT("escape bell",      function(T) T:ASSERT_EQ('["abc\\adef"]', luaaide.keyescape(setup1(7))) end),
    TT("escape backspace", function(T) T:ASSERT_EQ('["abc\\bdef"]', luaaide.keyescape(setup1(8))) end),
    TT("escape tab",       function(T) T:ASSERT_EQ('["abc\\tdef"]', luaaide.keyescape(setup1(9))) end),
    TT("escape lf",        function(T) T:ASSERT_EQ('["abc\\ndef"]', luaaide.keyescape(setup1(10))) end),
    TT("escape ff",        function(T) T:ASSERT_EQ('["abc\\fdef"]', luaaide.keyescape(setup1(12))) end),
    TT("escape cr",        function(T) T:ASSERT_EQ('["abc\\rdef"]', luaaide.keyescape(setup1(13))) end),
    TT("escape escape",    function(T) T:ASSERT_EQ('["abc\\edef"]', luaaide.keyescape(setup1(27))) end),
    TT("escape 21",        function(T) T:ASSERT_EQ('["abc\\x15def"]', luaaide.keyescape(setup1(21))) end),
    TT("escape space",     function(T) T:ASSERT_EQ('["abc def"]',   luaaide.keyescape("abc def")) end),
    TT("escape dot",       function(T) T:ASSERT_EQ('["abc.def"]',   luaaide.keyescape("abc.def")) end),
--  TT("escape ä",         function(T) T:ASSERT_EQ('["abcädef"]', luaaide.keyescape("abcädef")) end),
--  TT("escape ö",         function(T) T:ASSERT_EQ('["abcödef"]', luaaide.keyescape("abcödef")) end),
--  TT("escape ü",         function(T) T:ASSERT_EQ('["abcüdef"]', luaaide.keyescape("abcüdef")) end),
--  TT("escape Ä",         function(T) T:ASSERT_EQ('["abcÄdef"]', luaaide.keyescape("abcÄdef")) end),
--  TT("escape Ö",         function(T) T:ASSERT_EQ('["abcÖdef"]', luaaide.keyescape("abcÖdef")) end),
--  TT("escape Ü",         function(T) T:ASSERT_EQ('["abcÜdef"]', luaaide.keyescape("abcÜdef")) end),
--  TT("escape ß",         function(T) T:ASSERT_EQ('["abcßdef"]', luaaide.keyescape("abcßdef")) end),
},

{
    name="formatany",
    TT("1", function(T)
        local X=luaaide.formatany {21,22,23}
        local ok,R=pcall(load, X)
        T:ASSERT(ok)
        R=R()
        T:ASSERT_EQ(R[1], 21)
        T:ASSERT_EQ(R[2], 22)
        T:ASSERT_EQ(R[3], 23)
    end),
    TT("2", function(T)
        local X=luaaide.formatany {a={aa={aaa=111}, ab={aba=121}}, b={ba=21}}
        local ok,R=pcall(load, X)
        T:ASSERT(ok)
        R=R()
        T:ASSERT_EQ(R.a.aa.aaa, 111)
        T:ASSERT_EQ(R.a.ab.aba, 121)
        T:ASSERT_EQ(R.b.ba, 21)
    end),
}

)
