
XFILES   := LuaAide LuaCall LuaChunk LuaStack stringformat
CPPFLAGS := -Iinclude -I/usr/include/lua5.4 -I ../../../thirdparty/include

all: dir libLuaAide.a LuaAideTest b/a1

# ============================================================

dir:
	@mkdir -p b bt

# ============================================================

libLuaAide.a: $(XFILES:%=b/%.o)
	ar -crs $@ $^

b/%.o: src/%.cpp include/LuaAide.h
	g++ -o $@ -c $< $(CPPFLAGS)

# ============================================================

LuaAideTest: src/testmain.cpp $(XFILES:%=bt/%.o)
	g++ -o $@ $^ $(CPPFLAGS) -DGTEST_HAS_PTHREAD=1 -llua5.4 -lgtest

bt/%.o: src/%.cpp include/LuaAide.h
	g++ -o $@ -c $< $(CPPFLAGS) -DUNITTEST -DGTEST_HAS_PTHREAD=1

# ============================================================

b/%: examples/%.cpp libLuaAide.a
	g++ -o $@ $< $(CPPFLAGS) -L. -lLuaAide -llua5.4
