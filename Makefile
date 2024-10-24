
CPPFLAGS = -Iinclude -I/usr/include/lua5.4 -I ../../../thirdparty/include

all: dir libLuaAide.a
dir:
	mkdir -p b

libLuaAide.a: b/LuaAide.o b/LuaCall.o b/LuaChunk.o b/LuaStack.o b/stringformat.o
	ar -crs $@ $^

b/%.o: src/%.cpp
	g++ -o $@ -c $< $(CPPFLAGS)
