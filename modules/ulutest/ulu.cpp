
#include <string>
#include <vector>
#include <lua.hpp>

using namespace std;

bool check_tty(int fd);

const pair<string,string>
    red=make_pair("\x1b[1;31m", "\x1b[0m"),
    blue=make_pair("\x1b[1;34m", "\x1b[0m"),
    green=make_pair("\x1b[1;32m", "\x1b[0m"),
    yellow=make_pair("\x1b[1;33m", "\x1b[0m");

const auto
    RUNTEST=            "[ RUN      ]",
    FAILEDTEST=         "[  FAILED  ]",
    PASSEDTEST=         "[  PASSED  ]",
    SUCCESSFULTEST=     "[       OK ]",
    FAILEDCRITERION=    "[     FAIL ]",
    SUCCESSFULCRITERION="[       OK ]",
    FRAME=              "[==========]",
    SEP=                "[----------]",
    INFO=               "[     INFO ]",
    DISABLED=           "[ DISABLED ]",
    SKIPPING=           "[ skipping ]";

// local function red(str)    return grey and str or "\27[1;31m"..str.."\27[0m" end
// local function blue(str)   return grey and str or "\27[1;34m"..str.."\27[0m" end
// local function green(str)  return grey and str or "\27[1;32m"..str.."\27[0m" end
// local function yellow(str) return grey and str or "\27[1;33m"..str.."\27[0m" end
// RUNTEST=            blue   "[ RUN      ]"
// FAILEDTEST=         red    "[  FAILED  ]"
// PASSEDTEST=         green  "[  PASSED  ]"
// SUCCESSFULTEST=     green  "[       OK ]"
// FAILEDCRITERION=    red    "[     FAIL ]"
// SUCCESSFULCRITERION=green  "[       OK ]"
// FRAME=              blue   "[==========]"
// SEP=                blue   "[----------]"
// INFO=               yellow "[     INFO ]"
// DISABLED=           yellow "[ DISABLED ]"
// SKIPPING=           yellow "[ skipping ]"

const string bunt(const string&str, const pair<string,string>&color){ return color.first+str+color.second; }

const vector<pair<string,string>> Colortags={
    make_pair("RUNTEST", bunt(RUNTEST, blue)),
    make_pair("FAILEDTEST", bunt(FAILEDTEST, red)),
    make_pair("PASSEDTEST", bunt(PASSEDTEST, green)),
    make_pair("SUCCESSFULTEST", bunt(SUCCESSFULTEST, green)),
    make_pair("FAILEDCRITERION", bunt(FAILEDCRITERION, red)),
    make_pair("SUCCESSFULCRITERION", bunt(SUCCESSFULCRITERION, green)),
    make_pair("FRAME", bunt(FRAME, blue)),
    make_pair("SEP", bunt(SEP, blue)),
    make_pair("INFO", bunt(INFO, yellow)),
    make_pair("DISABLED", bunt(DISABLED, yellow)),
    make_pair("SKIPPING", bunt(SKIPPING, yellow))
};

const vector<pair<string,string>> Blacktags={
    make_pair("RUNTEST", RUNTEST),
    make_pair("FAILEDTEST", FAILEDTEST),
    make_pair("PASSEDTEST", PASSEDTEST),
    make_pair("SUCCESSFULTEST", SUCCESSFULTEST),
    make_pair("FAILEDCRITERION", FAILEDCRITERION),
    make_pair("SUCCESSFULCRITERION", SUCCESSFULCRITERION),
    make_pair("FRAME", FRAME),
    make_pair("SEP", SEP),
    make_pair("INFO", INFO),
    make_pair("DISABLED", DISABLED),
    make_pair("SKIPPING", SKIPPING)
};

static void mktagtable(lua_State*L, const vector<pair<string,string>>&Tags)
{
    lua_createtable(L, Tags.size(), 0);
    for (const auto&k: Tags)
    {
        lua_pushlstring(L, k.second.c_str(), k.second.size());
        lua_setfield(L, -2, k.first.c_str());
    }
}

extern "C" int gtest_tags(lua_State*L)
{
    const bool tty=check_tty(1);
    mktagtable(L, tty?Colortags:Blacktags);
    return 1;
}

// =====================================================================

#include <chrono>

using namespace std::chrono_literals;
using clk=chrono::high_resolution_clock;
using tp=clk::time_point;

namespace {

const auto mtname="timestamp_highres";
const void*mtpointer=nullptr; // identify metatable via lua_topointer()

static bool istimestamp(lua_State*L, int index)
{
    if (mtpointer==nullptr) return false;
    if (lua_type(L, index)!=LUA_TUSERDATA) return false;
    if (!lua_getmetatable(L, index)) return false;
    const void*p=lua_topointer(L, -1);
    lua_pop(L,1);
    return p==mtpointer;
}

extern "C" int tsdiff(lua_State*L)
{
    if (!istimestamp(L, 1)) return luaL_typeerror(L, 1, "timestamp (from tshighres)");
    if (!istimestamp(L, 2)) return luaL_typeerror(L, 2, "timestamp (from tshighres)");
    const tp T1=*reinterpret_cast<tp*>(lua_touserdata(L, 1)),
             T2=*reinterpret_cast<tp*>(lua_touserdata(L, 2));
#if 0
    auto d=(T1-T2)/1us;
    lua_pushnumber(L, 0.001*d);
#else
    lua_pushinteger(L, (T1-T2)/1ms);
#endif
    return 1;
}

static void gettsmeta(lua_State*L)
{
    lua_pushvalue(L, LUA_REGISTRYINDEX);            // Registry
    const auto t1=lua_getfield(L, -1, mtname);      // Registry mt|nil
    if (t1==LUA_TNIL)
    {
        lua_pop(L, 1);                              // Registry
        lua_createtable(L, 0, 2);                   // Reg {}
        mtpointer=lua_topointer(L, -1);
            lua_pushliteral(L, "timestamp");        // Reg {} ".."
            lua_setfield(L, -2, "__name");          // Reg {__name=".."}
            lua_pushcfunction(L, tsdiff);           // Reg {...} tsdiff
            lua_setfield(L, -2, "__sub");           // Reg {...m __sub=tsdiff}
        lua_setfield(L, -2, mtname);                // Reg
        const auto t2=lua_getfield(L, -1, mtname);  // Reg mt|nil
        if (t2!=LUA_TTABLE)
        {
            lua_pushstring(L, "Kann keine Tabelle erzeugen.");
            lua_error(L);
        }
        lua_remove(L, -2);  // mt
    }
    else if (t1!=LUA_TTABLE)
    {
        lua_pushfstring(L, "Cannot register timestamp, Registry.%s ist not a table but %s.", mtname, lua_typename(L, -1));
        lua_error(L);
    }
    else lua_remove(L, -2); // mt
}

} // anon

extern "C" int timestamp(lua_State*L)
{
    auto*jetzt=reinterpret_cast<tp*>(lua_newuserdatauv(L, sizeof(tp), 0));
    gettsmeta(L);
    lua_setmetatable(L, -2);
    *jetzt=clk::now();
    return 1;
}
