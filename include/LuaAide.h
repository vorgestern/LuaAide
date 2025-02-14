
#ifndef LUAAIDE_H
#define LUAAIDE_H

#include <functional>
#include <string_view>
#include <string>
#include <unordered_map>
#include <lua.hpp>

int formatany(lua_State*);
int keyescape(lua_State*);

class LuaStack;
class LuaCall;
class LuaList;

const enum class LuaNil {a} luanil=LuaNil::a;
const enum class LuaError {a} luaerror=LuaError::a;
const enum class LuaSwap {a} luaswap=LuaSwap::a;
const enum class LuaRotate:int {up5=-5, up4=-4, up3=-3, down3=3, down4=4, down5=5}
    luarot_5=LuaRotate::up5,
    luarot_4=LuaRotate::up4,
    luarot_3=LuaRotate::up3,
    luarot3=LuaRotate::down3,
    luarot4=LuaRotate::down4,
    luarot5=LuaRotate::down5;
const enum class LuaListStart {a} lualist=LuaListStart::a;
const enum class LuaListEnd {a} lualistend=LuaListEnd::a;
const enum class LuaTableTag {a} newtable=LuaTableTag::a;

enum class LuaType:int {
    TNONE=-1,
    TNIL=0,
    TBOOLEAN,
    TLIGHTUSERDATA,
    TNUMBER,
    TSTRING,
    TTABLE,
    TFUNCTION,
    TUSERDATA,
    TTHREAD
};
std::string_view tostring(LuaType);

enum class LuaMetaMethod:unsigned {
    tostring,
    add, sub, mul, div, mod, pow,
    unm,
    band, bor, bxor, bnot, shl, shr,
    concat,
    len,
    eq, lt, le,
    index, newindex,
    call,
    gc,
    close,
    mode,
    name                   
};
std::string_view tostring(LuaMetaMethod);

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

enum class distinct_pushable {a,s,as,te,u,v,r,lud,g,f,dc,gc,cl,co}; // array,struct,table,tableelement,upvalue,value,regvalue,lightuserdata,global,field,dotcall,globalcall,closure,code
template<typename I, distinct_pushable d>struct Distinct { I value; };
typedef Distinct<size_t, distinct_pushable::a> LuaArray;
typedef Distinct<size_t, distinct_pushable::s> LuaStruct;
typedef Distinct<std::pair<size_t,size_t>, distinct_pushable::as> LuaTable;
typedef Distinct<std::pair<int,lua_Integer>, distinct_pushable::te> LuaElement; // tablepos, elementindex
typedef Distinct<unsigned, distinct_pushable::u> LuaUpValue;
typedef Distinct<int, distinct_pushable::v> LuaValue;
typedef Distinct<const void*, distinct_pushable::r> LuaRegValue;
typedef Distinct<const void*, distinct_pushable::lud> LuaLightUserData;
typedef Distinct<std::string_view, distinct_pushable::g> LuaGlobal;
typedef Distinct<std::string_view, distinct_pushable::f> LuaField;
typedef Distinct<std::string_view, distinct_pushable::dc> LuaDotCall;
typedef Distinct<std::string_view, distinct_pushable::gc> LuaGlobalCall;
typedef Distinct<std::pair<lua_CFunction,unsigned>, distinct_pushable::cl> LuaClosure;
typedef Distinct<std::string_view, distinct_pushable::co> LuaCode;

// So erzeugt man eine Closure:
// Stack<<upvalue1<<upvalue2<<LuaClosure(function, 2)>>LuaGlobal("closurename");

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
    friend unsigned height(const LuaStack&S){ return lua_gettop(S.L); }
    friend unsigned version(const LuaStack&); // Lua 5.4.6 gibt 504 zurück.
    friend std::ostream&operator<<(std::ostream&, const LuaStack&);

protected:
    lua_State*L{nullptr};

    class absindex
    {
        friend class LuaStack;
        int posindex{0};
        friend int stackindex(const absindex&X){ return X.posindex; }
        absindex(int j): posindex(j){}
    };

