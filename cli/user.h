/**
 *  @file user.h
 *
 *  @brief user data header file
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

#ifndef JIUFENG_CLI_USER_H
#define JIUFENG_CLI_USER_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"

/* --- constants & data structures ------------------------------------------ */


#define MAX_USER_NAME_LEN       (24)
#define MAX_PASSWORD_LEN        (48)
#define MAX_USER_FULL_NAME_LEN  (48)
#define MAX_EMAIL_LEN           (48)

#define MAX_NUM_USER	        (16)

typedef struct
{   
    olchar_t ui_strUsername[MAX_USER_NAME_LEN];
    olchar_t ui_strFullname[MAX_USER_FULL_NAME_LEN];
    olchar_t ui_strPassword[MAX_PASSWORD_LEN];
    olchar_t ui_strEmail[MAX_EMAIL_LEN];
    u8 ui_u8UserGroupId;
    boolean_t ui_bEnable;
    u8 ui_u8Reserved[6];
} user_info_t;

typedef struct
{   
    u16 ul_u16NumUser;
    u16 ul_u16Reserved[3];
    user_info_t ul_tUser[MAX_NUM_USER];
} user_list_t;

#endif /*JIUFENG_CLI_USER_H*/

/*---------------------------------------------------------------------------*/




