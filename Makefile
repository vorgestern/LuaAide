
XFILES   := LuaCall LuaStack formatany keyescape streamout
XHEADER  := include/LuaAide.h
CPPFLAGS := -Iinclude -I/usr/include/lua5.4 -I ../../../thirdparty/include
CXXFLAGS := --std=c++20 -Wall -Werror
BT       := buildsys/gcc/bt
.PHONY: clean dir prerequisites test

all: prerequisites dir libLuaAide.a LuaAideTest \
        b/simplescripts b/errorhandling b/lightuserdata b/embedding_cppclass \
        b/vec3.so b/m2.so b/m3.so ulutest/ulutest.so
clean:
	@rm -rf b $(BT) libLuaAide.a LuaAideTest ulutest/ulutest.so
prerequisites:
	@which objcopy > /dev/null || echo "objcopy not installed (required to build ulutest)" || false
dir:
	@mkdir -p b buildsys/gcc/bt
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

b/simplescripts: examples/simplescripts.cpp libLuaAide.a $(XHEADER)
	@echo $<
	@g++ -o $@ $< $(CPPFLAGS) $(CXXFLAGS) -L. -lLuaAide -llua5.4
b/errorhandling: examples/errorhandling.cpp libLuaAide.a $(XHEADER)
	@echo $<
	@g++ -o $@ $< $(CPPFLAGS) $(CXXFLAGS) -L. -lLuaAide -llua5.4
b/lightuserdata: examples/lightuserdata.cpp libLuaAide.a $(XHEADER)
	@echo $<
	@g++ -o $@ $< $(CPPFLAGS) $(CXXFLAGS) -L. -lLuaAide -llua5.4
b/embedding_cppclass: examples/embedding_cppclass.cpp libLuaAide.a $(XHEADER)
	@echo $<
	@g++ -o $@ $< $(CPPFLAGS) $(CXXFLAGS) -L. -lLuaAide -llua5.4

b/a%: examples/a%.cpp libLuaAide.a $(XHEADER)
	@echo $<
	@g++ -o $@ $< $(CPPFLAGS) $(CXXFLAGS) -L. -lLuaAide -llua5.4

b/vec3.so: examples/module_vec3.cpp libLuaAide.a $(HEADER)
	g++ -shared -fpic -o $@ $^ $(CPPFLAGS) $(CXXFLAGS)

b/m%.so: examples/m%.cpp libLuaAide.a $(HEADER)
	g++ -shared -fpic -o $@ $^ $(CPPFLAGS) $(CXXFLAGS)

# ============================================================

ulutest/ulutest.so:
	make -C ulutest

# ============================================================

$(BT)/LuaAideTest.result: ./LuaAideTest
	@./LuaAideTest > $@
$(BT)/moduletest_vec3.result: examples/moduletest_vec3.lua b/vec3.so
	@lua $< > $@
$(BT)/m2test.result: examples/m2test.lua b/m2.so
	@lua $< > $@
TestSummary.lua: $(BT)/LuaAideTest.result $(BT)/moduletest_vec3.result $(BT)/m2test.result
	@lua buildsys/generic/summarise_tests.lua $@ $^

# ============================================================

lspcommands:
	@bear -- make -B all