public:
    LuaStack(){}
    LuaStack(lua_State*L1): L(L1){}
    operator lua_State*()const{ return L; }
    LuaStack&clear();
    LuaStack&swap(); //!< Tausche die beiden obersten Werte auf dem Stack.
    LuaStack&drop(unsigned num); //!< Wenn num>height ==> Leere den Stack.
    LuaStack&rotate(int wo, int num);
    LuaStack&dup(int was=-1);
    LuaStack&remove(int was);

    absindex index(int wo);

    LuaStack&operator<<(LuaSwap);
    LuaStack&operator<<(LuaRotate);

    LuaStack&operator<<(bool b){ lua_pushboolean(L, b?1:0); return*this; }
    LuaStack&operator<<(int n){ lua_pushinteger(L, n); return*this; }
    LuaStack&operator<<(unsigned n){ lua_pushinteger(L, static_cast<int>(n)); return*this; }
    LuaStack&operator<<(const char s[]){ lua_pushstring(L, s); return*this; }
    LuaStack&operator<<(std::string_view s){ lua_pushlstring(L, s.data(), s.size()); return*this; }
    LuaStack&operator<<(float x){ lua_pushnumber(L, x); return*this; }
    LuaStack&operator<<(double x){ lua_pushnumber(L, x); return*this; }
    LuaStack&operator<<(const LuaValue&X){ lua_pushvalue(L, X.value); return*this; }
    LuaStack&operator<<(const LuaUpValue&V){ lua_pushvalue(L, lua_upvalueindex(V.value)); return*this; }
    LuaStack&operator<<(const LuaGlobal&X){ lua_getglobal(L, X.value.data()); return*this; }
    LuaStack&operator<<(const absindex&X){ lua_pushvalue(L, stackindex(X)); return*this; }
    LuaStack&operator<<(const LuaField&X){ lua_getfield(L, -1, X.value.data()); return*this; }
    LuaStack&operator<<(const LuaElement&X){ lua_geti(L, X.value.first, X.value.second); return*this; }
    LuaStack&operator<<(const LuaNil&X){ lua_pushnil(L); return*this; }
    LuaStack&operator<<(const LuaTable&X){ lua_createtable(L, X.value.first, X.value.second); return*this; }
    LuaStack&operator<<(const LuaArray&X){ lua_createtable(L, X.value, 0); return*this; }
    LuaStack&operator<<(LuaTableTag){ lua_createtable(L, 0, 0); return*this; }
    LuaStack&operator<<(const LuaLightUserData&);
    LuaStack&operator<<(const std::vector<std::string>&);
    LuaStack&operator<<(const std::unordered_map<std::string, std::string>&);
    LuaStack&operator<<(const LuaRegValue&);

    LuaCall  operator<<(const LuaCode&);
    LuaCall  operator<<(const std::pair<std::string_view, const LuaCode&>&); // chunkname first, chunk second
    LuaCall  operator<<(lua_CFunction);
    LuaCall  operator<<(const LuaColonCall&);
    LuaCall  operator<<(const LuaDotCall&);
    LuaCall  operator<<(const LuaGlobalCall&);
    LuaCall  operator<<(const LuaClosure&);

    LuaList operator<<(LuaListStart);

    // Konzept: Creating arbitrary tables with <<value>=key;   (downside: not chainable due to preference descent)
    // Q<<newtable<<101>="key1";                             {key1=101, version="0.1", [..]=myprint}
    // Q<<"0.1">="version";
    // Q<<myprint>=LuaLightUserData(0x12345678);
    LuaStack&operator>=(int n){ operator<<(n); lua_settable(L, -3); return*this; }

    // Konzept: Creating arbitrary tables with .F(key, value)  (chainable; downside: restart, parentheses)
    // Q<<newtable;
    // Q.F("key1", 101).F("version", "0.1").F(?, myprint);
    template<typename K, typename V>LuaStack&F(K k, V v) {  operator<<(k)<<v; lua_settable(L, -3); return*this; }

    int operator>>(const LuaError&){ lua_error(L); return 0; }
    LuaStack&operator>>(const LuaGlobal&X){ lua_setglobal(L, X.value.data()); return*this; } //!< Zuweisung an globale Variable
    LuaStack&operator>>(const LuaField&F){ lua_setfield(L, -2, F.value.data()); return*this; }
    LuaStack&operator>>(const LuaElement&E){ lua_seti(L, E.value.first, E.value.second); return*this; }
    LuaStack&operator>>(const LuaRegValue&); // [value] ==> []

    LuaType operator()(const LuaElement&X){ return static_cast<LuaType>(lua_geti(L, X.value.first, X.value.second)); }
//  LuaType operator()(const LuaField&X){ return static_cast<LuaType>(lua_getfield(L, -1, X.name)); }

    bool posvalid(int pos){ return (pos>0)?(pos<=lua_gettop(L)):(pos<0)?(-pos<=lua_gettop(L)):false; }
    LuaType typeat(int pos){ return static_cast<LuaType>(lua_type(L, pos)); }
    bool hasat(LuaType t, int pos){ return posvalid(pos) && static_cast<LuaType>(lua_type(L, pos))==t; }

#if 1
    bool hasnilat(int pos){ return posvalid(pos) && lua_isnil(L, pos)!=0; }
    bool hasstringat(int pos){ return posvalid(pos) && lua_isstring(L, pos)!=0; }
    bool hasboolat(int pos){ return posvalid(pos) && lua_isboolean(L, pos)!=0; }
    bool hasintat(int pos){ return lua_isinteger(L, pos)!=0; }
    bool hasnumberat(int pos){ return lua_isnumber(L, pos)!=0; }
    bool hastableat(int pos){ return lua_istable(L, pos)!=0; }
    bool hasfunctionat(int pos){ return posvalid(pos) && lua_isfunction(L, pos)!=0; }
    bool hasthreadat(int pos){ return lua_isthread(L, pos)!=0; }
    bool hasuserdataat(int pos){ return lua_isuserdata(L, pos)!=0; }
    bool haslightuserdataat(int pos){ return lua_islightuserdata(L, pos)!=0; }
