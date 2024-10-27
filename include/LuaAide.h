
#ifndef LUAAIDE_H
#define LUAAIDE_H

#include <functional>
#include <string_view>
#include <lua.hpp>

class LuaStack;
class LuaCall;

const class LuaNil {} nil;
const class LuaError {} luaerror;

class LuaValue
{
    int stackindex;
    friend int stackindex(const LuaValue&X){ return X.stackindex; }
public:
    LuaValue(int index): stackindex(index){}
};

class LuaChunk
{
    const char*buffer{nullptr};
    unsigned bufferlength{0};
    const char*buffername{nullptr};
    friend LuaCall operator<<(LuaStack&, const LuaChunk&);
public:
    LuaChunk(const std::string_view&, const char name[]=nullptr);
    LuaChunk(const char a[], const char name[]=nullptr);
    LuaChunk(const char b[], unsigned len, const char name[]=nullptr): buffer(b), bufferlength(len), buffername(name){}
    operator bool()const { return buffer!=nullptr; }
    bool operator!()const { return buffer==nullptr; }
};

class LuaAbsIndex
{
    int absindex{0};
    friend int stackindex(const LuaAbsIndex&X){ return X.absindex; }
public:
    LuaAbsIndex(lua_State*L, int index=-1): absindex(lua_absindex(L, index)){}
    LuaAbsIndex(LuaStack&, int index=-1);
};

class LuaClosure
{
    // So legt man eine Closure auf den Stack:
    // LS<<upvalue1<<upvalue2<<LuaClosure(funcion, 2)>>LuaGlobal("closurename");
    lua_CFunction closure{nullptr};
    unsigned num_upvalues{0};
    friend LuaStack&operator<<(LuaStack&, const LuaClosure&);
public:
    LuaClosure(lua_CFunction c, unsigned numupvalues): closure(c), num_upvalues(numupvalues){}
};

//! LS<<LuaUpValue(1) legt UpValue 1 auf den Stack.
class LuaUpValue
{
    unsigned index{1};
    friend LuaStack&operator<<(LuaStack&, const LuaUpValue&);
    friend LuaCall&operator<<(LuaCall&, const LuaUpValue&);
public:
    LuaUpValue(unsigned n): index(n){}
};

class LuaGlobal
{
    friend class LuaStack;
    friend inline LuaStack&operator<<(LuaStack&, const LuaGlobal&);
    const char*name{nullptr};
public:
    LuaGlobal(const char s[]): name(s){}
};

class LuaField
{
    friend class LuaStack;
    friend class LuaCall;
    friend inline LuaStack&operator<<(LuaStack&, const LuaField&);
    const char*name{nullptr};
    bool replace_table{true}; //!< Wenn true, wird Tabelle<<LuaField("x") Tabelle auf dem Stack durch Tabelle.x ersetzen, sonst zusätzlich auf den Stack legen.
public:
    LuaField(const char s[], bool replace=false): name(s), replace_table(replace){}
};

//! Diese Klasse erleichtert den Aufruf einer Elementfunktion
//! (in Lua wäre das z.B. X:MyFunction(self, a, b, c)),
//! indem sie die Elementfunktion ermittelt und den Stack geeignet vorbereitet:
//!
//! Der Aufruf wird so realisiert:
//! Stack<<X<<a<<b<<c<<LuaColonCall("MyFunction",3)>>1;
//!
//! Vor der Ausgabe von LuaColonCall() liegt auf dem Stack
//! [X a b c]
//! Nach der Ausgabe von LuaColonCall() (Stack<<X<<a<<b<<c<<LuaColonCall("MyFunction",3))
//! liegt auf dem Stack
//! [X:MyFunction X a b c],
//! nach >>1 (wenn ein Rückgabewert erwartet wird)
//! [result]
class LuaColonCall
{
    friend LuaCall operator<<(LuaStack&, LuaColonCall&);
    const char*name{nullptr};
    unsigned numargs{0};
public:
    LuaColonCall(const char s[]): name(s){}
    LuaColonCall(const char s[], unsigned na): name(s), numargs(na){}
};

//! Diese Klasse erleichtert den Aufruf einer Funktion,
//! die in einer Struktur abgelegt ist. In Lua wäre das
//! z.B. Maintainer.func(...);
//! Das Object (z.B. Maintainer) wird beim Aufruf mit LuaStack<<LuaDotCall
//! oben auf dem Stack erwartet und vom Stack genommen.
class LuaDotCall
{
    friend LuaCall operator<<(LuaStack&, LuaDotCall&);
    const char*name{nullptr};
public:
    LuaDotCall(const char s[]): name(s){}
};

class LuaGlobalCall
{
    friend LuaCall operator<<(LuaStack&, LuaGlobalCall&);
    const char*name{nullptr};
public:
    LuaGlobalCall(const char s[]): name(s){}
};

class LuaCode
{
    friend LuaCall operator<<(LuaStack&, const LuaCode&);
    const char*text{nullptr};
public:
    LuaCode(const char s[]): text(s){}
};

