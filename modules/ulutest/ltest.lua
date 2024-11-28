
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

local bind=...
local FAILEDCRITERION=bind.tags.FAILEDCRITERION
local INFO=bind.tags.INFO
local SKIPPING=bind.tags.SKIPPING
local RUNTEST=bind.tags.RUNTEST
local FAILEDTEST=bind.tags.FAILEDTEST
local SUCCESSFULTEST=bind.tags.SUCCESSFULTEST
local FRAME=bind.tags.FRAME
local SEP=bind.tags.SEP
local DISABLED=bind.tags.DISABLED
local PASSEDTEST=bind.tags.PASSEDTEST

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
    if hint then return string.format("%s Failed Assertion: %s:%d: %s", FAILEDCRITERION, path, line, tostring(hint))
    else return string.format("%s Failed Assertion: %s:%d.", FAILEDCRITERION, path, line)
    end
end

local failedexpectation=function(tb, hint)
    local path,line=parse_traceback(tb)
    if hint then return string.format("%s Unmet Expectation: %s:%d: %s", FAILEDCRITERION, path, line, tostring(hint))
    else return string.format("%s Unmet Expectation: %s:%d.", FAILEDCRITERION, path, line)
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
        print(string.format("%s %s", INFO, string.format(fmt, ...)))
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

local function singularplural(num, name)
    if num==1 then return "1 "..name
    else return num.." "..name.."s"
    end
end

-- Leider braucht es bisher diesen 'globalen' Zustand.
local testcase_running=""

return {
    tags=bind.tags,
    timestamp=bind.timestamp,
    isatty=bind.isatty,
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
                print(string.format("%s %s", SKIPPING, name))
                return {name=T.name, outcome="skipped", duration=0}
            end
            local name_skipped=T.name:match "DISABLED%s*(.*)"
            if name_skipped then
                print(string.format("%s %s", SKIPPING, name_skipped))
                return {name=T.name, outcome="disabled", duration=0}
            end
            local testdotname=testcase_running.."."..name
            print(string.format("%s %s", RUNTEST, testdotname))
            local ta=bind.timestamp()
            local flag,err=xpcall(func, msghandler, T)
            local tb=bind.timestamp()
            local dur_test=tb-ta
            if not flag then
                print(string.format("%s Test was aborted: %s", FAILEDTEST, err))
                return {name=T.name, outcome="aborted", duration=tb-ta}
            elseif T.failed_assertions>0 then
                print(string.format("%s %s", FAILEDTEST, name))
                return {name=T.name, outcome="failed", duration=tb-ta}
            elseif T.unmet_expectations>0 then
                print(string.format("%s %s: unmet expectations", FAILEDTEST, name))
                return {name=T.name, outcome="unexpected", duration=tb-ta, unmet_expectations=T.unmet_expectations}
            elseif T.asserted_ok+T.met_expectations>0 then
                print(SUCCESSFULTEST.." "..testdotname.." ("..dur_test.." ms)")
                return {name=T.name, outcome="successful", duration=tb-ta}
            else
                print(string.format("%s Warning: Test applies no criteria", SUCCESSFULTEST))
                return {name=T.name, outcome="void", duration=tb-ta}
            end
        end
    end,
    RUN=function(...)
        local Summary={
            testcases=0,
            tests=0,
            passed=0, Failed={},
            skipped=0,
            Notplausible={}
        }
        local OutcomeClass={
            aborted=0, failed=0, unexpected=0,
            successful=1, void=1,
            disabled=10, skipped=10
        }
        local aggregate=function(R, testcasename, duration)
            Summary.tests=Summary.tests+1
            if not R.outcome or not R.name then
                table.insert(Summary.notplausible, testcasename.."."..(R.name or "unnamed (unplausible)"))
            else
                local f=OutcomeClass[R.outcome]
                if f==0 then
                    table.insert(Summary.Failed, testcasename.."."..R.name)
                elseif f==1 then
                    Summary.passed=Summary.passed+1
                elseif f==10 then
                    Summary.skipped=Summary.skipped+1
                end
            end
        end
        local Testcases={...}
        local total=0
        for j,k in ipairs(Testcases) do total=total+#k end
        print(FRAME.." Running "..singularplural(total, "test").." from "..singularplural(#Testcases, "test case")..".")
        print(SEP.." Global test environment set-up.")
        local Ta=bind.timestamp()
        for k,Testcase in ipairs(Testcases) do
            Summary.testcases=Summary.testcases+1
            -- Die Tests in Testcase werden mit ipairs abgefragt.
            -- Außerdem wird ein Feld 'name' beachtet.
            local testcasename=k
            if type(Testcase)=="table" and Testcase.name then
                testcasename=tostring(Testcase.name)
            end
            local testname_disabled=testcasename:match "^DISABLED%s*(.*)"
            if testname_disabled then
                print(DISABLED.." Skipping "..singularplural(#Testcase, "test").." from "..testname_disabled)
                for _,func in ipairs(Testcase) do
                    aggregate(func(true), testcasename, 0)
                end
            else
                local step=k>1 and "\n" or ""
                print(step..SEP.." "..singularplural(#Testcase, "test").." from "..testcasename)
                testcase_running=testcasename
                local nt,last=0,#Testcase
                local ta=bind.timestamp()
                for _,func in ipairs(Testcase) do
                    local R=func()
                    aggregate(R, testcasename)
                    nt=nt+1
                    -- if _<last then print(SEP) end
                end
                local tb=bind.timestamp()
                local dur_testcase=tb-ta
                print(SEP.." "..singularplural(#Testcase, "test").." from "..testcasename.." ("..dur_testcase.." ms total)")
            end
        end
        local Tb=bind.timestamp()
        local dur_total=Tb-Ta

        print("\n"..SEP.." Global test environment tear-down")
        print(FRAME..string.format(" %s from %s ran. (%d ms total)", singularplural(Summary.tests, "test"), singularplural(Summary.testcases, "test case"), dur_total))
        local wennplural="s"
        if Summary.passed==1 then wennplural="" end
        print(PASSEDTEST.." "..singularplural(Summary.passed, "test"))
        local nf=#Summary.Failed
        if nf>0 then
            print(FAILEDTEST..string.format(" %s, listed below:", singularplural(nf, "test")))
            for _,nam in ipairs(Summary.Failed) do print(FAILEDTEST.." "..nam) end
            print(string.format("\n%3d FAILED TEST%s", nf, wennplural))
        end
        local nu=#Summary.Notplausible
        if nu>0 then
            print(FAILEDTEST..string.format(" %s, listed below:", singularplural(nu, "test")))
            for _,nam in ipairs(Summary.Notplausible) do print(FAILEDTEST.." "..nam) end
            print(string.format("\n%3d TEST%s processed with unplausible outcome", nf, wennplural))
        end
    end
}
