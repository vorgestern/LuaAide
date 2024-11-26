
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

local function results(fn) return function(...) end end

local summary={
]]..table.concat(X, ",\n").."\n}"
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
    print(string.format("%2s: %4s|%-4s %s (%s)", "#", "ok", "fail", "testname", "failedtests"))
    for j,r in ipairs(S) do
        print(string.format("%2d: %4d|%-4d %s (%s)", j, r.passed, r.failed, r.name, table.concat(r.failedtests, ", ")))
    end
end

local args={...}
local ziel=table.remove(args, 1)
main(ziel, args)
