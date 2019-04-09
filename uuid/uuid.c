/**
 *  @file uuid.c
 *
 *  @brief The UUID library to generate UUID(Universally Unique Identifier)
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
    #include <sys/time.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "uuid.h"
#include "prng.h"
#include "ifmgmt.h"
#include "cghash.h"
#include "jf_bitop.h"
#include "jf_time.h"

#include "output.h"
#include "common.h"

/* --- private data/data structure section --------------------------------- */

/** maximum number of 100ns ticks of the actual resolution of system clock
 *  which in our case is 1us (= 1000ns) because we use getTimeOfDay
 */
#define UUIDS_PER_TICK    (10)

/** time offset between UUID and Unix Epoch time according to standards.
 *  UUID UTC base time is October 15, 1582
 *  Unix UTC base time is January  1, 1970
 */
static const u64 ls_u64UuidTimeOffset = 122192928000000000LL;

/** IEEE 802 MAC address encoding/decoding bit fields
 */
/** 0: individual address(unicast), 1: global address(mulcast)
 */
#define IEEE_MAC_MCBIT   JF_BITOP_OCTET(0,0,0,0,0,0,0,1)
/** 0: universally administered address, 1: locally administered address
 */
#define IEEE_MAC_LOBIT   JF_BITOP_OCTET(0,0,0,0,0,0,1,0)

/** uuid generation data structure
 */
typedef struct
{
    /** inlined UUID object */
    uuid_obj_t ug_uoObj;
    /** if it's initialized */
    boolean_t ug_bInitialized;
    u8 ug_u8Reserved[7];

    /** for V1 */
    /** pre-determined MAC address */
    uuid_uint8_t ug_u8Mac[JF_LIMIT_MAC_LEN];
    /** use multi-cast MAC address */
    boolean_t ug_bMulticastMac;
    u8 ug_u8Reserved2;
    /** last retrieved timestamp */
    struct timeval ug_tvLast;
    /** last timestamp sequence counter */
    u32 ug_u32TimeSeq;

    /** for V3 and V5 */
    /** MD5 sub-object */
    jf_cghash_md5_t ug_jcmMd5;
    jf_cghash_sha1_t ug_jcsSha1;
    /** Name, null-terminated string */
    olchar_t * ug_pstrName;
    /** Name space UUID, MUST BE UUID_LEN_BIN length */
    u8 * ug_pu8NameSpace;

} uuid_gen_t;

/* static data structure for generation of UUID
 */
static uuid_gen_t ls_ugUuidGen;

/* --- private routine section---------------------------------------------- */

/* brand UUID with version and variant */
static void _brandUuid(uuid_gen_t * pug, jf_uuid_ver_t version)
{
    /* set version (as given) */
    JF_BITOP_SET(pug->ug_uoObj.uo_u16TimeHiAndVersion, 15, 12, version);

    /* set variant (always DCE 1.1 only) */
    JF_BITOP_SET(pug->ug_uoObj.uo_u8ClockSeqHiAndReserved, 7, 6, 0x2);
}

