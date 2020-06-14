/**
 *  @file jf_menu.c
 *
 *  @brief Implementation file for menu object.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_jiukun.h"
#include "jf_menu.h"
#include "jf_err.h"
#include "jf_string.h"
#include "jf_process.h"

/* --- private data/data structure section ------------------------------------------------------ */

struct internal_menu;

struct internal_menu_entry;

typedef enum 
{ 
    ENTRY_TYPE_UNKNOWN,
    ENTRY_TYPE_MENU,
    ENTRY_TYPE_COMMAND,
    ENTRY_TYPE_QUIT,
    ENTRY_TYPE_UP_ONE_LEVEL,
} entry_type_t;

/** The entry definition
 */
typedef struct internal_menu_entry
{
    /**Name of the entry.*/
    olchar_t * ime_pstrName;
    /**Description of the entry.*/
    olchar_t * ime_pstrDesc;
    /**Attribute of the entry.*/
    u32 ime_u32Attribute;

    /**Type of the entry.*/
    entry_type_t ime_etType;
    /**Value of the entry, determined by the type of the entry.*/
    union
    {
        /**A routine for the "command" entry.*/
        struct
        {
            jf_menu_fnHandler_t is_fnHandler;
            void * is_pArg;
        } ime_stCommand;
        /**The entry is a menu.*/
        struct internal_menu * ime_pimMenu;
    };

    /**The parent menu containing this entry.*/
    struct internal_menu * ime_pimParent;

    /**The next entry in this menu.*/
    struct internal_menu_entry * ime_pimeNext;
} internal_menu_entry_t;

/** The menu definition.
 */
typedef struct internal_menu
{
    /**Entries of this menu.*/
    internal_menu_entry_t * im_pimeHead;
    /**Last entry of this menu.*/
    internal_menu_entry_t * im_pimeTail;
    /**The flag indicates the quit of menu, only available for top menu.*/
    boolean_t im_bQuit;
    u8 im_u8Reserved[7];

    /**The up level menu.*/
    struct internal_menu * im_pimParent;

    /**Before showing the menu, the handler is invoked.*/
    jf_menu_fnPreShow_t im_fnPreShow;
    /**After showing the menu, the handler is invoked.*/
    jf_menu_fnPostShow_t im_fnPostShow;
    /**Argument of the functions above.*/
    void * im_pArg;
} internal_menu_t;

/* --- private routine section ------------------------------------------------------------------ */

static internal_menu_t * _getTopMenu(internal_menu_t * pMenu)
{
    while (pMenu->im_pimParent != NULL)
        pMenu = pMenu->im_pimParent;

    return pMenu;
}

static olint_t _quitMenu(internal_menu_t * pMenu)
{
    internal_menu_t *pTop;

    pTop = _getTopMenu(pMenu);

    pTop->im_bQuit = TRUE;

    return 0;
}

static u32 _destroyMenuEntry(internal_menu_entry_t ** ppEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_menu_entry_t * pEntry = *ppEntry;

    if (pEntry->ime_pstrName != NULL)
        jf_string_free(&pEntry->ime_pstrName);

    if (pEntry->ime_pstrDesc != NULL)
        jf_string_free(&pEntry->ime_pstrDesc);

    jf_jiukun_freeMemory((void **)ppEntry);

    return u32Ret;
}

static u32 _newMenuEntry(
    internal_menu_t * pParent, const olchar_t * pstrName, 
    const olchar_t * pstrDesc, const u32 attr, internal_menu_entry_t ** ppEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_menu_entry_t *pEntry;

    u32Ret = jf_jiukun_allocMemory((void **)&pEntry, sizeof(internal_menu_entry_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pEntry, sizeof(internal_menu_entry_t));
        pEntry->ime_u32Attribute = attr;
        pEntry->ime_pimParent = pParent;

        u32Ret = jf_string_duplicate(&pEntry->ime_pstrName, pstrName);
    }

    if ((u32Ret == JF_ERR_NO_ERROR) && (pstrDesc != NULL))
        u32Ret = jf_string_duplicate(&pEntry->ime_pstrDesc, pstrDesc);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppEntry = pEntry;
    else if (pEntry != NULL)
        _destroyMenuEntry(&pEntry);

    return u32Ret;
}

static u32 _newQuitEntry(internal_menu_t * pParent, internal_menu_entry_t ** ppEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_menu_entry_t * pEntry = NULL;

    u32Ret = _newMenuEntry(pParent, "Quit", NULL, 0, &pEntry);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pEntry->ime_etType = ENTRY_TYPE_QUIT;

    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppEntry = pEntry;

    return u32Ret;
}

