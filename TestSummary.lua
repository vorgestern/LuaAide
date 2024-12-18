
local summary={
    {name='Alltagstest', passed=16, failed=0, failedtests={}},
    {name='LuaAideTest', passed=49, failed=1, failedtests={'CallEnv.CallIntErrorNonemptyStacktrace'}},
    {name='m1test', passed=11, failed=0, failedtests={}},
    {name='m2test', passed=7, failed=0, failedtests={}}
}

    print(string.format("%2s: %4s|%-4s %s (%s)", "#", "ok", "fail", "testname", "failed tests"))
    for j,r in ipairs(summary) do
        if r.filenotfound then
            print(string.format("%2d: %4s|%-4s %s (file not found: '%s')", j, "-", "-", r.name, r.filenotfound))
        else
            local F=table.concat(r.failedtests, ", ")
            print(string.format("%2d: %4d|%-4d %s (%s)", j, r.passed, r.failed, r.name, F))
        end
    end
