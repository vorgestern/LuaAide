
local function resformat(S)
    local X={}
    for _,A in ipairs(S) do
        local Lf=""
        if #A.failedtests>0 then
            Lf="'"..table.concat(A.failedtests, "', '").."'"
        end
        table.insert(X, string.format("    results '%s' {passed=%d, failed=%d, failedtests={%s}}", A.name, A.passed, A.failed, Lf))
    end
    return [[

local mt={
    __tostring=function(self)
        -- print(string.format("%2d: %4d|%-4d %s (%s)", j, self.passed, self.failed, self.name, table.concat(self.failedtests, ", ")))
        return string.format("%4d|%-4d %s (%s)", self.passed, self.failed, self.name, table.concat(self.failedtests, ", "))
    end
}

-- local function results(fn) return function(...) end end
local function results(fn) return function(outcome) outcome.name=fn return setmetatable(outcome, mt) end end

local summary={
]]
..table.concat(X, ",\n").."\n}\n"..
[[

local function main(args)
    if #args>0 and args[1]=="--print" then
        print(string.format("%2s: %4s|%-4s %s (%s)", "#", "ok", "fail", "testname", "failed tests"))
        for j,r in ipairs(summary) do print(string.format("%2d: %s", j, r)) end
    end
end

main {...}
]]
end

local function main(ziel, args)
    -- print("Ziel:", ziel)
    -- print("Args:", table.concat(args, ", "))
    local S={}
    for _,f in ipairs(args) do
        local A={name=f, failedtests={}, passed=0, failed=0}
        table.insert(S, A)
        for line in io.lines(f) do
            -- print("==", line)
            local num=line:match "%[  PASSED  %]%s+(%d+)%s+tests"
            if num then A.passed=num end
            local num=line:match "%[  FAILED  %]%s+(%d+)%s+test"
            if num then A.failed=tonumber(num)
            elseif A.failed>0 then
                local name=line:match "%[  FAILED  %]%s+(.+)"
                if name then table.insert(A.failedtests, name) end
            end
        end
    end
    local T=resformat(S)
    io.output(ziel):write(T, "\n")
    io.close()
    io.output(io.stdout)
end

local args={...}
local ziel=table.remove(args, 1)
main(ziel, args)
