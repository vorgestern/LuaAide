
local X=require "luaaide"
-- print("X", X)
-- for k,v in pairs(X or {}) do print(k,v) end

print("luaaide version:", X.version)
print("pwd", X.pwd())

X.cd "src99"
print("pwd 1", X.pwd())
