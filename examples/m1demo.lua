
package.cpath=package.cpath..";b/?.so"

local V=require "m1"

-- print(V)
-- for k,v in pairs(V) do print(k,v) end

local A,B=V.New {21, 22, 23}, V.New {210, 220, 230}

print("A", A)
print("B", B)

local C=A+B
print("C", C)
