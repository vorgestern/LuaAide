
#include <cstring>
#include <string_view>
#include <LuaAide.h>
#include <unistd.h>

using std::string_view;

// Die Adressen dieser Symbole liegen in ltest.o,
// wo sie von objcopy erzeugt wurden (beim Bauen unter Linux).
// Beachte die Anpassung der Namen in syminfo (opjcopy --redefine-syms=modules/ulutest/syminfo)
extern "C" char ltest_start;
extern "C" char ltest_end;

string_view chunk_ulutest()
{
    // const string_view ulutest(&ltest_start, &ltest_end-&ltest_start);
    // return ulutest;
    return {&ltest_start, static_cast<size_t>(&ltest_end-&ltest_start)};
}

bool check_tty(int fd)
{
    return isatty(fd)!=0;
}
