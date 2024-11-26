
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

constexpr string bunt(const string&str, const pair<string,string>&color){ return color.first+str+color.second; }

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
