/**
 *  @file jf_menu.h
 *
 *  @brief Header file defines the menu object.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# A menu has a series of entries, the entry has many attributes, like name, description,
 *   type, handler, etc. The entry may be a menu, if that's the case, the entry has a child menu. 
 *  -# If the entry is a command, the selection of this entry will execute a given routine.
 *  -# Every menu except root menu has at least 2 entries (the top menu certainly has only one
 *   entry that is "exit"): "exit" and "up one level". This 2 entries are built along with the
 *   creation of an entry.
 */

#ifndef JIUTAI_MENU_H
#define JIUTAI_MENU_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

typedef void  jf_menu_t;

typedef u32 (* jf_menu_fnPreShow_t)(void * pArg);

typedef u32 (* jf_menu_fnPostShow_t)(void * pArg);

typedef u32 (* jf_menu_fnHandler_t)(void * pArg);

/* --- functional routines ---------------------------------------------------------------------- */

/** Create the top menu.
 *
 *  @param fnPreShow [in] Routine invoked before showing the menu.
 *  @param fnPostShow [in] Routine invoked after showing the menu.
 *  @param pArg [in] The argument for the routine
 *  @param ppMenu [in/out] The top menu handle
 *
 *  @return
 */
u32 jf_menu_createTopMenu(
    jf_menu_fnPreShow_t fnPreShow, jf_menu_fnPostShow_t fnPostShow, void * pArg,
    jf_menu_t ** ppMenu);

/** Add a sub menu 
 *
 *  @param pParent [in] Parent of the entry.
 *  @param pstrName [in] Name of the entry.
 *  @param pstrDesc [in] Description of the entry.
 *  @param attr [in] Attribute of the entry.
 *  @param fnPreShow [in] Routine invoked before entering a menu.
 *  @param fnPostShow [in] Routine invoked after leaving a menu.
 *  @param pArg [in] Argument of the routine.
 *  @param ppMenu [in/out] Pointer to sub-menu handle.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OUT_OF_MEMORY Out of memory.
 *
 */
u32 jf_menu_addSubMenu(
    jf_menu_t * pParent, const olchar_t * pstrName, const olchar_t * pstrDesc, const u32 attr,
    jf_menu_fnPreShow_t fnPreShow, jf_menu_fnPostShow_t fnPostShow, void * pArg,
    jf_menu_t ** ppMenu);

/** Add an entry to the menu.
 *
 *  @param pParent [in] Parent of the entry.
 *  @param pstrName [in] Name of the entry.
 *  @param pstrDesc [in] Description of the entry.
 *  @param attr [in] Attribute of the entry.
 *  @param fnHandler [in] Handler of the entry.
 *  @param pArg [in] Argument for the fnHandler.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OUT_OF_MEMORY out of memory
 *
 */
u32 jf_menu_addEntry(
    jf_menu_t * pParent, const olchar_t * pstrName, const olchar_t * pstrDesc, const u32 attr,
    jf_menu_fnHandler_t fnHandler, void * pArg);

/** Start menu
 * 
 *  @param pTop [in] The top menu handle.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_menu_start(jf_menu_t * pTop);

#endif  /*JIUTAI_MENU_H*/

/*------------------------------------------------------------------------------------------------*/

