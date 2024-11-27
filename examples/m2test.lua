
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

package.cpath=package.cpath..";b/?.so"

require "m2"

-- print("tshighres", tshighres)
-- print("\n\n\n")

local t1=tshighres(); print("t1", t1)
local text=takeawhile()
local t2=tshighres(); print("t2", t2)
local d=t2-t1; print("d", d)

-- io.output("nixdat.txt"); io.write(text)
