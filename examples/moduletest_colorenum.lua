
local bpattern={
    ["/"]="b/?.so;ulutest/?.so;",
    ["\\"]="b\\?.dll;ulutest\\?.dll;",
}
package.cpath=(bpattern[package.config:sub(1,1)] or "") .. package.cpath

local ok,colorenum=pcall(require, "colorenum")

if not ok then
    error("\n\tThis is a test suite for module 'colorenum'."..
    "\n\tHowever, require 'colorenum' failed."..
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
    CASE "Exported by module colorenum" {
        TT("version",   function(T) T:ASSERT_EQ("string", type(colorenum.version)) end),
        TT("origin",    function(T) T:ASSERT_EQ("string", type(colorenum.origin)) end),
        TT("colortype", function(T) T:ASSERT_EQ("table",  type(colorenum.colortype)) end)
    },
    CASE "Values of enum colortype" {
        TT("Has expected keys", function(T)
            T:ASSERT(colorenum.colortype.LCT_GREY)
            T:ASSERT(colorenum.colortype.LCT_RGB)
            T:ASSERT(colorenum.colortype.LCT_PALETTE)
            T:ASSERT(colorenum.colortype.LCT_GREY_ALPHA)
            T:ASSERT(colorenum.colortype.LCT_RGBA)
        end),
        TT("Numeric values as expected", function(T)
            T:ASSERT_EQ(0, colorenum.colortype.LCT_GREY:numeric())
            T:ASSERT_EQ(2, colorenum.colortype.LCT_RGB:numeric())
            T:ASSERT_EQ(3, colorenum.colortype.LCT_PALETTE:numeric())
            T:ASSERT_EQ(4, colorenum.colortype.LCT_GREY_ALPHA:numeric())
            T:ASSERT_EQ(6, colorenum.colortype.LCT_RGBA:numeric())
        end),
    },
    CASE "Iteration over enum colortype" {
        TT("pairs", function(T)
            local K,V,Keys,Values={},{},{},{}
            for k,v in pairs(colorenum.colortype) do
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
