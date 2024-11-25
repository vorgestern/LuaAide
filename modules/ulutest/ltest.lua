
--[[
        Ideen:
        + Automatische Ermittlung von Zeilennummern für die Ausgabe von Fehlermeldungen
          Lies die Dokumentation zu xpcall, "error_object"
        + Unterscheidung EXPECT/ASSERT
        + EXPECT_NIL, EXPECT_NOTNIL
        + Benenne TestCases.
        + EXPECT_EQ, ASSERT_EQ
        - Implementiere den Zugriff auf Datei und Zeilennummer mit Hilfe von debug.getinfo
--]]

local grey=not isatty(1)
local function red(str)    return grey and str or "\27[1;31m"..str.."\27[0m" end
local function blue(str)   return grey and str or "\27[1;34m"..str.."\27[0m" end
local function green(str)  return grey and str or "\27[1;32m"..str.."\27[0m" end
local function yellow(str) return grey and str or "\27[1;33m"..str.."\27[0m" end
local tags={
    RUNTEST=            blue   "[ RUN      ]",
    FAILEDTEST=         red    "[  FAILED  ]",
    PASSEDTEST=         green  "[  PASSED  ]",
    SUCCESSFULTEST=     green  "[       OK ]",
    FAILEDCRITERION=    red    "[     FAIL ]",
    SUCCESSFULCRITERION=green  "[       OK ]",
    FRAME=              blue   "[==========]",
    SEP=                blue   "[----------]",
    INFO=               yellow "[     INFO ]",
    DISABLED=           yellow "[ DISABLED ]",
    SKIPPING=           yellow "[ skipping ]"
}

local helpful_boolean=function(v)
    if v==true then return "true"
    elseif v==false then return "false"
    else return string.format("<<<unexpected boolean>>>")
    end
end

local helpful_table=function(t)
    return "table"
end

local helpful_string=function(s, maxlen)
    if string.len(s)<=maxlen then return string.format("'%s'", s)
    else return string.format("'%s...'", string.sub(s, 1, maxlen))
    end
end

local function helpful_value_representation(value)
    if type(value)=="nil" then return "nil"
    elseif type(value)=="string" then return helpful_string(value, 40)
    elseif type(value)=="number" then return tostring(value)
    elseif type(value)=="table" then return helpful_table(value)
    elseif type(value)=="boolean" then return helpful_boolean(value)
    -- elseif type(value)=="function" then return "function"
    -- elseif type(value)=="thread" then return "thread"
    -- elseif type(value)=="userdata" then return "userdata"
    else return type(value)
    end
end

local function parse_traceback(tb)
    local X={}
    for line in string.gmatch(tb, "[^\n]*") do table.insert(X, line) end
    -- return "<<<"..tostring(X[3])..">>>"
    local filepath,linenumber=string.match(X[3], "%s*([^:]+):(%d+):")
    return filepath,linenumber
end

local failedassertion=function(tb, hint)
    local path,line=parse_traceback(tb)
    if hint then return string.format("%s Failed Assertion: %s:%d: %s", tags.FAILEDCRITERION, path, line, tostring(hint))
    else return string.format("%s Failed Assertion: %s:%d.", tags.FAILEDCRITERION, path, line)
    end
end

local failedexpectation=function(tb, hint)
    local path,line=parse_traceback(tb)
    if hint then return string.format("%s Unmet Expectation: %s:%d: %s", tags.FAILEDCRITERION, path, line, tostring(hint))
    else return string.format("%s Unmet Expectation: %s:%d.", tags.FAILEDCRITERION, path, line)
    end
end

