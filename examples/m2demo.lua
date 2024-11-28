
-- Find m2.so in b/m2.so
package.cpath=package.cpath..";b/?.so"

local X=require "m2"

local t1=X.now();
X.sleep_ms(1501)
local t2=X.now();

local diff=t2-t1;

print("t1", t1)
print("t2", t2)
print("diff", diff, "msec")
