/**
 *  @file archivecommon.h
 *
 *  @brief common data structure and macro definition of archive library 
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

#ifndef ARCHIVE_COMMON_H
#define ARCHIVE_COMMON_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "arfile.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */
/* values for ah_u8Type field */
#define AH_TYPE_REGULAR   0x30    /* regular file */
#define AH_TYPE_HARD_LINK 0x31    /* hard link */
#define AH_TYPE_SYM_LINK  0x32    /* symbolic link */
#define AH_TYPE_CHAR      0x33    /* character device */
#define AH_TYPE_BLOCK     0x34    /* block device */
#define AH_TYPE_DIR       0x35    /* directory */
#define AH_TYPE_FIFO      0x36    /* FIFO file */
#define AH_TYPE_SOCK      0x37    /* Socket file */

/* values for ah_u8NameLen field */
#define AH_NAME_LEN_LONG  0x4c    /* long name */

/* values for ah_u8Magic */
#define AH_MAGIC  "oar"

/* version */
#define AH_VERSION "01"

/* values for ah_u8DirDepth */
#define AH_DIR_DEPTH_NULL 0xFF

/* file mode */
#define AH_MODE_SUID   0x00000800
#define AH_MODE_SGID   0x00000400
#define AH_MODE_SVTX   0x00000200
#define AH_MODE_UREAD  0x00000100
#define AH_MODE_UWRITE 0x00000080
#define AH_MODE_UEXEC  0x00000040
#define AH_MODE_GREAD  0x00000020
#define AH_MODE_GWRITE 0x00000010
#define AH_MODE_GEXEC  0x00000008
#define AH_MODE_OREAD  0x00000004
#define AH_MODE_OWRITE 0x00000002
#define AH_MODE_OEXEC  0x00000001

/* length of the fields */
#define AH_NAME_LEN 64
#define AH_MODE_LEN 16
#define AH_USER_ID_LEN 8
#define AH_USER_GROUP_ID_LEN 8
#define AH_SIZE_LEN 24
#define AH_TIME_LEN 16
#define AH_MAGIC_LEN 6
#define AH_VERSION_LEN 2
#define AH_USER_NAME_LEN 32
#define AH_GROUP_NAME_LEN 32

typedef struct archive_header
{                             /* byte offset */
    u8 ah_u8HeaderFlags[13];
    u8 ah_u8DirDepth;
    u8 ah_u8NameLen;
    u8 ah_u8Type;
    u8 ah_u8Name[AH_NAME_LEN];
    u8 ah_u8Mode[AH_MODE_LEN];
    u8 ah_u8UserId[AH_USER_ID_LEN];
    u8 ah_u8GroupId[AH_USER_GROUP_ID_LEN];
    u8 ah_u8Size[AH_SIZE_LEN];
    u8 ah_u8ModifyTime[AH_TIME_LEN];
    u8 ah_u8Magic[6];
    u8 ah_u8Version[AH_VERSION_LEN];
    u8 ah_u8Chksum[8];
    u8 ah_u8UserName[AH_USER_NAME_LEN];
    u8 ah_u8GroupName[AH_GROUP_NAME_LEN];
} archive_header_t;

#define ARCHIVE_BLOCK_SIZE  512


typedef union archive_block
{
    u8 ab_u8Buffer[ARCHIVE_BLOCK_SIZE];
    archive_header_t ab_ahHeader;
} archive_block_t;

#define MAX_BLOCKS    120

#define MAX_ARCHIVE_BUFFER_LEN  (sizeof(archive_block_t) * MAX_BLOCKS)

typedef struct
{
    ar_file_t * fh_pafArchive;
    olchar_t fh_strFullpath[JF_LIMIT_MAX_PATH_LEN];
    u8 * fh_pu8Buffer;
    olsize_t fh_sBuf;
    boolean_t fh_bListArchive;
    boolean_t fh_bVerbose;
    u8 fh_u8Reserved[6];
    u8 fh_u8Reserved2[56];
} file_handler_t;

#define ARCHIVE_ALIGH_SIZE(size) \
    (((size - 1) & (~(ARCHIVE_BLOCK_SIZE - 1))) + ARCHIVE_BLOCK_SIZE)

/* --- functional routines ------------------------------------------------- */


#endif /*ARCHIVE_COMMON_H*/

/*---------------------------------------------------------------------------*/