local mttest={
    EXPECT=function(self, cond, hint)
        if not cond then
            print(failedexpectation(debug.traceback("",2), hint))
            self.unmet_expectations=self.unmet_expectations+1
        else
            self.met_expectations=self.met_expectations+1
        end
    end,
    EXPECT_NIL=function(self, value, hint)
        if not value then
            self.met_expectations=self.met_expectations+1
        else
            print(failedexpectation(debug.traceback("",2), string.format("%s, but is %s", hint, helpful_value_representation(value))))
            self.unmet_expectations=self.unmet_expectations+1
        end
    end,
    EXPECT_EQ=function(self, value1, value2, hint)
        if (value1 and value2 and value1==value2) or (not value1 and not value2) then
            self.met_expectations=self.met_expectations+1
        else
            print(failedexpectation(debug.traceback("",2), string.format("%s not equal: %s, %s", hint or "", helpful_value_representation(value1), helpful_value_representation(value2))))
            self.unmet_expectations=self.unmet_expectations+1
        end
    end,
    ASSERT=function(self, cond, hint)
        if not cond then
            print(failedassertion(debug.traceback("",2), hint))
            self.failed_assertions=self.failed_assertions+1
            -- Hier geben wir error eine Tabelle,
            -- damit der Messagehandler den Fehler von einem Fehler im
            -- usercode unterscheiden kann.
            error({"Assertion failed"})
        else
            self.asserted_ok=self.asserted_ok+1
        end
    end,
    ASSERT_NIL=function(self, value, hint)
        if not value then
            self.asserted_ok=self.asserted_ok+1
        else
            print(failedassertion(debug.traceback("",2), string.format("%s, but is %s", hint, helpful_value_representation(value))))
            self.failed_assertions=self.failed_assertions+1
            -- Hier geben wir error eine Tabelle,
            -- damit der Messagehandler den Fehler von einem Fehler im
            -- usercode unterscheiden kann.
            error({"Assertion NIL failed"})
        end
    end,
    ASSERT_EQ=function(self, value1, value2, hint)
        if (value1 and value2 and value1==value2) or (not value1 and not value2) then
            self.asserted_ok=self.asserted_ok+1
        else
            print(failedassertion(debug.traceback("",2), string.format("%s not equal: %s, %s", hint, helpful_value_representation(value1), helpful_value_representation(value2))))
            self.failed_assertions=self.failed_assertions+1
            -- Hier geben wir error eine Tabelle,
            -- damit der Messagehandler den Fehler von einem Fehler im
            -- usercode unterscheiden kann.
            error({"Assertion EQ failed"})
        end
    end,
    PRINTF=function(self, fmt, ...)
        print(string.format("%s %s", tags.INFO, string.format(fmt, ...)))
    end,
}
mttest.__index=mttest

local function msghandler(msg)
    if type(msg)=="string" then
        -- Diese Fehler werden von Lua selbst erzeugt.
        return tostring(msg)
    elseif type(msg)=="table" then
        -- Diese Fehler werden von ltest.lua erzeugt.
        -- Sie sollen nicht normal formatiert werden,
        -- weil der Anwender nichts mit den Dateinamen und
        -- Zeilennummern in ltest.lua anfangen kann.
        local path,line=parse_traceback(debug.traceback("",4))
        return string.format("%s:%d: %s.", path, line, msg[1])
    else return "error of type "..type(msg)
    end
end

return {
    TT=function(name, func)
        local T=setmetatable({
            name=name,
            asserted_ok=0,
            failed_assertions=0,
            met_expectations=0,
            unmet_expectations=0
        }, mttest)
        return function(disabled)
            if disabled then
                print(string.format("%s %s", tags.SKIPPING, name))
                return
            end
            local name_skipped=T.name:match "DISABLED%s*(.*)"
            if name_skipped then
                print(string.format("%s %s", tags.SKIPPING, name_skipped))
                return
            end
            print(string.format("%s %s", tags.RUNTEST, name))
            local flag,err=xpcall(func, msghandler, T)
            if not flag then
                print(string.format("%s Test was aborted: %s", tags.FAILEDTEST, err))
            elseif T.failed_assertions>0 then
                print(string.format("%s %s", tags.FAILEDTEST, name))
            elseif T.unmet_expectations>0 then
                print(string.format("%s %s: unmet expectations", tags.FAILEDTEST, name))
            elseif T.asserted_ok+T.met_expectations>0 then
                print(tags.SUCCESSFULTEST)
            else
                print(string.format("%s Warning: Test applies no criteria", tags.SUCCESSFULTEST))
            end
        end
    end,
    RUN=function(...)
        for k,Tests in ipairs {...} do
            -- Die Tests in Tests werden mit ipairs abgefragt.
            -- Außerdem wird ein Feld 'name' beachtet.
            local testname=k
            if type(Tests)=="table" and Tests.name then
                testname=tostring(Tests.name)
            end
            local testname_disabled=testname:match "^DISABLED%s*(.*)"
            if testname_disabled then
                print(tags.DISABLED.." Skipping "..#Tests.." Tests from TestCase '"..testname_disabled.."'")
                for _,func in ipairs(Tests) do func(true) end
            else
                print(tags.FRAME.." Running "..#Tests.." Tests from TestCase '"..testname.."'")
                local nt,last=0,#Tests
                for _,func in ipairs(Tests) do
                    func()
                    nt=nt+1
                    if _<last then print(tags.SEP) end
                end
                print(tags.FRAME.." "..#Tests.." Tests finished")
            end
        end
        print("\n"..tags.SEP.." Global test environment tear-down")
        print(tags.FRAME..string.format(" %d tests from %d test cases ran.", 999, 999))
        print(tags.PASSEDTEST..string.format(" %d tests", 999))
        if 1 then
            print(tags.FAILEDTEST..string.format(" %d test%s, listed below:", 999, "S"))
            for _,nam in ipairs({"abc", "def"}) do print(tags.FAILEDTEST.." "..nam) end
            print(string.format("\n%3d FAILED TEST%s", 999, "s"))
        end
    end
}
