
local bpattern={
    ["/"]="b/?.so;ulutest/?.so;",
    ["\\"]="b\\?.dll;ulutest\\?.dll;",
}
package.cpath=(bpattern[package.config:sub(1,1)] or "") .. package.cpath

local ok,m3=pcall(require, "m3")

if not ok then
    error("\n\tThis is a test suite for module 'm3'."..
    "\n\tHowever, require 'm3' failed."..
    "\n\tBuild it right here.")
end

local ok,ULU=pcall(require, "ulutest")

if not ok then
    error("\n\tThis is a Unit Test implemented with 'ulutest'."..
    "\n\tHowever, require 'ulutest' failed."..
    "\n\tBuild it as a submodule right here.")
end

local TT=ULU.TT

local function CASE(name)
    return function(tests)
        tests.name=name
        return tests
    end
end

ULU.RUN {
    CASE "Exported by module m3" {
        TT("version",   function(T) T:ASSERT_EQ("string", type(m3.version)) end),
        TT("origin",    function(T) T:ASSERT_EQ("string", type(m3.origin)) end),
        TT("colortype", function(T) T:ASSERT_EQ("table",  type(m3.colortype)) end)
    },
    CASE "Values of enum colortype" {
        TT("Has expected keys", function(T)
            T:ASSERT(m3.colortype.LCT_GREY)
            T:ASSERT(m3.colortype.LCT_RGB)
            T:ASSERT(m3.colortype.LCT_PALETTE)
            T:ASSERT(m3.colortype.LCT_GREY_ALPHA)
            T:ASSERT(m3.colortype.LCT_RGBA)
        end),
        TT("Numeric values as expected", function(T)
            T:ASSERT_EQ(0, m3.colortype.LCT_GREY:numeric())
            T:ASSERT_EQ(2, m3.colortype.LCT_RGB:numeric())
            T:ASSERT_EQ(3, m3.colortype.LCT_PALETTE:numeric())
            T:ASSERT_EQ(4, m3.colortype.LCT_GREY_ALPHA:numeric())
            T:ASSERT_EQ(6, m3.colortype.LCT_RGBA:numeric())
        end),
    },
    CASE "Iteration over enum colortype" {
        TT("pairs", function(T)
            local K,V,Keys,Values={},{},{},{}
            for k,v in pairs(m3.colortype) do
                K[tostring(k)]=1
                V[tostring(v)]=1
                table.insert(Keys, k)
                table.insert(Values, tostring(v))
            end
            local SKeys=table.concat(Keys, ", ")
            local SValues=table.concat(Values, ", ")
            T:ASSERT(K.LCT_GREY, SKeys)
            T:ASSERT(K.LCT_RGB, SKeys)
            T:ASSERT(K.LCT_PALETTE, SKeys)
            T:ASSERT(K.LCT_GREY_ALPHA, SKeys)
            T:ASSERT(K.LCT_RGBA, SKeys)
            T:ASSERT(V.LCT_GREY, SValues)
            T:ASSERT(V.LCT_RGB, SValues)
            T:ASSERT(V.LCT_PALETTE, SValues)
            T:ASSERT(V.LCT_GREY_ALPHA, SValues)
            T:ASSERT(V.LCT_RGBA, SValues)
            T:ASSERT_EQ(5, #Keys)
            T:ASSERT_EQ(5, #Values)
        end)
    }
}
