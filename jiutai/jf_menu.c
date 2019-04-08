/**
 *  @file menu.c
 *
 *  @brief implementation of menu
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "xmalloc.h"
#include "jf_menu.h"
#include "jf_err.h"
#include "stringparse.h"

/* --- private data/data structure section --------------------------------- */
struct internal_menu;
struct internal_menu_entry;

typedef enum 
{ 
    ENTRY_TYPE_UNKNOWN,
    ENTRY_TYPE_MENU,
    ENTRY_TYPE_COMMAND,
    ENTRY_TYPE_QUIT,
    ENTRY_TYPE_UPONELEVEL,
} entry_type_t;

/** The entry definition
 */
typedef struct internal_menu_entry
{
    /** Name of the entry */
    olchar_t * ime_pstrName;
    /** Description of the entry */
    olchar_t * ime_pstrDesc;
    /** Attribute of the entry */
    u32 ime_u32Attribute;

    /** Type of the entry */
    entry_type_t ime_etType;
    /** Value of the entry, determined by the type of the entry */
    union
    {
        /** A routine for the "command" entry */
        struct
        {
            jf_menu_fnHandler_t is_fnHandler;
            void * is_pArg;
        } iu_stCommand;
        /** The entry is a menu */
        struct internal_menu *iu_imMenu;
    } ime_uContent;

    /** The parent menu containing this entry */
    struct internal_menu *ime_imParent;

    /** The next entry in this menu */
    struct internal_menu_entry *ime_imeNext;
} internal_menu_entry_t;

/** name Attribute of entry, up-one-level entry
 */
#define  MENU_ENTRY_ATTR_UP_ONE_LEVEL        (0x1)

/** name Attribute of entry, quit entry
 */
#define  MENU_ENTRY_ATTR_QUIT                (0x2)


/** The entry is an up-one-level entry
 */
#define  set_uponelevel_menu_entry(entry)   \
    (entry->ime_u32Attribute |= MENU_ENTRY_ATTR_UP_ONE_LEVEL)

/** The entry is a quit entry
 */
#define  set_quit_menu_entry(entry)   \
    (entry->ime_u32Attribute |= MENU_ENTRY_ATTR_QUIT)

/** Verify up-one-level entry
 */
#define  is_uponelevel_menu_entry(entry)    \
    (entry->ime_u32Attribute & MENU_ENTRY_ATTR_UP_ONE_LEVEL)

/** Verify quit entry
 */
#define  is_quit_menu_entry(entry) \
    (entry->ime_u32Attribute & MENU_ENTRY_ATTR_QUIT)

/** The menu definition
 */
typedef struct internal_menu
{
    /** Entries of this menu */
    internal_menu_entry_t *im_imeEntries;
    /** The flag indicates the quit of menu, that is available for top menu */
    boolean_t im_bQuit;

    /** The up level menu */
    struct internal_menu *im_imParent;

    /** Before showing the menu, the handler is invoked */
    jf_menu_fnPreShow_t im_fnPreShow;
    /** After showing the menu, the handler is invoked */
    jf_menu_fnPostShow_t im_fnPostShow;
    /** Argument of the functions above */
    void * im_pArg;
} internal_menu_t;

/* --- private routine section---------------------------------------------- */

static internal_menu_t * _getTopMenu(internal_menu_t * pMenu)
{
    while (pMenu->im_imParent != NULL)
        pMenu = pMenu->im_imParent;

    return pMenu;
}

static olint_t _quitMenu(internal_menu_t * pMenu)
{
    internal_menu_t *pTop;

    pTop = _getTopMenu(pMenu);

    pTop->im_bQuit = TRUE;

    return 0;
}

