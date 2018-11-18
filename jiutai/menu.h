/**
 *  @file menu.h
 *
 *  @brief header file of menu common object
 *
 *  @author Min Zhang
 *
 *  @note a menu has a series of entries, the entry has many attributes, like
 *  name, description, type, handler, etc. The entry may be a menu, if 
 *  that's the case, the entry has a child menu. 
 *  @note If the entry is a command, the selection of this entry will execute a 
 *  given routine.
 *  @note Every menu except root menu has at least 2 entries (the top menu 
 *  certainly has only one entry that is "exit"): "exit" and "up one 
 *  level". This 2 entries are built along with the creation of an entry.
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

/** Create the top menu
 *
 *  @param fnPreShow [in] routine invoked before showing the menu
 *  @param fnPostShow [in] routine invoked after showing the menu
 *  @param pArg [in] the argument for the routine
 *  @param ppMenu [in/out] the top menu handle
 *
 *  @return
 */
u32 createTopMenu(
    fnPreShow_t fnPreShow, fnPostShow_t fnPostShow, void * pArg,
    menu_t ** ppMenu);

/** Add a sub menu 
 *
 *  @param pParent [in] parent of the entry
 *  @param pstrName [in] name of the entry
 *  @param pstrDesc [in] description of the entry
 *  @param attr [in] attribute of the entry
 *  @param fnPreShow [in] routine invoked before entering a menu
 *  @param fnPostShow [in] routine invoked after leaving a menu
 *  @param pArg [in] argument of the routine
 *  @param ppMenu [in/out] pointer to sub-menu handle
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *  @retval OLERR_OUT_OF_MEMORY out of memory
 *
 */
u32 addSubMenu(
    menu_t * pParent, const olchar_t * pstrName,
    const olchar_t * pstrDesc, const u32 attr,
    fnPreShow_t fnPreShow, fnPostShow_t fnPostShow,
    void * pArg, menu_t ** ppMenu);

/** Add an entry to the menu 
 *
 *  @param pParent [in] parent of the entry
 *  @param pstrName [in] name of the entry
 *  @param pstrDesc [in] description of the entry
 *  @param attr [in] attribute of the entry
 *  @param fnHandler [in] handler of the entry
 *  @param pArg [in] argument for the fnHandler
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *  @retval OLERR_OUT_OF_MEMORY out of memory
 *
 */
u32 addMenuEntry(
    menu_t * pParent, const olchar_t * pstrName,
    const olchar_t * pstrDesc, const u32 attr,
    fnHandler_t fnHandler, void * pArg);

/** Start menu
 * 
 *  @param pTop [in] the top menu handle
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
u32 startMenu(menu_t * pTop);

#endif  // JIUTAI_MENU_H

/*--------------------------------------------------------------------------*/

