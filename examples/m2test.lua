
package.cpath=package.cpath..";b/?.so"

local function takeawhile()
    local S="ABCDEFGH"
    local T=""
    for j=1,200 do
        for k=1,1000 do
            local r=(j*k)%8
            local c=string.sub(S, 1+r, 1+r)
            T=T..c
        end
    end
    return T
end

local X=require "m2"

local t1=X.now(); print("t1", t1)
local text=takeawhile()
local t2=X.now(); print("t2", t2)
local diff=t2-t1; print("diff", diff, "msec")

-- io.output("nixda.txt"); io.write(text)
