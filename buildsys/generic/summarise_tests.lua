
local function resformat(S)
    local X={}
    for _,A in ipairs(S) do
        if A.filenotfound then
            table.insert(X, string.format("{name='%s', filenotfound='%s'}", A.name, A.filenotfound))
        else
            local Lf=""
            if #A.failedtests>0 then
                Lf="'"..table.concat(A.failedtests, "', '").."'"
            end
            table.insert(X, string.format("{name='%s', passed=%d, failed=%d, failedtests={%s}}", A.name, A.passed, A.failed, Lf))
        end
    end
    return "local summary={\n    "..table.concat(X, ",\n    ").."\n}"
end

local function main(ziel, args)
    local S={}
    for _,f in ipairs(args) do
        local stem=f:match "^.*[/\\]([^/\\]*)%.result" or f:match "^(.*)%.result"
        local A={name=stem, failedtests={}, passed=0, failed=0, filenotfound=nil}
        table.insert(S, A)
        local checkfile=io.open(f, "r")
        if not checkfile then
            A.filenotfound=f
        else
            checkfile:close()
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
    end
    local T=resformat(S)
    io.output(ziel):write("\n", T, "\n\n", [[
    print(string.format("%2s: %4s|%-4s %s (%s)", "#", "ok", "fail", "testname", "failed tests"))
    for j,r in ipairs(summary) do
        if r.filenotfound then
            print(string.format("%2d: %4s|%-4s %s (file not found: '%s')", j, "-", "-", r.name, r.filenotfound))
        else
            local F=table.concat(r.failedtests, ", ")
            print(string.format("%2d: %4d|%-4d %s (%s)", j, r.passed, r.failed, r.name, F))
        end
    end
]])
    io.close()
end

local args={...}
local ziel=table.remove(args, 1)
main(ziel, args)
