
# 1 + Schaffe LuaChunk ab.
# 2   Lege vor lua_error eine Stringdarstellung des Stacks global ab.
# 3 + Benutze luaL_loadbufferx statt lua_load.
# 4   Bearbeite alle Vorkommen von lua_error() und >>luaerror.
# 5   Beachte: lua_error leert den Stack bis auf die Fehlermeldung.
# 6   Steuere die Objekterstellung mit objcopy so, dass unabhängig vom Zielpfad immer der gewählte Name verwendet wird.

XFILES   := LuaAide LuaCall LuaStack stringformat
XHEADER  := include/LuaAide.h
CPPFLAGS := -Iinclude -I/usr/include/lua5.4 -I ../../../thirdparty/include
CXXFLAGS := --std=c++20 -Wall -Werror

all: dir libLuaAide.a LuaAideTest b/a1 b/a2 luaaide.so ulutest.so
clean:
	@rm -rf b/* bt/* libLuaAide.a LuaAideTest luaaide.so ulutest.so

# ============================================================

dir:
	@mkdir -p b/luaaide b/ulutest bt

# ============================================================

libLuaAide.a: $(XFILES:%=b/%.o)
	@echo $<
	@ar -crs $@ $^

b/%.o: src/%.cpp $(XHEADER)
	@echo $<
	@g++ -fpic -o $@ -c $< $(CPPFLAGS) $(CXXFLAGS)

# ============================================================

LuaAideTest: src/testmain.cpp $(XFILES:%=bt/%.o)
	@echo $<
	@g++ -o $@ $^ $(CPPFLAGS) $(CXXFLAGS) -DUNITTEST -DGTEST_HAS_PTHREAD=1 -llua5.4 -lgtest

bt/%.o: src/%.cpp $(XHEADER)
	@echo $<
	@g++ -o $@ -c $< $(CPPFLAGS) $(CXXFLAGS) -DUNITTEST -DGTEST_HAS_PTHREAD=1

# ============================================================

b/%: examples/%.cpp libLuaAide.a $(XHEADER)
	@echo $<
	@g++ -o $@ $< $(CPPFLAGS) $(CXXFLAGS) -L. -lLuaAide -llua5.4

# ============================================================

luaaide.so: b/luaaide/main.o b/luaaide/formatany.o b/luaaide/keyescape.o libLuaAide.a
	g++ -shared -fpic -o $@ $^

b/luaaide/main.o: modules/luaaide/main.cpp $(XHEADER)
	g++ -c -Wall -Werror -fpic -o $@ $< $(CPPFLAGS) $(CXXFLAGS)

b/luaaide/formatany.o: modules/luaaide/formatany.cpp $(XHEADER)
	g++ -c -Wall -Werror -fpic -o $@ $< $(CPPFLAGS) $(CXXFLAGS)

b/luaaide/keyescape.o: modules/luaaide/keyescape.cpp $(XHEADER)
	g++ -c -Wall -Werror -fpic -o $@ $< $(CPPFLAGS) $(CXXFLAGS)

# ============================================================

ulutest.so: b/ulutest/main.o b/ulutest/ltest.o libLuaAide.a
	g++ -shared -fpic -o $@ $^

b/ulutest/main.o: modules/ulutest/main.cpp
	g++ -o $@ -c $< -fpic $(CPPFLAGS)

b/ulutest/ltest.o: b/ulutest/ltest.luac
	objcopy -I binary -O elf64-x86-64 --redefine-syms=modules/ulutest/syminfo $< $@

b/ulutest/ltest.luac: modules/ulutest/ltest.lua
	luac -o $@ $<
