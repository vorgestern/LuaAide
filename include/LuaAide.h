
#ifndef LUAAIDE_H
#define LUAAIDE_H

#include <functional>
#include <string_view>
#include <string>
#include <unordered_map>
#include <lua.hpp>

class LuaStack;
class LuaCall;

const class LuaNil {} nil;
const class LuaError {} luaerror;
const class LuaSwap {} luaswap;
struct LuaRotate { int index; };
constexpr const LuaRotate luarot_3 {-3}; // X a b   ==> a b X
constexpr const LuaRotate luarot_4 {-4}; // X a b c ==> a b c X
constexpr const LuaRotate luarot3  { 3}; // a b X   ==> X a b
constexpr const LuaRotate luarot4  { 4}; // a b c X ==> X a b c

class LuaValue
{
    int stackindex;
    friend int stackindex(const LuaValue&X){ return X.stackindex; }
public:
    LuaValue(int index): stackindex(index){}
};

class LuaAbsIndex
{
    friend class LuaStack;
    int absindex{0};
    friend int stackindex(const LuaAbsIndex&X){ return X.absindex; }
    LuaAbsIndex(int j): absindex(j){}
};

class LuaUpValue
{
    friend class LuaStack;
    //! LS<<LuaUpValue(1) legt UpValue 1 auf den Stack.
    unsigned index{1};
public:
    LuaUpValue(unsigned n): index(n){}
};

class LuaLightUserData
{
    friend class LuaStack;
    void*data{nullptr};
public:
    LuaLightUserData(void*p): data(p){}
};

class LuaChunk
{
    friend class LuaStack;
    const char*buffer{nullptr};
    unsigned bufferlength{0};
    const char*buffername{nullptr};
public:
    LuaChunk(const std::string_view&, const char name[]=nullptr);
    LuaChunk(const char a[], const char name[]=nullptr);
    LuaChunk(const char b[], unsigned len, const char name[]=nullptr): buffer(b), bufferlength(len), buffername(name){}
    operator bool()const { return buffer!=nullptr; }
    bool operator!()const { return buffer==nullptr; }
};

class LuaClosure
{
    friend class LuaStack;
    // So legt man eine Closure auf den Stack:
    // LS<<upvalue1<<upvalue2<<LuaClosure(funcion, 2)>>LuaGlobal("closurename");
    lua_CFunction closure{nullptr};
    unsigned num_upvalues{0};
public:
    LuaClosure(lua_CFunction c, unsigned numupvalues): closure(c), num_upvalues(numupvalues){}
};

class LuaGlobal
{
    friend class LuaStack;
    const char*name{nullptr};
public:
    LuaGlobal(const char s[]): name(s){}
};

class LuaField
{
    friend class LuaStack;
    friend class LuaCall;
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
    friend class LuaStack;
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
    friend class LuaStack;
    const char*name{nullptr};
public:
    LuaDotCall(const char s[]): name(s){}
};

class LuaGlobalCall
{
    friend class LuaStack;
    const char*name{nullptr};
public:
    LuaGlobalCall(const char s[]): name(s){}
};

class LuaCode
{
    friend class LuaStack;
    const char*text{nullptr};
public:
    LuaCode(const char s[]): text(s){}
};

class LuaTable
{
    friend class LuaStack;
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

    LuaAbsIndex index(int n){ return LuaAbsIndex(lua_absindex(L, n)); }

    LuaStack&operator<<(LuaSwap){ lua_rotate(L, -2, 1); return*this; }
    LuaStack&operator<<(LuaRotate X){
        if (X.index<0) lua_rotate(L, X.index, -1);
        else if (X.index>0) lua_rotate(L, -X.index, 1);
        return*this;
    }

    LuaStack&operator<<(bool b){ lua_pushboolean(L, b?1:0); return*this; }
    LuaStack&operator<<(int n){ lua_pushinteger(L, n); return*this; }
    LuaStack&operator<<(unsigned n){ lua_pushinteger(L, static_cast<int>(n)); return*this; }
    LuaStack&operator<<(const char s[]){ lua_pushstring(L, s); return*this; }
    LuaStack&operator<<(std::string_view s){ lua_pushstring(L, s.data()); return*this; }
    LuaStack&operator<<(float x){ lua_pushnumber(L, x); return*this; }
    LuaStack&operator<<(double x){ lua_pushnumber(L, x); return*this; }
    LuaStack&operator<<(const LuaValue&X){ lua_pushvalue(L, stackindex(X)); return*this; }
    LuaStack&operator<<(const LuaUpValue&V){ lua_pushvalue(L, lua_upvalueindex(V.index)); return*this; }
    LuaStack&operator<<(const LuaGlobal&X){ lua_getglobal(L, X.name); return*this; }
    LuaStack&operator<<(const LuaAbsIndex&X){ lua_pushvalue(L, stackindex(X)); return*this; }
    LuaStack&operator<<(const LuaNil&X){ lua_pushnil(L); return*this; }
    LuaStack&operator<<(const LuaTable&X){ lua_createtable(L, X.numindex, X.numfields); return*this; }
    LuaStack&operator<<(const LuaLightUserData&X){ lua_pushlightuserdata(L, X.data); return*this; }
    LuaStack&operator<<(const std::vector<std::string>&);
    LuaStack&operator<<(const std::unordered_map<std::string, std::string>&);
    LuaCall  operator<<(const LuaCode&);
    LuaCall  operator<<(const std::pair<std::string_view, const LuaCode&>&); // chunkname first, chunk second
    LuaCall  operator<<(lua_CFunction);
    LuaCall  operator<<(const LuaChunk&);
    LuaCall  operator<<(const LuaColonCall&);
    LuaCall  operator<<(const LuaDotCall&);
    LuaCall  operator<<(const LuaGlobalCall&);
    LuaCall  operator<<(const LuaClosure&);