class LuaTable
{
    friend LuaStack&operator<<(LuaStack&, const LuaTable&);
    unsigned numindex{0}, numfields{0};
public:
    LuaTable(){}
    LuaTable(unsigned nindex, unsigned nfields): numindex(nindex), numfields(nfields){}
};

class LuaArray: public LuaTable
{
public:
    LuaArray(unsigned numindex): LuaTable(numindex, 0){}
};

class LuaLightUserData
{
    void*data{nullptr};
    friend inline LuaStack&operator<<(LuaStack&, const LuaLightUserData&);
    friend inline LuaCall&operator<<(LuaCall&, const LuaLightUserData&);
public:
    LuaLightUserData(void*p): data(p){}
};

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

class LuaStack
{
    friend class LuaAbsIndex;
    friend unsigned height(const LuaStack&S){ return lua_gettop(S.L); }
    friend unsigned version(const LuaStack&); // Lua 5.4.6 gibt 504 zurück.
    friend inline LuaStack&operator<<(LuaStack&S, std::string_view s){ lua_pushstring(S.L, s.data()); return S; }
    friend inline LuaStack&operator<<(LuaStack&S, float x){ lua_pushnumber(S.L, x); return S; }
    friend inline LuaStack&operator<<(LuaStack&S, double x){ lua_pushnumber(S.L, x); return S; }
    friend inline LuaStack&operator<<(LuaStack&S, const LuaValue&X){ lua_pushvalue(S.L, stackindex(X)); return S; }
    friend inline LuaStack&operator<<(LuaStack&S, const LuaGlobal&X){ lua_getglobal(S.L, X.name); return S; }
    friend inline LuaStack&operator<<(LuaStack&S, const LuaAbsIndex&X){ lua_pushvalue(S.L, stackindex(X)); return S; }
    friend inline LuaStack&operator<<(LuaStack&S, const LuaNil&X){ lua_pushnil(S.L); return S; }
    friend inline LuaStack&operator<<(LuaStack&S, const LuaTable&X){ lua_createtable(S.L, X.numindex, X.numfields); return S; }
    friend inline LuaStack&operator<<(LuaStack&S, const LuaLightUserData&X){ lua_pushlightuserdata(S.L, X.data); return S; }
    friend LuaCall operator<<(LuaStack&, lua_CFunction);
    friend LuaCall operator<<(LuaStack&, const LuaChunk&);
    friend LuaCall operator<<(LuaStack&, LuaColonCall&);
    friend LuaCall operator<<(LuaStack&, LuaDotCall&);
    friend LuaCall operator<<(LuaStack&, LuaGlobalCall&);
    friend LuaCall operator<<(LuaStack&, const LuaCode&);
    friend std::ostream&operator<<(std::ostream&, const LuaStack&);

protected:
    lua_State*L{nullptr};

public:
    LuaStack(){}
    LuaStack(lua_State*L1): L(L1){}
    operator lua_State*()const{ return L; }
    LuaStack&clear();
    LuaStack&swap(); //!< Tausche die beiden obersten Werte auf dem Stack.
    LuaStack&drop(unsigned num); //!< Wenn num>height ==> Leere den Stack.
    LuaStack&dup(int was=-1){ lua_pushvalue(L, was); return*this; }

    LuaStack&operator<<(bool b){ lua_pushboolean(L, b?1:0); return*this; }
    LuaStack&operator<<(int n){ lua_pushinteger(L, n); return*this; }
    LuaStack&operator<<(unsigned n){ lua_pushinteger(L, static_cast<int>(n)); return*this; }
    LuaStack&operator<<(const char s[]){ lua_pushstring(L, s); return*this; }

    void operator>>(const LuaError&){ lua_error(L); }
    LuaStack&operator>>(const LuaGlobal&X){ lua_setglobal(L, X.name); return*this; } //!< Zuweisung an globale Variable
    LuaStack&operator>>(const LuaField&F){ lua_setfield(L, -2, F.name); if (F.replace_table) lua_remove(L, -2); return*this; }

    bool posvalid(int pos){ return (pos>0)?(pos<=lua_gettop(L)):(pos<0)?(-pos<=lua_gettop(L)):false; }

    bool hasnilat(int pos){ return posvalid(pos) && lua_isnil(L, pos)!=0; }
    bool hasstringat(int pos){ return posvalid(pos) && lua_isstring(L, pos)!=0; }
    bool hasboolat(int pos){ return posvalid(pos) && lua_isboolean(L, pos)!=0; }
    bool hasintat(int pos){ return lua_isnumber(L, pos)!=0; }
    bool hastableat(int pos){ return lua_istable(L, pos)!=0; }
    bool hasfunctionat(int pos){ return lua_isfunction(L, pos)!=0; }
    bool hasthreadat(int pos){ return lua_isthread(L, pos)!=0; }
    bool hasuserdataat(int pos){ return lua_isuserdata(L, pos)!=0; }

    const char*tostring(int pos){ return lua_tostring(L, pos); }
    bool tobool(int pos){ return lua_toboolean(L, pos)!=0; }
    long long toint(int pos){ return lua_tointeger(L, pos); }
    double todouble(int pos){ return lua_tonumber(L, pos); }