static u32 _newUpOneLevelEntry(
    internal_menu_t * pParent, internal_menu_entry_t ** ppEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_menu_entry_t * pEntry;

    u32Ret = _newMenuEntry(pParent, "Up", "Up one level", 0, &pEntry);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pEntry->ime_etType = ENTRY_TYPE_UP_ONE_LEVEL;

    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppEntry = pEntry;

    return u32Ret;
}

static u32 _addEntry(internal_menu_t * pParent, internal_menu_entry_t * pEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pParent->im_pimeHead == NULL)
        pParent->im_pimeHead = pEntry;
    else
        pParent->im_pimeTail->ime_pimeNext = pEntry;

    pParent->im_pimeTail = pEntry;

    return u32Ret;
}

static u32 _newMenu(
    internal_menu_t * pParent, jf_menu_fnPreShow_t fnPreShow, jf_menu_fnPostShow_t fnPostShow,
    void * pArg, boolean_t bUpOneLevel, internal_menu_t ** ppMenu)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_menu_t * pMenu = NULL;
    internal_menu_entry_t *pEntry = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&pMenu, sizeof(internal_menu_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pMenu, sizeof(*pMenu));

        pMenu->im_fnPreShow = fnPreShow;
        pMenu->im_fnPostShow = fnPostShow;
        pMenu->im_pimParent = pParent;
        pMenu->im_pArg = pArg;

        /*Setup the quit entry.*/
        u32Ret = _newQuitEntry(pMenu, &pEntry);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _addEntry(pMenu, pEntry);

    if ((u32Ret == JF_ERR_NO_ERROR) && bUpOneLevel)
    {
        u32Ret = _newUpOneLevelEntry(pMenu, &pEntry);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _addEntry(pMenu, pEntry);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppMenu = pMenu;
    else if (pMenu != NULL)
        jf_menu_destroy((jf_menu_t **)&pMenu);

    return u32Ret;
}

/** Print menu and wait for user's selection and execute corresponding routines.
 *
 *  @param pMenu [in] The internal menu data structure.
 *
 *  @return Internal menu data structure.
 */
static internal_menu_t * _showMenu(internal_menu_t * pMenu)
{
    internal_menu_entry_t * pEntry = NULL;
    olint_t i = 0;
    boolean_t bValid = TRUE;
    olsize_t len = 0, j = 0;
    internal_menu_t * ret = pMenu;
    olint_t choice = 0;
    olchar_t buf[30];

    ol_printf("\n");

    i = 0;
    pEntry = pMenu->im_pimeHead;
    while (pEntry != NULL)
    {
        /*The display for sub-menu is a little different from the command.*/
        if (pEntry->ime_etType == ENTRY_TYPE_MENU)
            ol_printf("[%d] %s ->\n", i++, pEntry->ime_pstrName);
        else
            ol_printf("[%d] %s\n", i++, pEntry->ime_pstrName);
        pEntry = pEntry->ime_pimeNext;
    }

    /*Receive user's input. user doesn't need to input the name of the menu but rather select the
      index of the entry.*/
    do
    {
        ol_bzero(buf, 30);
        ol_printf("Please select [0-%d]: ", i - 1);
        fgets(buf, sizeof(buf) - 1, stdin);
        len = ol_strlen(buf);
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';
        for (j = 0; j < ol_strlen(buf); j++)
            if (buf[j] < '0' || buf[j] > '9')
            {
                bValid = FALSE;
                break;
            }

        if ((*buf == '\0') || (! bValid))
            continue;

        choice = atoi(buf);
    } while (choice < 0 || choice > i - 1);

    /*Get the entry according to user's selection.*/
    i = 0;
    pEntry = pMenu->im_pimeHead;
    while (pEntry != NULL)
    {
        if (i++ == choice)
            break;
        pEntry = pEntry->ime_pimeNext;
    }

    /*Process the selection.*/
    switch (pEntry->ime_etType)
    {
    case ENTRY_TYPE_COMMAND:
        pEntry->ime_stCommand.is_fnHandler(pEntry->ime_stCommand.is_pArg);
        break;
    case ENTRY_TYPE_MENU:
        ret = pEntry->ime_pimMenu;
        /*Before entering the menu, do the initializing stuff.*/
        if (ret->im_fnPreShow != NULL)
            ret->im_fnPreShow(ret->im_pArg);
        break;
    case ENTRY_TYPE_QUIT:
        _quitMenu(pEntry->ime_pimParent);
        break;
    case ENTRY_TYPE_UP_ONE_LEVEL:
        /*Before upping one level, make clean.*/
        if (ret->im_fnPostShow != NULL)
            ret->im_fnPostShow(ret->im_pArg);
        /*If the up one level menu is NULL, then this is the top menu.*/
        ret = (pEntry->ime_pimParent->im_pimParent == NULL) ?
            pEntry->ime_pimParent : pEntry->ime_pimParent->im_pimParent;
        break;
    default:
        break;
    }

    return ret;
}

