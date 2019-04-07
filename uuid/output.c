/**
 *  @file output.c
 *
 *  @brief output UUID with specified format
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "errcode.h"
#include "uuid.h"
#include "common.h"
//#include "bignum.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */
static u32 _writeUuidBin(uuid_obj_t * puo, olchar_t * pstrUuid, olsize_t sUuid)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    uuid_uint32_t tmp32;
    uuid_uint16_t tmp16;
    u32 i;

    /* pack "uo_u32TimeLow" field */
    tmp32 = puo->uo_u32TimeLow;
    pstrUuid[3] = (uuid_uint8_t)(tmp32 & 0xff);
    tmp32 >>= 8;
    pstrUuid[2] = (uuid_uint8_t)(tmp32 & 0xff);
    tmp32 >>= 8;
    pstrUuid[1] = (uuid_uint8_t)(tmp32 & 0xff);
    tmp32 >>= 8;
    pstrUuid[0] = (uuid_uint8_t)(tmp32 & 0xff);

    /* pack "uo_u16TimeMid" field */
    tmp16 = puo->uo_u16TimeMid;
    pstrUuid[5] = (uuid_uint8_t)(tmp16 & 0xff);
    tmp16 >>= 8;
    pstrUuid[4] = (uuid_uint8_t)(tmp16 & 0xff);

    /* pack "uo_u16TimeHiAndVersion" field */
    tmp16 = puo->uo_u16TimeHiAndVersion;
    pstrUuid[7] = (uuid_uint8_t)(tmp16 & 0xff);
    tmp16 >>= 8;
    pstrUuid[6] = (uuid_uint8_t)(tmp16 & 0xff);

    /* pack "uo_u8ClockSeqHiAndReserved" field */
    pstrUuid[8] = puo->uo_u8ClockSeqHiAndReserved;

    /* pack "uo_u8ClockSeqLow" field */
    pstrUuid[9] = puo->uo_u8ClockSeqLow;

    /* pack "uo_u8Node" field */
    for (i = 0; i < (u32)sizeof(puo->uo_u8Node); i++)
        pstrUuid[10 + i] = puo->uo_u8Node[i];

    return u32Ret;
}

static u32 _writeUuidStr(uuid_obj_t * puo, olchar_t * pstrUuid, olsize_t sUuid)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /* format UUID into string representation */
    ol_sprintf(pstrUuid,
             "%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             (unsigned long)puo->uo_u32TimeLow,
             (u32)puo->uo_u16TimeMid,
             (u32)puo->uo_u16TimeHiAndVersion,
             (u32)puo->uo_u8ClockSeqHiAndReserved,
             (u32)puo->uo_u8ClockSeqLow,
             (u32)puo->uo_u8Node[0],
             (u32)puo->uo_u8Node[1],
             (u32)puo->uo_u8Node[2],
             (u32)puo->uo_u8Node[3],
             (u32)puo->uo_u8Node[4],
             (u32)puo->uo_u8Node[5]);

    return u32Ret;
}

static u32 _writeUuidHex(uuid_obj_t * puo, olchar_t * pstrUuid, olsize_t sUuid)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /* format UUID into string representation */
    ol_sprintf(pstrUuid,
            "%08lx%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
            (unsigned long)puo->uo_u32TimeLow,
            (u32)puo->uo_u16TimeMid,
            (u32)puo->uo_u16TimeHiAndVersion,
            (u32)puo->uo_u8ClockSeqHiAndReserved,
            (u32)puo->uo_u8ClockSeqLow,
            (u32)puo->uo_u8Node[0],
            (u32)puo->uo_u8Node[1],
            (u32)puo->uo_u8Node[2],
            (u32)puo->uo_u8Node[3],
            (u32)puo->uo_u8Node[4],
            (u32)puo->uo_u8Node[5]);

    return u32Ret;
}

static u32 _writeUuidSiv(uuid_obj_t * puo, olchar_t * pstrUuid, olsize_t sUuid)
{
    u32 u32Ret = JF_ERR_NOT_IMPLEMENTED;


    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
u32 outputUuid(
    uuid_obj_t * puo, jf_uuid_fmt_t fmt, olchar_t * pstrUuid, olsize_t sUuid)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (fmt == JF_UUID_FMT_BIN)
    {
        u32Ret = _writeUuidBin(puo, pstrUuid, sUuid);
    }
    else if (fmt == JF_UUID_FMT_STR)
    {
        u32Ret = _writeUuidStr(puo, pstrUuid, sUuid);
    }
    else if (fmt == JF_UUID_FMT_HEX)
    {
        u32Ret = _writeUuidHex(puo, pstrUuid, sUuid);
    }
    else if (fmt == JF_UUID_FMT_SIV)
    {
        u32Ret = _writeUuidSiv(puo, pstrUuid, sUuid);
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


