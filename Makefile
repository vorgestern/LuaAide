
CPPFLAGS = -Iinclude -I/usr/include/lua5.4 -I ../../../thirdparty/include

all: dir libLuaAide.a b/a1
dir:
	@mkdir -p b

libLuaAide.a: b/LuaAide.o b/LuaCall.o b/LuaChunk.o b/LuaStack.o b/stringformat.o
	ar -crs $@ $^

b/%.o: src/%.cpp include/LuaAide.h
	g++ -o $@ -c $< $(CPPFLAGS)

b/%: examples/%.cpp include/LuaAide.h
	g++ -o $@ $< $(CPPFLAGS) -L. -lLuaAide -llua5.4