static u32 _newMenuEntry(
    internal_menu_t * pParent, const olchar_t * pstrName, 
    const olchar_t * pstrDesc, const u32 attr, internal_menu_entry_t ** ppEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_menu_entry_t *pEntry;

    u32Ret = jf_mem_alloc((void **)&pEntry, sizeof(internal_menu_entry_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(pEntry, 0, sizeof(internal_menu_entry_t));
        pEntry->ime_u32Attribute = attr;
        pEntry->ime_imParent = pParent;

        u32Ret = jf_string_duplicate(&(pEntry->ime_pstrName), pstrName);
        if (u32Ret != JF_ERR_NO_ERROR)
        {
            jf_mem_free((void **)&pEntry);
        }
        else
        {
            if (pstrDesc != NULL)
            {
                u32Ret = jf_string_duplicate(&(pEntry->ime_pstrDesc), pstrDesc);
                if (u32Ret != JF_ERR_NO_ERROR)
                {
                    jf_mem_free((void **)&(pEntry->ime_pstrName));
                    jf_mem_free((void **)&pEntry);
                }
            }
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppEntry = pEntry;

    return u32Ret;
}

static u32 _destroyEntry(internal_menu_entry_t ** ppEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    free(*ppEntry);
    *ppEntry = NULL;

    return u32Ret;
}

static u32 _newQuitEntry(
    internal_menu_t * pParent, internal_menu_entry_t ** ppEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_menu_entry_t * pEntry;

    u32Ret = _newMenuEntry(pParent, "quit", NULL, 0, &pEntry);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pEntry->ime_etType = ENTRY_TYPE_QUIT;
        *ppEntry = pEntry;
    }

    return u32Ret;
}

static u32 _newUponelevelEntry(
    internal_menu_t * pParent, internal_menu_entry_t ** ppEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_menu_entry_t * pEntry;

    u32Ret = _newMenuEntry(pParent, "up", "up one level", 0, &pEntry);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pEntry->ime_etType = ENTRY_TYPE_UPONELEVEL;
        *ppEntry = pEntry;
    }

    return u32Ret;
}

static u32 _addEntry(internal_menu_t * pParent, internal_menu_entry_t *pEntry)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pEntry->ime_imeNext = pParent->im_imeEntries;
    pParent->im_imeEntries = pEntry;

    return u32Ret;
}

static u32 _newMenu(
    internal_menu_t * pParent, jf_menu_fnPreShow_t fnPreShow,
    jf_menu_fnPostShow_t fnPostShow, void * pArg, internal_menu_t ** ppMenu)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_menu_t *pMenu;
    internal_menu_entry_t *pEntry;

    u32Ret = jf_mem_alloc((void **)&pMenu, sizeof(internal_menu_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(pMenu, 0, sizeof(internal_menu_t));

        pMenu->im_fnPreShow = fnPreShow;
        pMenu->im_fnPostShow = fnPostShow;
        pMenu->im_imParent = pParent;
        pMenu->im_pArg = pArg;

        // setup the quit entry
        u32Ret = _newQuitEntry(pMenu, &pEntry);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            _addEntry(pMenu, pEntry);
            if (pParent != NULL)
            {
                u32Ret = _newUponelevelEntry(pMenu, &pEntry);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    _addEntry(pMenu, pEntry);
                }
                else
                {
                    jf_mem_free((void **)&pMenu);
                    _destroyEntry(&pEntry);
                }
            }
        }
        else
        {
            jf_mem_free((void **)&pMenu);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        *ppMenu = pMenu;
    }

    return u32Ret;
}

/** Print menu and wait for user's selection and execute corresponding routines
 *
 *  @param pMenu [in] the internal menu data structure
 *
 *  @return internal menu data structure
 */