    //! dofile() und dostring() erlauben jeweils die Übergabe von Argumenten im Stil (argc, argv).
    //! Sie werden auf den Stack gelegt und sind dann mit args={...} abrufbar!
    bool dofile(const char filename[], int argc, char*argv[]);
    bool dostring(const char text[], int argc, char*argv[], const char tag[]);

    static lua_State*New(bool defaultlibs, lua_CFunction errorhandler)
    {
        auto*L=luaL_newstate();
        if (defaultlibs) luaL_openlibs(L);
        if (errorhandler!=nullptr) lua_atpanic(L, errorhandler);
        return L;
    }
    void Close()
    {
        if (L!=nullptr)
        {
            lua_close(L);
            L=nullptr;
        }
    }
};

class LuaCall: public LuaStack
{
    friend LuaCall operator<<(LuaStack&, LuaColonCall&);

    //! Dieser Konstruktor ist nur für Freunde zugänglich.
    //! Diese haben u.U. bereits Informationen auf den Stack gelegt,
    //! die am Ende entfernt werden sollen.
    LuaCall(lua_State*, unsigned start);

protected:
    LuaAbsIndex funcindex; // Stackindex der Funktion auf dem Stack. Wird benutzt, um beim Aufruf der Funktion die Anzahl der Argumente zu ermitteln.

public:
    LuaCall(lua_State*);
    LuaCall(LuaStack&);

    using LuaStack::operator>>; // Damit der folgende Operator nicht alle geerbten versteckt.

    // Aufruf der Funktion mit dem Operator >>
    int operator>>(int numresults); // Führt den Aufruf aus. Gibt rc zurück.
};
inline LuaCall&operator<<(LuaCall&S, const LuaNil&X){ static_cast<LuaStack&>(S)<<X; return S; }
inline LuaCall&operator<<(LuaCall&S, const char s[]){ static_cast<LuaStack&>(S)<<s; return S; }
inline LuaCall&operator<<(LuaCall&S, const LuaAbsIndex&X){ static_cast<LuaStack&>(S)<<X; return S; }
inline LuaCall&operator<<(LuaCall&S, lua_CFunction X){ static_cast<LuaStack&>(S)<<X; return S; }
inline LuaCall&operator<<(LuaCall&S, const LuaTable&X){ static_cast<LuaStack&>(S)<<X; return S; }
inline LuaCall&operator<<(LuaCall&S, std::function<void(LuaStack&)>ArgumentProvider){ ArgumentProvider(S); return S; }
inline LuaCall&operator<<(LuaCall&S, const LuaClosure&X){ static_cast<LuaStack&>(S)<<X; return S; }
inline LuaCall&operator<<(LuaCall&S, int n){ static_cast<LuaStack&>(S)<<n; return S; }
inline LuaCall&operator<<(LuaCall&S, unsigned n){ static_cast<LuaStack&>(S)<<n; return S; }
inline LuaCall&operator<<(LuaCall&S, bool f){ static_cast<LuaStack&>(S)<<f; return S; }
inline LuaCall&operator<<(LuaCall&S, float x){ static_cast<LuaStack&>(S)<<x; return S; }
inline LuaCall&operator<<(LuaCall&S, double x){ static_cast<LuaStack&>(S)<<x; return S; }
LuaCall&operator<<(LuaCall&, const LuaUpValue&);

class LuaDotCall2: public LuaCall
{
public:
    LuaDotCall2(lua_State*L, const char functionname[], int objectindex=-1): LuaCall(L)
    {
        lua_getfield(L, objectindex, functionname);
        lua_remove(L, objectindex-1);
    }
};

/*
    LuaIterator realisiert eine Schleife über die Elemente des obersten Objekts auf dem Stack.
    Das Äquivalent in Lua ist
        for name,value in pairs(X) do
        end
    So benutzt man LuaIterator
        LuaStack LS(...);
        for (LuaIterator K(LS); next(K); ++K)
        {
          (unsigned)K ist der einsbasierte Index.
          Bei Stack[-2] liegt der name.
          Bei Stack[-1] liegt der value.
        }
*/
class LuaIterator
{
    lua_State*L{nullptr};
    unsigned index{1}; //!< Index in Lua-Zählweise
    friend bool next(LuaIterator&X) //!< Erwartet den bisherigen Index auf dem Stack, inkrementiert ihn und legt den zugehörigen Wert auf den Stack
    {
        return lua_next(X.L, -2)!=0;
    }
public:
    LuaIterator(LuaStack&S): L(S)
    {
        // Lege nil als ungültigen Index als Startwert auf dem Stack.
        lua_pushnil(L);  // first key
    }
   ~LuaIterator()
    {
        // Nimm den Index wieder vom Stack, den der Konstruktor abgelegt hat.
    //  lua_pop(L,1);
    }
    operator unsigned(){ return index; }
    unsigned operator++()
    {
        lua_pop(L, 1);
        return++index;
    }
};

#endif