#else
    // Use hasat(Type, pos) instead.
#endif
    bool hasheavyuserdataat(int pos){ return lua_isuserdata(L, pos) && !lua_islightuserdata(L, pos); }

    std::string tostring(int pos);
    bool tobool(int pos){ return lua_toboolean(L, pos)!=0; }
    long long toint(int pos){ return lua_tointeger(L, pos); }
    double todouble(int pos){ return lua_tonumber(L, pos); }
    template<typename X>X touserdata(int pos){ return reinterpret_cast<X>(lua_touserdata(L, pos)); }
    template<typename X>X*touserpointer(int pos){ auto P=reinterpret_cast<X**>(lua_touserdata(L, pos)); return P!=nullptr?*P:nullptr; }

    std::string asstring(int pos); // Stack element not converted!

    //! dofile() und dostring() erlauben jeweils die Übergabe von Argumenten im Stil (argc, argv).
    //! Sie werden auf den Stack gelegt und sind dann mit args={...} abrufbar!
    bool dofile(const char filename[], int argc, char*argv[]);
    bool dostring(const char text[], int argc, char*argv[], const char tag[]);

    static lua_State*New(bool defaultlibs, lua_CFunction errorhandler);
    void Close();
    bool check(int numpos);

    std::string errormessage();

    // luaerror if condition not met:
    void argcheck(int index, LuaType, std::string_view hint); // Expect argument type
    void argcheck(int index, const std::function<bool(LuaStack&, int index)>&cond, std::string_view hint); // Expect argument to meet cond

    LuaCall operator[](LuaMetaMethod);
    LuaStack&operator>>(LuaMetaMethod m){ lua_setfield(L, -2, ::tostring(m).data()); return*this; } // [table, value] ==> [table={..., method=value}]
};

class LuaCall: public LuaStack
{
    friend class LuaStack;

    //! Dieser Konstruktor ist nur für Freunde zugänglich.
    //! Diese haben u.U. bereits Informationen auf den Stack gelegt, die am Ende entfernt werden sollen.
    LuaCall(lua_State*, absindex);

protected:
    absindex funcindex; // Stackindex der Funktion auf dem Stack. Wird benutzt, um beim Aufruf der Funktion die Anzahl der Argumente zu ermitteln.

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
    LuaCall&operator<<(std::function<void(LuaStack&)>ArgumentProvider){ ArgumentProvider(*this); return*this; }
    template<typename Arg>LuaCall&operator<<(const Arg&X){ LuaStack::operator<<(X); return*this; }
};

class LuaList: LuaStack
{
    friend class LuaStack;
    unsigned index {0};
    LuaList&append(){ lua_seti(L, -2, ++index); return*this; }
    LuaList(lua_State*L): LuaStack(L){}
public:
    LuaList&operator<<(const LuaNil&X){ LuaStack::operator<<(X); return append(); }
    LuaList&operator<<(const LuaValue&X){ LuaStack::operator<<(X); return append(); }
    LuaList&operator<<(const char s[]){ LuaStack::operator<<(s); return append(); }
    LuaList&operator<<(const absindex&X){ LuaStack::operator<<(X); return append(); }
    LuaList&operator<<(lua_CFunction X){ LuaStack::operator<<(X); return append(); }
    LuaList&operator<<(const LuaTable&X){ LuaStack::operator<<(X); return append(); }
    LuaList&operator<<(const std::vector<std::string>&X){ LuaStack::operator<<(X); return append(); }
    LuaList&operator<<(int n){ LuaStack::operator<<(n); return append(); }
    LuaList&operator<<(unsigned n){ LuaStack::operator<<(n); return append(); }
    LuaList&operator<<(bool f){ LuaStack::operator<<(f); return append(); }
    LuaList&operator<<(float x){ LuaStack::operator<<(x); return append(); }
    LuaList&operator<<(double x){ LuaStack::operator<<(x); return append(); }
    LuaStack operator<<(LuaListEnd){ return*this; }
};

/*
    LuaIterator realisiert eine Schleife über die Elemente des obersten Objekts auf dem Stack.
    Das Äquivalent in Lua ist
        for key,value in pairs(X) do end
    So benutzt man LuaIterator
        LuaStack LS(...);
        for (LuaIterator K(LS); next(K); ++K)
        {
            (unsigned)K ist der einsbasierte Index.
            Bei Stack[-2] liegt key.
            Bei Stack[-1] liegt value.
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
