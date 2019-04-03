/**
 *  @file dynlib.c
 *
 *  @brief The common object file to process dynamic library
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

#if defined(LINUX)
    #include <dlfcn.h>
#elif defined(WINDOWS)

#endif

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "dynlib.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */
typedef struct
{
#if defined(LINUX)
    void * idl_pDynLib;
#elif defined(WINDOWS)
    HMODULE idl_hDynLib;
#endif
} internal_dyn_lib_t;

/* --- private routine section---------------------------------------------- */


/* --- public routine section ---------------------------------------------- */
u32 loadDynLib(const olchar_t * pstrLibFile, dyn_lib_t ** ppLib)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dyn_lib_t * pidl = NULL;

#if defined(LINUX)
    u32Ret = xmalloc((void **)&pidl, sizeof(internal_dyn_lib_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(pidl, 0, sizeof(internal_dyn_lib_t));
        
        pidl->idl_pDynLib = dlopen(pstrLibFile, RTLD_LAZY);
        if (pidl->idl_pDynLib == NULL)
            u32Ret = JF_ERR_FAIL_LOAD_DYNLIB;
    }

#elif defined(WINDOWS)
    u32Ret = xmalloc((void **)&pidl, sizeof(internal_dyn_lib_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(pidl, 0, sizeof(internal_dyn_lib_t));

        pidl->idl_hDynLib = LoadLibrary(pstrLibFile);
        if (pidl->idl_hDynLib == NULL)
            u32Ret = JF_ERR_FAIL_LOAD_DYNLIB;
    }

#endif

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppLib = pidl;
    else if (pidl != NULL)
        freeDynLib((dyn_lib_t **)&pidl);

    return u32Ret;
}

u32 freeDynLib(dyn_lib_t ** ppLib)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dyn_lib_t * pidl = NULL;

#if defined(LINUX)
    olint_t nRet;

    assert((ppLib != NULL) && (*ppLib != NULL));

    pidl = (internal_dyn_lib_t *) *ppLib;

    if (pidl->idl_pDynLib != NULL)
    {
        nRet = dlclose(pidl->idl_pDynLib);
        if (nRet != 0)
            u32Ret = JF_ERR_FAIL_FREE_DYNLIB;
    }

    xfree(ppLib);

#elif defined(WINDOWS)
    boolean_t bRet;

    assert((ppLib != NULL) && (*ppLib != NULL));

    pidl = (internal_dyn_lib_t *) *ppLib;

    if (pidl->idl_hDynLib != NULL)
    {
        bRet = FreeLibrary(pidl->idl_hDynLib);
        if (! bRet)
            u32Ret = JF_ERR_FAIL_FREE_DYNLIB;
    }

    xfree(ppLib);

#endif

    return u32Ret;
}

u32 getSymbolAddress(dyn_lib_t * pLib, const olchar_t * pstrSymbol,
    void ** ppAddress)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dyn_lib_t * pidl = (internal_dyn_lib_t *)pLib;

#if defined(LINUX)
    assert(pLib != NULL);

    *ppAddress = dlsym(pidl->idl_pDynLib, pstrSymbol);
    if (*ppAddress == NULL)
        u32Ret = JF_ERR_FAIL_GET_SYMBOL_ADDR;

#elif defined(WINDOWS)
    assert(pLib != NULL);

    *ppAddress = GetProcAddress(pidl->idl_hDynLib, pstrSymbol);
    if (*ppAddress == NULL)
        u32Ret = JF_ERR_FAIL_GET_SYMBOL_ADDR;

#endif

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


