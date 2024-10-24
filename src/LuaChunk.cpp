
#include <LuaAide.h>
#include <cstring>

LuaChunk::LuaChunk(const char a[], const char name[]): buffer(a), bufferlength(strlen(a)), buffername(name){}
LuaChunk::LuaChunk(const std::string_view&a, const char name[]): buffer(a.data()), bufferlength(a.length()), buffername(name){}
