
local bpattern={
    ["/"]="b/?.so;ulutest/?.so;",
    ["\\"]="b\\?.dll;ulutest\\?.dll;",
}
package.cpath=(bpattern[package.config:sub(1,1)] or "") .. package.cpath

local ok,vec3=pcall(require, "m1")

if not ok then
    error("\n\tThis is a test suite for 'm1'."..
    "\n\tHowever, require 'm1' failed."..
    "\n\tBuild it right here.")
end

local ok,ULU=pcall(require, "ulutest")

if not ok then
    error("\n\tThis is a Unit Test implemented with 'ulutest'."..
    "\n\tHowever, require 'ulutest' failed."..
    "\n\tBuild it from luaaide.git.")
end

local TT=ULU.TT

ULU.RUN(

{
    name="Construction",
    TT("Arguments x,y,z", function(T)
        local A=vec3.New(21,22,23)
        T:ASSERT("userdata", type(A))
        -- T:ASSERT_EQ(21, A.x)
        -- T:ASSERT_EQ(22, A.y)
        -- T:ASSERT_EQ(23, A.z)
        -- T:ASSERT_EQ(21, A[1])
        -- T:ASSERT_EQ(22, A[2])
        -- T:ASSERT_EQ(23, A[3])
    end),
    TT("Argument {x,y,z}", function(T)
        local A=vec3.New {21,22,23}
        T:ASSERT("userdata", type(A))
        -- T:ASSERT_EQ(21, A.x)
        -- T:ASSERT_EQ(22, A.y)
        -- T:ASSERT_EQ(23, A.z)
        -- T:ASSERT_EQ(21, A[1])
        -- T:ASSERT_EQ(22, A[2])
        -- T:ASSERT_EQ(23, A[3])
    end),
    TT("No Argument", function(T)
        local A=vec3.New()
        T:ASSERT("userdata", type(A))
        -- T:ASSERT_EQ(0, A.x)
        -- T:ASSERT_EQ(0, A.y)
        -- T:ASSERT_EQ(0, A.z)
        -- T:ASSERT_EQ(0, A[1])
        -- T:ASSERT_EQ(0, A[2])
        -- T:ASSERT_EQ(0, A[3])
    end),
},

{
    name="String representation",
    TT("zero", function(T)
        local A=vec3.New()
        T:ASSERT_EQ("{0, 0, 0}", tostring(A))
    end),
    TT("pi", function(T)
        local A=vec3.New(math.pi, math.pi, math.pi)
        T:ASSERT_EQ("{3.14159, 3.14159, 3.14159}", tostring(A))
    end),
},

{
    name="Addition",
    TT("vec3+vec3", function(T)
        local A,B=vec3.New(1,2,3), vec3.New(10,20,30)
        local S=A+B
        T:ASSERT_EQ("{11, 22, 33}", tostring(S))
    end),
    TT("vec3+{10,20,30}", function(T)
        local A,B=vec3.New(1,2,3), {10,20,30}
        local S=A+B
        T:ASSERT_EQ("{11, 22, 33}", tostring(S))
    end),
    TT("{10,20,30}+vec3", function(T)
        local A,B={1,2,3},vec3.New(10,20,30)
        local S=A+B
        T:ASSERT_EQ("{11, 22, 33}", tostring(S))
    end),
},

{
    name="Subtraction",
    TT("vec3-vec3", function(T)
        local A,B=vec3.New(10,20,30), vec3.New(1,2,3)
        local D=A-B
        T:ASSERT_EQ("{9, 18, 27}", tostring(D))
    end),
    TT("vec3-{1,2,3}", function(T)
        local A,B=vec3.New(10,20,30), {1,2,3}
        local D=A-B
        T:ASSERT_EQ("{9, 18, 27}", tostring(D))
    end),
    TT("{10,20,30}-vec3", function(T)
        local A,B={10,20,30}, vec3.New(1,2,3)
        local D=A-B
        T:ASSERT_EQ("{9, 18, 27}", tostring(D))
    end),
}

)
