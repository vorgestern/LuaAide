
#ifdef _MSC_BUILD

#include <Windows.h>
#include <libloaderapi.h>
#include <string_view>
#include <vector>

using std::string_view;
using std::vector;

static string_view loadresource_msvc(unsigned resid, unsigned restype)
{
    HRSRC resinfo=FindResource(nullptr, MAKEINTRESOURCE(resid), MAKEINTRESOURCE(restype));
    printf("loadresource(%u, %u)=>%p\n", resid, restype, resinfo);
    if (resinfo!=0)
    {
        HGLOBAL reshandle=LoadResource(nullptr, resinfo);
        if (reshandle!=0) return {static_cast<char*>(LockResource(reshandle)),
            static_cast<size_t>(SizeofResource(nullptr, resinfo))};
        else return {};
    }
    else return {};
}

string_view chunk_ulutest() { return loadresource_msvc(1001, 101); }

// ============================================================================

const unsigned allres=RESOURCE_ENUM_MUI|RESOURCE_ENUM_LN|RESOURCE_ENUM_VALIDATE;

static BOOL __stdcall enumerate_restype(HMODULE hModule, LPSTR typ, LONG_PTR lParam)
{
    printf("\t%p\n", typ);
    return TRUE;
}

static BOOL __stdcall collect_resname(HMODULE, LPCSTR typ, LPSTR nam, LONG_PTR lParam)
{
    printf("\t%p %p\n", typ, nam);
    return TRUE;
}

void enumerate_resources()
{
    printf("Resource Types:\n");
    BOOL f=EnumResourceTypesEx(nullptr, enumerate_restype, 0l, allres, 0);
    const vector<LPCSTR> Types {MAKEINTRESOURCE(0x18)};
    printf("Resources:\n");
    for (auto k: Types)
    {
        BOOL g=EnumResourceNamesEx(nullptr, k, collect_resname, 0l, allres, 0);
    }
}

#endif
