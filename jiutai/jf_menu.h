/**
 *  @file jf_menu.h
 *
 *  @brief Header file defines the menu object.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_menu object.
 *  -# A menu has a series of entries, the entry has many attributes, like name, description,
 *   type, handler, etc. The entry may be a menu, if that's the case, the entry has a child menu. 
 *  -# If the entry is a command, the selection of this entry will execute a given routine.
 *  -# Every menu except root menu has at least 2 entries (the top menu certainly has only one
 *   entry that is "Quit"): "Quit" and "Up". This 2 entries are built along with the creation of
 *   a menu.
 *  -# Signals are catched and ignored for menu. Use "Quit" to exit menu.
 *  -# Link with jf_string library for string operation.
 *  -# Link with jf_process common object for signal handling.
 *  -# Link with jf_jiukun library for memory allocation.
 */

#ifndef JIUTAI_MENU_H
#define JIUTAI_MENU_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the menu data type.
 */
typedef void  jf_menu_t;

/** Define the function data type. It's called before showing the menu.
 */
typedef u32 (* jf_menu_fnPreShow_t)(void * pArg);

/** Define the function data type. It's called after showing the menu.
 */
typedef u32 (* jf_menu_fnPostShow_t)(void * pArg);

/** Define the hanlder function for menu entry.
 */
typedef u32 (* jf_menu_fnHandler_t)(void * pArg);

/* --- functional routines ---------------------------------------------------------------------- */

/** Create the top menu.
 *
 *  @note
 *  -# Signal is ignored in this function. Use quit entry to exit menu.
 *
 *  @param fnPreShow [in] Routine invoked before showing the menu.
 *  @param fnPostShow [in] Routine invoked after showing the menu.
 *  @param pArg [in] The argument for the routine.
 *  @param ppMenu [out] The top menu handle.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_menu_create(
    jf_menu_fnPreShow_t fnPreShow, jf_menu_fnPostShow_t fnPostShow, void * pArg,
    jf_menu_t ** ppMenu);

/** Add a sub menu.
 *
 *  @param pParent [in] Parent of the entry.
 *  @param pstrName [in] Name of the entry.
 *  @param pstrDesc [in] Description of the entry.
 *  @param u32Attr [in] Attribute of the entry.
 *  @param fnPreShow [in] Routine invoked before showing a menu.
 *  @param fnPostShow [in] Routine invoked after showing a menu.
 *  @param pArg [in] Argument of the routine.
 *  @param ppMenu [out] Pointer to sub-menu object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OUT_OF_MEMORY Out of memory.
 */
u32 jf_menu_addSubMenu(
    jf_menu_t * pParent, const olchar_t * pstrName, const olchar_t * pstrDesc, const u32 u32Attr,
    jf_menu_fnPreShow_t fnPreShow, jf_menu_fnPostShow_t fnPostShow, void * pArg,
    jf_menu_t ** ppMenu);

/** Add an entry to the menu.
 *
 *  @param pParent [in] Parent of the entry.
 *  @param pstrName [in] Name of the entry.
 *  @param pstrDesc [in] Description of the entry.
 *  @param u32Attr [in] Attribute of the entry.
 *  @param fnHandler [in] Handler of the entry.
 *  @param pArg [in] Argument for the fnHandler.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OUT_OF_MEMORY Out of memory.
 */
u32 jf_menu_addEntry(
    jf_menu_t * pParent, const olchar_t * pstrName, const olchar_t * pstrDesc, const u32 u32Attr,
    jf_menu_fnHandler_t fnHandler, void * pArg);

/** Run menu, it will enter loop until quit.
 * 
 *  @param pTop [in] The top menu handle.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_menu_run(jf_menu_t * pTop);

/** Destroy the menu.
 *
 *  @param ppMenu [in/out] The menu object to destroy.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_menu_destroy(jf_menu_t ** ppMenu);

#endif  /*JIUTAI_MENU_H*/

/*------------------------------------------------------------------------------------------------*/
