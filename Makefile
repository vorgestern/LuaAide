
XFILES   := LuaCall LuaStack formatany keyescape streamout keys map apply other
XHEADER  := include/LuaAide.h
CPPFLAGS := -Iinclude -I/usr/include/lua5.4 -I ../../../thirdparty/include
CXXFLAGS := --std=c++20 -Wall
# -Werror
BT       := buildsys/gcc/bt
.PHONY: clean dir test

all: dir libLuaAide.a ulutest/ulutest.so LuaAideTest \
        b/simplescripts b/errorhandling b/lightuserdata b/embedding_cppclass \
        b/vec3.so b/timestamp.so b/colorenum.so \
	b/hilf1
clean:
	@rm -rf b $(BT) libLuaAide.a LuaAideTest ulutest/ulutest.so
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

LuaAideTest: src/testmain.cpp src/testpush.cpp $(XFILES:%=$(BT)/%.o)
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

b/vec3.so: examples/module_vec3.cpp libLuaAide.a $(HEADER)
	g++ -shared -fpic -o $@ $^ $(CPPFLAGS) $(CXXFLAGS)
b/timestamp.so: examples/module_timestamp.cpp libLuaAide.a $(HEADER)
	g++ -shared -fpic -o $@ $^ $(CPPFLAGS) $(CXXFLAGS)
b/colorenum.so: examples/module_colorenum.cpp libLuaAide.a $(HEADER)
	g++ -shared -fpic -o $@ $^ $(CPPFLAGS) $(CXXFLAGS)

# ============================================================

ulutest/ulutest.so:
	make -C ulutest

# ============================================================

$(BT)/LuaAideTest.result: ./LuaAideTest
	@./LuaAideTest > $@
$(BT)/moduletest_vec3.result: examples/moduletest_vec3.lua b/vec3.so
	@lua $< > $@
$(BT)/moduletest_timestamp.result: examples/moduletest_timestamp.lua b/timestamp.so
	@lua $< > $@
$(BT)/moduletest_colorenum.result: examples/moduletest_colorenum.lua b/colorenum.so
	@lua $< > $@
TestSummary.lua: $(BT)/LuaAideTest.result $(BT)/moduletest_vec3.result $(BT)/moduletest_timestamp.result $(BT)/moduletest_colorenum.result
	@lua buildsys/generic/summarise_tests.lua $@ $^

# ============================================================

lspcommands:
	@bear -- make -B all

b/hilf1: examples/hilf1.cpp libLuaAide.a $(XHEADER)
	@echo $<
	@g++ -o $@ $< $(CPPFLAGS) $(CXXFLAGS) -L. -lLuaAide -llua5.4
