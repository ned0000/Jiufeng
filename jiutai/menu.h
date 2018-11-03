/**
 *  @file menu.h
 *
 *  @brief header file of menu common object
 *
 *  @Author Min Zhang
 *
 *  @note
 *  A menu has a series of entries, the entry has many attributes, like
 *  name, description, type, handler, etc. The entry may be a menu, if 
 *  that's the case, the entry has a child menu. 
 *  This is a little confusing, but remind the relationship between file 
 *  and directory.
 *  If the entry is a command, the selection of this entry will execute a 
 *  given routine.
 *  Every menu except root menu has at least 2 entries (the top menu 
 *  certainly has only one entry that is "exit"): "exit" and "up one 
 *  level". This 2 entries are built along with the creation of an entry.
 *
 */

#ifndef JIUTAI_MENU_H
#define JIUTAI_MENU_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */

typedef void  menu_t;

typedef u32 (* fnPreShow_t)(void * pArg);

typedef u32 (* fnPostShow_t)(void * pArg);

typedef u32 (* fnHandler_t)(void * pArg);

/* --- functional routines ------------------------------------------------- */
u32 createTopMenu(
    fnPreShow_t fnPreShow, fnPostShow_t fnPostShow, void * pArg,
    menu_t ** ppMenu);

u32 addSubMenu(
    menu_t * pParent, const olchar_t * pstrName,
    const olchar_t * pstrDesc, const u32 attr,
    fnPreShow_t fnPreShow, fnPostShow_t fnPostShow,
    void * pARg, menu_t ** ppMenu);

u32 addMenuEntry(
    menu_t * pParent, const olchar_t * pstrName,
    const olchar_t * pstrDesc, const u32 attr,
    fnHandler_t fnHandler, void * pArg);

u32 startMenu(menu_t * pTop);

#endif  // JIUTAI_MENU_H

/*--------------------------------------------------------------------------*/