    void operator>>(const LuaError&){ lua_error(L); }
    LuaStack&operator>>(const LuaGlobal&X){ lua_setglobal(L, X.name); return*this; } //!< Zuweisung an globale Variable
    LuaStack&operator>>(const LuaField&F){ lua_setfield(L, -2, F.name); if (F.replace_table) lua_remove(L, -2); return*this; }

    bool posvalid(int pos){ return (pos>0)?(pos<=lua_gettop(L)):(pos<0)?(-pos<=lua_gettop(L)):false; }

    bool hasnilat(int pos){ return posvalid(pos) && lua_isnil(L, pos)!=0; }
    bool hasstringat(int pos){ return posvalid(pos) && lua_isstring(L, pos)!=0; }
    bool hasboolat(int pos){ return posvalid(pos) && lua_isboolean(L, pos)!=0; }
    bool hasintat(int pos){ return lua_isnumber(L, pos)!=0; }
    bool hastableat(int pos){ return lua_istable(L, pos)!=0; }
    bool hasfunctionat(int pos){ return posvalid(pos) && lua_isfunction(L, pos)!=0; }
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

    std::string stringrepr(int index)const;
};

class LuaCall: public LuaStack
{
    friend class LuaStack;

    //! Dieser Konstruktor ist nur für Freunde zugänglich.
    //! Diese haben u.U. bereits Informationen auf den Stack gelegt, die am Ende entfernt werden sollen.
    LuaCall(lua_State*, LuaAbsIndex);

protected:
    LuaAbsIndex funcindex; // Stackindex der Funktion auf dem Stack. Wird benutzt, um beim Aufruf der Funktion die Anzahl der Argumente zu ermitteln.

public:
    LuaCall(lua_State*);
    LuaCall(LuaStack&);

    using LuaStack::operator>>;     // Damit der folgende Operator nicht alle geerbten verdeckt.
    int operator>>(int numresults); // Führt den Aufruf aus. Gibt rc zurück.

    int operator>>(std::pair<int,int>);
        // Führt den Aufruf mit numresults='second' aus,
        // entfernt 'first' Elemente, die vor dem LuaCall auf dem Stack gelegen haben.
        // Dies vereinfacht die Wiederverwendung von Stackelementen für einen LuaCall.
        // Beispiel:
        // Q<<a;
        // Q<<function<<LuaValue(-2)>>make_pair(1,1);
        // Dabei hat a aufgrund vorausgegangener Aktionen bereits auf dem Stack gelegen.
        // Jetzt soll es mit 'function' weiterverarbeitet werden.
        // Man darf es aber nicht mit Q<<function<<rotate(-2)>>1 hervorholen, weil man dabei
        // die Buchhaltung von LuaCall durcheinanderbringt.

    // Überschreibe die Ausgabefunktionen mit Varianten, die LuaCall statt LuaStack zurückgeben.
    LuaCall&operator<<(const LuaNil&X){ LuaStack::operator<<(X); return*this; }
    LuaCall&operator<<(const LuaValue&X){ LuaStack::operator<<(X); return*this; }
    LuaCall&operator<<(const char s[]){ LuaStack::operator<<(s); return*this; }
    LuaCall&operator<<(const LuaAbsIndex&X){ LuaStack::operator<<(X); return*this; }
    LuaCall&operator<<(lua_CFunction X){ LuaStack::operator<<(X); return*this; }
    LuaCall&operator<<(const LuaTable&X){ LuaStack::operator<<(X); return*this; }
    LuaCall&operator<<(const std::vector<std::string>&X){ LuaStack::operator<<(X); return*this; }
    LuaCall&operator<<(std::function<void(LuaStack&)>ArgumentProvider){ ArgumentProvider(*this); return*this; }
//  LuaCall&operator<<(const LuaClosure&X){ LuaStack::operator<<(X); return*this; }
    LuaCall&operator<<(int n){ LuaStack::operator<<(n); return*this; }
    LuaCall&operator<<(unsigned n){ LuaStack::operator<<(n); return*this; }
    LuaCall&operator<<(bool f){ LuaStack::operator<<(f); return*this; }
    LuaCall&operator<<(float x){ LuaStack::operator<<(x); return*this; }
    LuaCall&operator<<(double x){ LuaStack::operator<<(x); return*this; }
    LuaCall&operator<<(const LuaUpValue&X){ LuaStack::operator<<(X); return*this; }
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
