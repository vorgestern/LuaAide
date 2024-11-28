
package.cpath=package.cpath..";b/?.so"

local ok,m2=pcall(require, "m2")

if not ok then
    error("\n\tThis is a test suite for module 'm2'."..
    "\n\tHowever, require 'm2' failed."..
    "\n\tBuild it right here.")
end

local ok,ULU=pcall(require, "ulutest")

if not ok then
    error("\n\tThis is a Unit Test implemented with 'ulutest'."..
    "\n\tHowever, require 'ulutest' failed."..
    "\n\tBuild it from luaaide.git.")
end

local TT=ULU.TT

local function TCASE(name)
    return function(tests)
        tests.name=name
        return tests
    end
end

ULU.RUN (
    TCASE "Exported functions" {
        TT("now", function(T) T:ASSERT_EQ(type(m2.now), "function") end),
        TT("sleep_ms", function(T) T:ASSERT_EQ(type(m2.sleep_ms), "function") end)
    },
    TCASE "Timestamp" {
        TT("has metatable", function(T) T:ASSERT(getmetatable(m2.now())) end),
        TT("is printable", function(T) T:ASSERT(getmetatable(m2.now()).__name) end),
        TT("difference is defined", function(T)
            local a=m2.now(); T:ASSERT(a)
            local b=m2.now(); T:ASSERT(b)
            T:ASSERT_EQ("number", type(b-a)) end),
    },
    TCASE "Millisecond resolution" {
        TT("sleeping 100ms", function(T)
            -- This test is autocratic in that it evaluates m2.now vs m2.sleep.
            local ta=m2.now()
            m2.sleep_ms(100)
            local tb=m2.now()
            m2.sleep_ms(10)
            local tc=m2.now()
            T:ASSERT(tb-ta>0, "should be 100 ms")
            T:ASSERT(tc-tb>0, "should be 10 ms")
            local d10=tc-tb
            T:ASSERT_EQ(10*d10, tb-ta, "tb-ta sould be 10 x tc-tb")
        end),
    },
    TCASE "Megamicrosleep" {
        TT("WorksOnMyMachine", function(T)
            local ta=os.time()
            m2.sleep_ms(510)
            local tb=os.time()
            m2.sleep_ms(510)
            local tc=os.time()
            T:EXPECT(os.difftime(tb,ta)==0 or os.difftime(tc, tb)==0)
            T:EXPECT_EQ(1, os.difftime(tc, ta))
            -- Ulutest uses the same mechanism for measuring time as this example (m2),
            -- so unsurprisingly it computes 1020ms execution time for this test.
        end),
    }
)
