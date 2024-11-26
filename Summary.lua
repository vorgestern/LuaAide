
local summary={
    {name='Alltagstest.result', passed=18, failed=0, failedtests={}},
    {name='LuaAideTest.result', passed=43, failed=0, failedtests={}}
}

    print(string.format("%2s: %4s|%-4s %s (%s)", "#", "ok", "fail", "testname", "failed tests"))
    for j,r in ipairs(summary) do
        local F=table.concat(r.failedtests, ", ")
        print(string.format("%2d: %4d|%-4d %s (%s)", j, r.passed, r.failed, r.name, F))
    end
