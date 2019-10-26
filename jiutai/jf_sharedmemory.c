/**
 *  @file jf_sharedmemory.c
 *
 *  @brief The implementation of share memory common object
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#if defined(LINUX)
    #include <sys/ipc.h>
    #include <sys/shm.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_jiukun.h"
#include "jf_sharedmemory.h"
#if defined(WINDOWS)
    #include "jf_uuid.h"
#endif

/* --- private data/data structure section ------------------------------------------------------ */

#if defined(LINUX)
    #define SV_W    0400
    #define SV_R    0200

    #define SVMSG_MODE  (SV_W | SV_R | (SV_W >> 3) | (SV_R >> 3) | (SV_W >> 6) | (SV_R >> 6))
    #define SVSHM_MODE  SVMSG_MODE
#elif defined(WINDOWS)
    #define DEFAULT_SHARED_MEMORY_SIZE 4000
#endif

/* --- private routine section ------------------------------------------------------------------ */
#if defined(LINUX)
static u32 _getShmId(jf_sharedmemory_id_t * pjsi, olint_t * pnShmId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (sscanf(pjsi, "%d", pnShmId) != 1)
        u32Ret = JF_ERR_INVALID_SHAREDMEMORY_ID;

    return u32Ret;
}
#elif defined(WINDOWS)
static u32 _getShmId(jf_sharedmemory_id_t * pjsi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    uuid_param_t up;

	ol_memset(&up, 0, sizeof(uuid_param_t));

	up.up_ufFmt = UUID_FMT_HEX;

	u32Ret = getUuid(pjsi, JF_SHAREDMEMORY_ID_LEN, UUID_VER_1, &up);
    if (u32Ret != JF_ERR_NO_ERROR)
	{
		u32Ret = JF_ERR_FAIL_CREATE_SHAREDMEMORY;
	}

	return u32Ret;
}
#endif

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_sharedmemory_create(jf_sharedmemory_id_t ** ppShmId, u32 u32MemorySize)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_sharedmemory_id_t * pjsi = NULL;
#if defined(LINUX)
    olint_t nShmId;

    assert(ppShmId != NULL);

    u32Ret = jf_jiukun_allocMemory((void **)&pjsi, JF_SHAREDMEMORY_ID_LEN);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(pjsi, 0, JF_SHAREDMEMORY_ID_LEN);

        nShmId = shmget(IPC_PRIVATE, u32MemorySize, SVSHM_MODE | IPC_CREAT);
        if (nShmId == -1)
            u32Ret = JF_ERR_FAIL_CREATE_SHAREDMEMORY;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /* remove the shared memory id to avoid memory leak due to
         * ungraceful client exit or crash
         * if it is attached, it will remain there until the last detachment
         */
//        nRet = shmctl(nShmId, IPC_RMID, NULL);

        ol_snprintf(pjsi, JF_SHAREDMEMORY_ID_LEN - 1, "%d", nShmId);
    }

#elif defined(WINDOWS)
	HANDLE hMap;

    assert(ppShmId != NULL);

    u32Ret = jf_jiukun_allocMemory((void **)&pjsi, JF_SHAREDMEMORY_ID_LEN);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(pjsi, 0, JF_SHAREDMEMORY_ID_LEN);

        u32Ret = _getShmId(pjsi);
    }

	if (u32Ret == JF_ERR_NO_ERROR)
	{
        hMap = CreateFileMapping(
            INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, u32MemorySize, pjsi);
	    if (hMap == NULL)
		    u32Ret = JF_ERR_FAIL_CREATE_SHAREDMEMORY;
	}

#endif
    if (u32Ret == JF_ERR_NO_ERROR)
        *ppShmId = pjsi;
    else if (pjsi != NULL)
        jf_sharedmemory_destroy((jf_sharedmemory_id_t **)&pjsi);

    return u32Ret;
}

u32 jf_sharedmemory_destroy(jf_sharedmemory_id_t ** ppShmId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_sharedmemory_id_t * pjsi = NULL;
#if defined(LINUX)
    olint_t nRet, nShmId;

    assert((ppShmId != NULL) && (*ppShmId != NULL));

    pjsi = *ppShmId;

    u32Ret = _getShmId(pjsi, &nShmId);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        nRet = shmctl(nShmId, IPC_RMID, NULL);
        if (nRet == -1)
            u32Ret = JF_ERR_FAIL_DESTROY_SHAREDMEMORY;
    }

    jf_jiukun_freeMemory((void **)ppShmId);

#elif defined(WINDOWS)
	HANDLE hMap;

    assert((ppShmId != NULL) && (*ppShmId != NULL));

    pjsi = *ppShmId;

    hMap = CreateFileMapping(
        INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
        DEFAULT_SHARED_MEMORY_SIZE, pjsi);
	if (hMap == NULL)
		u32Ret = JF_ERR_FAIL_DESTROY_SHAREDMEMORY;
	else
	{
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			/* The shared memory must exist before, otherwise we were creating 
			   a new shared memory, close it and return an error */
			CloseHandle(hMap);
			u32Ret = JF_ERR_INVALID_SHAREDMEMORY_ID;
		}
	}

	if (u32Ret == JF_ERR_NO_ERROR)
		CloseHandle(hMap);

    jf_jiukun_freeMemory((void **)ppShmId);

#endif

    return u32Ret;
}

u32 jf_sharedmemory_attach(jf_sharedmemory_id_t * pShmId, void ** ppMapAddress)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    void * pMap;
    olint_t nShmId;

    assert(pShmId != NULL);

    u32Ret = _getShmId(pShmId, &nShmId);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pMap = shmat(nShmId, NULL, 0);
#if defined(JIUFENG_64BIT)
        if ((s64)pMap == -1)
#else
        if ((s32)pMap == -1)
#endif
        {
            u32Ret = JF_ERR_FAIL_ATTACH_SHAREDMEMORY;
        }
        else
        {
            *ppMapAddress = pMap;
        }
    }

#elif defined(WINDOWS)
	HANDLE hMap;
    void * pMap;

    assert(pShmId != NULL);

    hMap = CreateFileMapping(
        INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
        DEFAULT_SHARED_MEMORY_SIZE, pShmId);
	if (hMap == NULL)
		u32Ret = JF_ERR_INVALID_SHAREDMEMORY_ID;
	else
	{
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			/* The shared memory must exist before, otherwise we were creating 
			   a new shared memory, close it and return an error */
			CloseHandle(hMap);
			u32Ret = JF_ERR_INVALID_SHAREDMEMORY_ID;
		}
	}

	if (u32Ret == JF_ERR_NO_ERROR)
	{
		pMap = MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, 0);
		if (pMap == NULL)
			u32Ret = JF_ERR_FAIL_ATTACH_SHAREDMEMORY;
	}

	if (u32Ret == JF_ERR_NO_ERROR)
        *ppMapAddress = pMap;
#endif

    return u32Ret;
}

u32 jf_sharedmemory_detach(void ** ppMapAddress)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;

    nRet = shmdt(*ppMapAddress);
    if (nRet == -1)
        u32Ret = JF_ERR_FAIL_DETACH_SHAREDMEMORY;

#elif defined(WINDOWS)
    boolean_t bRet;

	bRet = UnmapViewOfFile(*ppMapAddress);
	if (! bRet)
		u32Ret = JF_ERR_FAIL_DETACH_SHAREDMEMORY;

#endif

    *ppMapAddress = NULL;

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