static u32 _makeUuidV1(uuid_gen_t * pug)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct timeval time_now;
    u64 t;
    uuid_uint16_t clck = 0;

    /* determine current system time and sequence counter */
    for ( ; u32Ret == JF_ERR_NO_ERROR; )
    {
        /* determine current system time */
        u32Ret = jf_time_getTimeOfDay(&time_now);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /* check whether system time changed since last retrieve */
            if (! ((time_now.tv_sec == pug->ug_tvLast.tv_sec) &&
                   (time_now.tv_usec == pug->ug_tvLast.tv_usec)))
            {
                /* reset time sequence counter and continue */
                pug->ug_u32TimeSeq = 0;
                break;
            }

            /* until we are out of UUIDs per tick, increment the time/tick
               sequence counter and continue */
            if (pug->ug_u32TimeSeq < UUIDS_PER_TICK)
            {
                pug->ug_u32TimeSeq++;
                break;
            }
            /* stall the UUID generation until the system clock catches up.
               getTimeOfDay has resolution of 1us. Sleep for 500ns (1/2us) */
            jf_time_nsleep(500);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /* convert from timeval (sec,usec) to u64 (microsecond) format */
        t = (u64)time_now.tv_sec;
        t *= 1000000;
        t += (u64)time_now.tv_usec;
        /* 100-nanosecond */
        t *= 10;

        /* adjust for offset between UUID and Unix Epoch time */
        t += ls_u64UuidTimeOffset;

        /* compensate for low resolution system clock by adding
           the time/tick sequence counter */
        if (pug->ug_u32TimeSeq > 0)
            t += (u64)pug->ug_u32TimeSeq;

        pug->ug_uoObj.uo_u16TimeHiAndVersion = JF_BITOP_GET(t, 59, 48);
        pug->ug_uoObj.uo_u16TimeMid = JF_BITOP_GET(t, 47, 32);
        pug->ug_uoObj.uo_u32TimeLow = JF_BITOP_GET(t, 31, 0);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /* retrieve current clock sequence */
        JF_BITOP_SET(
            clck, 13, 8,
            JF_BITOP_GET((u64)pug->ug_uoObj.uo_u8ClockSeqHiAndReserved, 5, 0));
        JF_BITOP_SET(clck, 7, 0, pug->ug_uoObj.uo_u8ClockSeqLow);

        /* generate new random clock sequence (initially or if the time has
           stepped backwards) or else just increase it */
        if ((clck == 0) ||
            ((time_now.tv_sec < pug->ug_tvLast.tv_sec) ||
             ((time_now.tv_sec == pug->ug_tvLast.tv_sec) &&
              (time_now.tv_usec < pug->ug_tvLast.tv_usec))))
            u32Ret = jf_prng_getData((void *)&clck, sizeof(clck));
        else
            clck++;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        clck %= JF_BITOP_POW2(14);

        /* store back new clock sequence */
        JF_BITOP_SET(
            pug->ug_uoObj.uo_u8ClockSeqHiAndReserved, 5, 0,
            JF_BITOP_GET(clck, 13, 8));
        pug->ug_uoObj.uo_u8ClockSeqLow = JF_BITOP_GET(clck, 7, 0);
    }

    /*  Use a random multi-cast MAC address instead of the real MAC address
     */
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if ((pug->ug_bMulticastMac) ||
            (pug->ug_u8Mac[0] & IEEE_MAC_MCBIT))
        {
            /* generate random IEEE 802 local multicast MAC address */
            u32Ret = jf_prng_getData(
                (void *)&(pug->ug_uoObj.uo_u8Node),
                sizeof(pug->ug_uoObj.uo_u8Node));
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                pug->ug_uoObj.uo_u8Node[0] |= IEEE_MAC_MCBIT;
                pug->ug_uoObj.uo_u8Node[0] |= IEEE_MAC_LOBIT;
            }
        }
        else
        {
            /* use real regular MAC address */
            memcpy(pug->ug_uoObj.uo_u8Node, pug->ug_u8Mac, sizeof(pug->ug_u8Mac));
        }
    }

    /* remember current system time for next iteration */
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pug->ug_tvLast.tv_sec  = time_now.tv_sec;
        pug->ug_tvLast.tv_usec = time_now.tv_usec;

        /* brand with version and variant */
        _brandUuid(pug, JF_UUID_VER_1);
    }

    return u32Ret;
}

static u32 _makeUuidV3(uuid_gen_t * pug)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /* initialize MD5 context */
    jf_cghash_initMd5(&pug->ug_jcmMd5);

    /* load the namespace UUID into MD5 context */
    jf_cghash_updateMd5(
        &pug->ug_jcmMd5, pug->ug_pu8NameSpace, JF_UUID_LEN_BIN);
    /* load the argument name string into MD5 context */
    jf_cghash_updateMd5(
        &pug->ug_jcmMd5, (u8 *)pug->ug_pstrName, ol_strlen(pug->ug_pstrName));

    /* store MD5 result into UUID */
    jf_cghash_finalMd5(&pug->ug_jcmMd5, (u8 *)&pug->ug_uoObj);

    /* brand UUID with version and variant */
    _brandUuid(pug, JF_UUID_VER_3);

    return u32Ret;
}

static u32 _makeUuidV4(uuid_gen_t * pug)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_prng_getData((void *)&pug->ug_uoObj, sizeof(pug->ug_uoObj));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /* brand UUID with version and variant */
        _brandUuid(pug, JF_UUID_VER_4);
    }
    
    return u32Ret;
}

static u32 _makeUuidV5(uuid_gen_t * pug)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 u8Sha1[JF_CGHASH_SHA1_DIGEST_LEN];

    /* initialize SHA1 context */
    jf_cghash_initSha1(&pug->ug_jcsSha1);

    /* load the namespace UUID into SHA1 context */
    u32Ret = jf_cghash_updateSha1(
        &pug->ug_jcsSha1, pug->ug_pu8NameSpace, JF_UUID_LEN_BIN);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /* load the argument name string into SHA1 context */
        u32Ret = jf_cghash_updateSha1(
            &pug->ug_jcsSha1, (u8 *)pug->ug_pstrName, ol_strlen(pug->ug_pstrName));
    }
    
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /* store SHA1 result into UUID */
        u32Ret = jf_cghash_finalSha1(&pug->ug_jcsSha1, u8Sha1);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memcpy((u8 *)&pug->ug_uoObj, u8Sha1, sizeof(pug->ug_uoObj));
        /* brand UUID with version and variant */
        _brandUuid(pug, JF_UUID_VER_5);
    }

    return u32Ret;
}

