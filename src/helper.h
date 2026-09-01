
class LuaStackItem
{
    lua_State*Q;
    int index{-1};
    friend lua_State*State(const LuaStackItem&X){ return X.Q; }
    friend int Index(const LuaStackItem&X){ return X.index; }
public:
    LuaStackItem(lua_State*Q1, int index1): Q(Q1), index(index1){}
    operator bool()const{ return lua_toboolean(Q, index)!=0; }
    operator double()const{ return lua_tonumber(Q, index); }
    operator const char*()const{ return (const char*)lua_tostring(Q, index); }
    friend std::ostream&operator<<(std::ostream&, const LuaStackItem&);
};
