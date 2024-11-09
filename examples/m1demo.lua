
package.cpath=package.cpath..";b/?.so"

local V=require "m1"

local A,B=V.New {21, 22, 23}, V.New {210, 220, 230}

print("A", A)
print("B", B)

local C=A+B
print("C", C)

local D=B-A
print("D", D)