static u32 _initUuidGenV1(uuid_gen_t * pug)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_ifmgmt_getMacOfFirstIf(pug->ug_u8Mac);
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        ol_bzero(pug->ug_u8Mac, JF_LIMIT_MAC_LEN);
        pug->ug_u8Mac[0] = IEEE_MAC_MCBIT;
        u32Ret = JF_ERR_NO_ERROR;
    }

    return u32Ret;
}

static u32 _initUuidGenV3(uuid_gen_t * pug)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}
static u32 _initUuidGenV4(uuid_gen_t * pug)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

static u32 _initUuidGenV5(uuid_gen_t * pug)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

static void _copyUuidParam(uuid_gen_t * pug, jf_uuid_param_t * pjup)
{
    pug->ug_bMulticastMac = pjup->jup_bMulticastMac;
    pug->ug_pstrName = pjup->jup_pstrName;
    pug->ug_pu8NameSpace = pjup->jup_pu8NameSpace;
}

static u32 _initUuidGen(
    uuid_gen_t * pug, jf_uuid_ver_t version, jf_uuid_param_t * pjup)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    _copyUuidParam(pug, pjup);

    if (pug->ug_bInitialized)
        return u32Ret;
    
    memset(pug, 0, sizeof(uuid_gen_t));

    u32Ret = _initUuidGenV1(pug);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _initUuidGenV3(pug);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _initUuidGenV4(pug);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _initUuidGenV5(pug);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _copyUuidParam(pug, pjup);
        pug->ug_bInitialized = TRUE;
    }

    return u32Ret;
}

static u32 _checkParam(u32 u32Len, jf_uuid_ver_t version, jf_uuid_param_t * pjup)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32BufLen = JF_UUID_LEN_BIN;

    switch (version)
    {
    case JF_UUID_VER_1:
        break;
    case JF_UUID_VER_3:
        if ((pjup->jup_pstrName == NULL) || (pjup->jup_pu8NameSpace == NULL))
            u32Ret = JF_ERR_INVALID_PARAM;
        break;
    case JF_UUID_VER_4:
        break;
    case JF_UUID_VER_5:
        break;
    default:
        u32Ret = JF_ERR_INVALID_PARAM;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pjup->jup_ufFmt == JF_UUID_FMT_BIN)
            u32BufLen = JF_UUID_LEN_BIN;
        else if (pjup->jup_ufFmt == JF_UUID_FMT_STR)
            u32BufLen = JF_UUID_LEN_STR;
        else if (pjup->jup_ufFmt == JF_UUID_FMT_HEX)
            u32BufLen = JF_UUID_LEN_HEX;
        else if (pjup->jup_ufFmt == JF_UUID_FMT_SIV)
            u32BufLen = JF_UUID_LEN_SIV;
        else
            u32Ret = JF_ERR_INVALID_UUID_FORMAT;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (u32Len < u32BufLen)
            u32Ret = JF_ERR_BUFFER_TOO_SMALL;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
    }

    return u32Ret;
}

static u32 _genUuid(
    u8 * pu8Uuid, u32 u32Len, jf_uuid_ver_t version, jf_uuid_param_t * pjup)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    uuid_gen_t * pug = &ls_ugUuidGen;
    jf_uuid_fmt_t fmt = JF_UUID_FMT_BIN;

    u32Ret = _checkParam(u32Len, version, pjup);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _initUuidGen(pug, version, pjup);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (version == JF_UUID_VER_1)
        {
            u32Ret = _makeUuidV1(pug);
        }
        else if (version == JF_UUID_VER_3)
        {
            u32Ret = _makeUuidV3(pug);
        }
        else if (version == JF_UUID_VER_4)
        {
            u32Ret = _makeUuidV4(pug);
        }
        else if (version == JF_UUID_VER_5)
        {
            u32Ret = _makeUuidV5(pug);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pjup != NULL)
            fmt = pjup->jup_ufFmt;   
        u32Ret = outputUuid(&(pug->ug_uoObj), fmt, pu8Uuid, u32Len);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 jf_uuid_get(
    u8 * pu8Uuid, u32 u32Len, jf_uuid_ver_t version, jf_uuid_param_t * pjup)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert(pjup != NULL);

    u32Ret = _genUuid(pu8Uuid, u32Len, version, pjup);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


