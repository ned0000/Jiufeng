/**
 *  @file sharedmemory.c
 *
 *  @brief The implementation of share memory common object
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
    #include <sys/ipc.h>
    #include <sys/shm.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "xmalloc.h"
#include "sharedmemory.h"
#if defined(WINDOWS)
    #include "uuid.h"
#endif

/* --- private data/data structure section --------------------------------- */

#if defined(LINUX)
    #define SV_W    0400
    #define SV_R    0200

    #define SVMSG_MODE  (SV_W | SV_R | (SV_W >> 3) | (SV_R >> 3) | (SV_W >> 6) | (SV_R >> 6))
    #define SVSHM_MODE  SVMSG_MODE
#elif defined(WINDOWS)
    #define DEFAULT_SHARED_MEMORY_SIZE 4000
#endif

#define SHM_ID_LEN   40

/* --- private routine section---------------------------------------------- */
#if defined(LINUX)
static u32 _getShmId(shm_id_t * psi, olint_t * pnShmId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (sscanf(psi, "%d", pnShmId) != 1)
        u32Ret = JF_ERR_INVALID_SHAREDMEMORY_ID;

    return u32Ret;
}
#elif defined(WINDOWS)
static u32 _getShmId(shm_id_t * psi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    uuid_param_t up;

	memset(&up, 0, sizeof(uuid_param_t));

	up.up_ufFmt = UUID_FMT_HEX;

	u32Ret = getUuid(psi, SHM_ID_LEN, UUID_VER_1, &up);
    if (u32Ret != JF_ERR_NO_ERROR)
	{
		u32Ret = JF_ERR_FAIL_CREATE_SHAREDMEMORY;
	}

	return u32Ret;
}
#endif

/* --- public routine section ---------------------------------------------- */
u32 createSharedMemory(shm_id_t ** ppShmId, u32 u32MemorySize)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    shm_id_t * psi = NULL;
#if defined(LINUX)
    olint_t nShmId;

    assert(ppShmId != NULL);

    u32Ret = jf_mem_alloc((void **)&psi, SHM_ID_LEN);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(psi, 0, SHM_ID_LEN);

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

        ol_snprintf(psi, SHM_ID_LEN - 1, "%d", nShmId);
    }

#elif defined(WINDOWS)
	HANDLE hMap;

    assert(ppShmId != NULL);

    u32Ret = jf_mem_alloc((void **)&psi, SHM_ID_LEN);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(psi, 0, SHM_ID_LEN);

        u32Ret = _getShmId(psi);
    }

	if (u32Ret == JF_ERR_NO_ERROR)
	{
        hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
            0, u32MemorySize, psi);
	    if (hMap == NULL)
		    u32Ret = JF_ERR_FAIL_CREATE_SHAREDMEMORY;
	}

#endif
    if (u32Ret == JF_ERR_NO_ERROR)
        *ppShmId = psi;
    else if (psi != NULL)
        destroySharedMemory((shm_id_t **)&psi);

    return u32Ret;
}

u32 destroySharedMemory(shm_id_t ** ppShmId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    shm_id_t * psi = NULL;
#if defined(LINUX)
    olint_t nRet, nShmId;

    assert((ppShmId != NULL) && (*ppShmId != NULL));

    psi = *ppShmId;

    u32Ret = _getShmId(psi, &nShmId);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        nRet = shmctl(nShmId, IPC_RMID, NULL);
        if (nRet == -1)
            u32Ret = JF_ERR_FAIL_DESTROY_SHAREDMEMORY;
    }
#elif defined(WINDOWS)
	HANDLE hMap;

    assert((ppShmId != NULL) && (*ppShmId != NULL));

    psi = *ppShmId;

    hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
        0, DEFAULT_SHARED_MEMORY_SIZE, psi);
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
#endif

    return u32Ret;
}

u32 attachSharedMemory(shm_id_t * pShmId, void ** ppMapAddress)
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

    hMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
        0, DEFAULT_SHARED_MEMORY_SIZE, pShmId);
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

u32 detachSharedMemory(void ** ppMapAddress)
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

/*---------------------------------------------------------------------------*/


