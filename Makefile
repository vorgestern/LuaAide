
# 1 Schaffe LuaChunk ab.
# 2 Lege vor lua_error eine Stringdarstellung des Stacks global ab.
# 3 Benutze luaL_loadbufferx statt lua_load.
# 4 Bearbeite alle Vorkommen von lua_error() und >>luaerror.
# 5 Beachte: lua_error leert den Stack bis auf die Fehlermeldung.

XFILES   := LuaAide LuaCall LuaChunk LuaStack stringformat
XHEADER  := include/LuaAide.h
CPPFLAGS := -Iinclude -I/usr/include/lua5.4 -I ../../../thirdparty/include
CXXFLAGS := --std=c++20 -Wall -Werror

all: dir libLuaAide.a LuaAideTest b/a1 b/a2 luaaide.so
clean:
	@rm -rf b/* bt/* libLuaAide.a LuaAideTest

# ============================================================

dir:
	@mkdir -p b/luaaide bt

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

luaaide.so: b/luaaide/main.o b/luaaide/formatany.o libLuaAide.a
	g++ -shared -fpic -o $@ $^

b/luaaide/main.o: modules/luaaide/main.cpp $(XHEADER)
	g++ -c -Wall -Werror -fpic -o $@ $< $(CPPFLAGS) $(CXXFLAGS)

b/luaaide/formatany.o: modules/luaaide/formatany.cpp $(XHEADER)
	g++ -c -Wall -Werror -fpic -o $@ $< $(CPPFLAGS) $(CXXFLAGS)
