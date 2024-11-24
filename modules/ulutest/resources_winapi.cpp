
#ifdef _MSC_BUILD

#include <Windows.h>
#include <libloaderapi.h>
#include <string_view>
#include <vector>

extern HMODULE ulutest_module;

using std::string_view;
using std::vector;

static string_view loadresource_msvc(unsigned resid, unsigned restype)
{
    HRSRC resinfo=FindResource(ulutest_module, MAKEINTRESOURCE(resid), MAKEINTRESOURCE(restype));
    // printf("FindResource(%p, %u, %u)=>%p\n", ulutest_module, resid, restype, resinfo);
    if (resinfo!=0)
    {
        HGLOBAL reshandle=LoadResource(ulutest_module, resinfo);
        // printf("LoadResource(%p, %u, %u)=>%p\n", ulutest_module, resid, restype, reshandle);
        if (reshandle!=nullptr) return {static_cast<char*>(LockResource(reshandle)),
            static_cast<size_t>(SizeofResource(ulutest_module, resinfo))};
        else return {};
    }
    else return {};
}

string_view chunk_ulutest() { return loadresource_msvc(101, 1024); }

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
    printf("Resource Types (%p):\n", ulutest_module);
    BOOL f=EnumResourceTypesEx(ulutest_module, enumerate_restype, 0l, allres, 0);
    const vector<LPCSTR> Types {MAKEINTRESOURCE(0x18)};
    printf("Resources:\n");
    for (auto k: Types)
    {
        BOOL g=EnumResourceNamesEx(ulutest_module, k, collect_resname, 0l, allres, 0);
    }
}

#endif
