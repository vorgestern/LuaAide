
XFILES   := LuaCall LuaStack formatany keyescape streamout
XHEADER  := include/LuaAide.h
CPPFLAGS := -Iinclude -I/usr/include/lua5.4 -I ../../../thirdparty/include
CXXFLAGS := --std=c++20 -Wall -Werror
BT       := buildsys/gcc/bt
.PHONY: clean dir prerequisites test

all: prerequisites dir libLuaAide.a LuaAideTest b/a1 b/a2 b/a3 b/a4 b/m1.so b/m2.so alltag.so ulutest/ulutest.so
clean:
	@rm -rf b/* $(BT) libLuaAide.a LuaAideTest alltag.so ulutest/ulutest.so
prerequisites:
	@which objcopy > /dev/null || echo "objcopy not installed (required to build ulutest)" || false
dir:
	@mkdir -p b/alltag buildsys/gcc/bt
test: TestSummary.lua
	@lua $< --print

# ============================================================

libLuaAide.a: $(XFILES:%=b/%.o)
	@echo $<
	@ar -crs $@ $^
b/%.o: src/%.cpp $(XHEADER)
	@echo $<
	@g++ -fpic -o $@ -c $< $(CPPFLAGS) $(CXXFLAGS)

# ============================================================

LuaAideTest: src/testmain.cpp $(XFILES:%=$(BT)/%.o)
	@echo $<
	@g++ -o $@ $^ $(CPPFLAGS) $(CXXFLAGS) -DUNITTEST -DGTEST_HAS_PTHREAD=1 -llua5.4 -lgtest
$(BT)/%.o: src/%.cpp $(XHEADER)
	@echo $<
	@g++ -o $@ -c $< $(CPPFLAGS) $(CXXFLAGS) -DUNITTEST -DGTEST_HAS_PTHREAD=1

# ============================================================

b/a%: examples/a%.cpp libLuaAide.a $(XHEADER)
	@echo $<
	@g++ -o $@ $< $(CPPFLAGS) $(CXXFLAGS) -L. -lLuaAide -llua5.4
b/m%.so: examples/m%.cpp libLuaAide.a $(HEADER)
	g++ -shared -fpic -o $@ $^ $(CPPFLAGS) $(CXXFLAGS)

# ============================================================

alltag.so: b/alltag/main.o libLuaAide.a
	g++ -shared -fpic -o $@ $^
b/alltag/%.o: modules/alltag/%.cpp $(XHEADER)
	g++ -c -Wall -Werror -fpic -o $@ $< $(CPPFLAGS) $(CXXFLAGS)

# ============================================================

ulutest/ulutest.so:
	make -C ulutest

# ============================================================

$(BT)/LuaAideTest.result: ./LuaAideTest
	@./LuaAideTest > $@
$(BT)/Alltagstest.result: modules/alltag/Alltagstest.lua
	@lua $< > $@
$(BT)/m1test.result: examples/m1test.lua b/m2.so
	@lua $< > $@
$(BT)/m2test.result: examples/m2test.lua b/m2.so
	@lua $< > $@
TestSummary.lua: $(BT)/Alltagstest.result $(BT)/LuaAideTest.result $(BT)/m1test.result $(BT)/m2test.result
	@lua buildsys/generic/summarise_tests.lua $@ $^
