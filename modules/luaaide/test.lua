
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

ULU.RUN(

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
}

)
