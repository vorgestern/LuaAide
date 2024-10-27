
local X=require "luaaide"

print("luaaide version:", X.version)
print("pwd", X.pwd())

X.cd "modules/luaaide"
print("pwd 1", X.pwd())

X.cd ".."
print("pwd 2", X.pwd())

X.cd ".."
print("pwd 3", X.pwd())

X.cd "hoppla"
print("pwd 4", X.pwd())