static u32 _destroyMenu(internal_menu_t ** ppMenu)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_menu_t * pMenu = *ppMenu;
    internal_menu_entry_t * pEntry = NULL, * pTemp = NULL;

    pEntry = pMenu->im_pimeHead;
    while (pEntry != NULL)
    {
        pTemp = pEntry->ime_pimeNext;

        if (pEntry->ime_etType == ENTRY_TYPE_MENU)
            _destroyMenu(&pEntry->ime_pimMenu);

        _destroyMenuEntry(&pEntry);

        pEntry = pTemp;
    }

    jf_jiukun_freeMemory((void **)ppMenu);

    return u32Ret;
}

static void _terminateMenu(olint_t signal)
{
    /*Do nothing. Use "Quit" to exit menu.*/
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_menu_create(
    jf_menu_fnPreShow_t fnPreShow, jf_menu_fnPostShow_t fnPostShow, void * pArg,
    jf_menu_t ** ppMenu)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _newMenu(NULL, fnPreShow, fnPostShow, pArg, FALSE, (internal_menu_t **)ppMenu);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_process_registerSignalHandlers(_terminateMenu);

    return u32Ret;
}

u32 jf_menu_addEntry(
    jf_menu_t * pParent, const olchar_t * pstrName, const olchar_t * pstrDesc,
    const u32 u32Attr, jf_menu_fnHandler_t fnHandler, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_menu_entry_t * pEntry = NULL;
    internal_menu_t * pParentMenu = (internal_menu_t *)pParent;

    assert((pstrName != NULL) && (pParent != NULL) && (fnHandler != NULL));

    u32Ret = _newMenuEntry(pParentMenu, pstrName, pstrDesc, u32Attr, &pEntry);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pEntry->ime_etType = ENTRY_TYPE_COMMAND;
        pEntry->ime_stCommand.is_pArg = pArg;
        pEntry->ime_stCommand.is_fnHandler = fnHandler;

        u32Ret = _addEntry(pParentMenu, pEntry);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        ;
    else if (pEntry != NULL)
        _destroyMenuEntry(&pEntry);

    return u32Ret;
}

u32 jf_menu_addSubMenu(
    jf_menu_t * pParent, const olchar_t * pstrName, const olchar_t * pstrDesc, const u32 u32Attr,
    jf_menu_fnPreShow_t fnPreShow, jf_menu_fnPostShow_t fnPostShow, void * pArg,
    jf_menu_t ** ppMenu)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_menu_entry_t * pEntry = NULL;
    internal_menu_t * pMenu = NULL;
    internal_menu_t * pParentMenu = (internal_menu_t *)pParent;

    assert((pstrName != NULL) && (pParent != NULL));

    u32Ret = _newMenuEntry(pParentMenu, pstrName, pstrDesc, u32Attr, &pEntry);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pEntry->ime_etType = ENTRY_TYPE_MENU;

        u32Ret = _newMenu(pParentMenu, fnPreShow, fnPostShow, pArg, TRUE, &pMenu);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pEntry->ime_pimMenu = pMenu;

        u32Ret = _addEntry(pParentMenu, pEntry);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppMenu = pMenu;
    else
        jf_menu_destroy((jf_menu_t **)&pMenu);

    return u32Ret;
}

u32 jf_menu_run(jf_menu_t * pTop)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_menu_t * p = NULL, * pMenu = (internal_menu_t *)pTop;

    assert((pMenu != NULL) && (pMenu->im_pimParent == NULL));

    if (pMenu->im_fnPreShow != NULL)
        pMenu->im_fnPreShow(pMenu->im_pArg);

    p = pMenu;
    while (! pMenu->im_bQuit)
    {
        p = _showMenu(p);
    }

    if (pMenu->im_fnPostShow != NULL)
        pMenu->im_fnPostShow(pMenu->im_pArg);

    return u32Ret;
}

u32 jf_menu_destroy(jf_menu_t ** ppMenu)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_menu_t * pTop = *ppMenu;

    *ppMenu = NULL;
    _destroyMenu(&pTop);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