static internal_menu_t * _showMenu(internal_menu_t * pMenu)
{
    internal_menu_entry_t *pEntry;
    olint_t i;
    boolean_t bValid;
    olsize_t len, j;
    internal_menu_t *ret = pMenu;
    olint_t choice;
    olchar_t buf[30];

    ol_printf("\n");

    i = 0;
    pEntry = pMenu->im_imeEntries;
    while (pEntry != NULL)
    {
        // The display for sub-menu is a little different from the command
        if (pEntry->ime_etType == ENTRY_TYPE_MENU)
            ol_printf("[%d] %s ->\n", i++, pEntry->ime_pstrName);
        else
            ol_printf("[%d] %s\n", i++, pEntry->ime_pstrName);
        pEntry = pEntry->ime_imeNext;
    }

    // Receive user's input. user doesn't need to input the name of the menu
    //  but rather select the index of the entry
    bValid = TRUE;
    do
    {
        memset(buf, 0, 30);
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

    // Get the entry according to user's selection
    i = 0;
    pEntry = pMenu->im_imeEntries;
    while (pEntry != NULL)
    {
        if (i++ == choice)
            break;
        pEntry = pEntry->ime_imeNext;
    }

    // process the selection 
    switch (pEntry->ime_etType)
    {
    case ENTRY_TYPE_COMMAND:
        pEntry->ime_uContent.iu_stCommand.is_fnHandler(
            pEntry->ime_uContent.iu_stCommand.is_pArg);
        break;
    case ENTRY_TYPE_MENU:
        ret = pEntry->ime_uContent.iu_imMenu;
        // before entering the menu, do the initializing stuff
        if (ret->im_fnPreShow != NULL)
            ret->im_fnPreShow(ret->im_pArg);
        break;
    case ENTRY_TYPE_QUIT:
        _quitMenu(pEntry->ime_imParent);
        break;
    case ENTRY_TYPE_UPONELEVEL:
        // before upping one level, make clean
        if (ret->im_fnPostShow != NULL)
            ret->im_fnPostShow(ret->im_pArg);
        // if the uponelevel menu is NULL, then this is the top menu
        ret = (pEntry->ime_imParent->im_imParent == NULL) ?
            pEntry->ime_imParent : pEntry->ime_imParent->im_imParent;
        break;
    default:
        break;
    }

    return ret;
}

static olint_t _destroyMenu(internal_menu_t * pTop)
{
    internal_menu_entry_t *p, *p1;

    p = pTop->im_imeEntries;
    while (p != NULL)
    {
        p1 = p->ime_imeNext;

        free(p->ime_pstrName);
        p->ime_pstrName = NULL;
        if (p->ime_pstrDesc != NULL)
        {
            free(p->ime_pstrDesc);
            p->ime_pstrDesc = NULL;
        }
        if (p->ime_etType == ENTRY_TYPE_MENU)
            _destroyMenu(p->ime_uContent.iu_imMenu);

        free(p);

        p = p1;
    }

    return 0;
}

/* --- public routine section ---------------------------------------------- */

u32 jf_menu_createTopMenu(
    jf_menu_fnPreShow_t fnPreShow, jf_menu_fnPostShow_t fnPostShow, void * pArg,
    jf_menu_t ** ppMenu)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _newMenu(
        NULL, fnPreShow, fnPostShow, pArg, (internal_menu_t **)ppMenu);

    return u32Ret;
}

u32 jf_menu_addEntry(
    jf_menu_t * pParent, const olchar_t * pstrName, const olchar_t * pstrDesc,
    const u32 attr, jf_menu_fnHandler_t fnHandler, void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_menu_entry_t * pEntry;
    internal_menu_t * pParentMenu = (internal_menu_t *)pParent;

    assert((pstrName != NULL) && (pParent != NULL) &&
           (fnHandler != NULL));

    u32Ret = _newMenuEntry(pParentMenu, pstrName, pstrDesc, attr, &pEntry);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pEntry->ime_etType = ENTRY_TYPE_COMMAND;
        pEntry->ime_uContent.iu_stCommand.is_pArg = pArg;
        pEntry->ime_uContent.iu_stCommand.is_fnHandler = fnHandler;

        _addEntry(pParentMenu, pEntry);
    }

    return u32Ret;
}

u32 jf_menu_addSubMenu(
    jf_menu_t * pParent, const olchar_t * pstrName,
    const olchar_t * pstrDesc, const u32 attr,
    jf_menu_fnPreShow_t fnPreShow, jf_menu_fnPostShow_t fnPostShow,
    void * pArg, jf_menu_t ** ppMenu)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_menu_entry_t * pEntry;
    internal_menu_t * pMenu;
    internal_menu_t * pParentMenu = (internal_menu_t *)pParent;

    assert((pstrName != NULL) && (pParent != NULL));

    u32Ret = _newMenuEntry(pParentMenu, pstrName, pstrDesc, attr, &pEntry);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pEntry->ime_etType = ENTRY_TYPE_MENU;
        u32Ret = _newMenu(pParentMenu, fnPreShow, fnPostShow, pArg, &pMenu);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            pEntry->ime_uContent.iu_imMenu = pMenu;
            *ppMenu = pMenu;
            _addEntry(pParent, pEntry);
        }
        else
        {
            _destroyEntry(&pEntry);
        }
    }

    return u32Ret;
}

u32 jf_menu_start(jf_menu_t * pTop)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_menu_t * p, * pMenu = (internal_menu_t *)pTop;

    assert(pMenu != NULL && pMenu->im_imParent == NULL);

    if (pMenu->im_fnPreShow != NULL)
        pMenu->im_fnPreShow(pMenu->im_pArg);

    p = pMenu;
    while (! pMenu->im_bQuit)
    {
        p = _showMenu(p);
    }

    if (pMenu->im_fnPostShow != NULL)
        pMenu->im_fnPostShow(pMenu->im_pArg);

    _destroyMenu(pMenu);

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

