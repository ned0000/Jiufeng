/**
 *  @file array.c
 *
 *  @brief basic array implementation
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "array.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */

/* a single list */
typedef struct internal_jf_array_node_t
{
    jf_array_element_t * ijan_pjaeElement;
    struct internal_jf_array_node_t * ijan_pijanNext;
} internal_jf_array_node_t;

typedef struct
{
    u32 ija_u32ArraySize;
    internal_jf_array_node_t * ija_pijanElements;
} internal_jf_array_t;

/* --- private routine section ------------------------------------------------ */

static u32 _getElementAt(
    internal_jf_array_t * pija, u32 u32Index, jf_array_element_t ** ppjae)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_array_node_t *pijan;
    u32 u32Pos;

    u32Pos = 0;
    pijan = pija->ija_pijanElements;
    while ((u32Pos < u32Index) && (pijan != NULL))
    {
        pijan = pijan->ijan_pijanNext;
        u32Pos = u32Pos + 1;
    }

    if ((u32Pos == u32Index) && (pijan != NULL))
        *ppjae = pijan->ijan_pjaeElement;
    else
        u32Ret = JF_ERR_OUT_OF_RANGE;

    return u32Ret;
}

static u32 _removeElementAt(
    internal_jf_array_t * pija, u32 u32Index,
    jf_array_fnDestroyElement_t fnDestoryElement)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_array_node_t * pijanPrevious, * pijan;
    u32 u32Pos;

    u32Pos = 0;
    pijanPrevious = NULL;
    pijan = pija->ija_pijanElements;
    while ((u32Pos < u32Index) && (pijan != NULL))
    {
        pijanPrevious = pijan;
        pijan = pijan->ijan_pijanNext;
        u32Pos = u32Pos + 1;
    }

    if ((u32Pos != u32Index) || (pijan == NULL))
        u32Ret = JF_ERR_OUT_OF_RANGE;
    else
    {
        if (fnDestoryElement != NULL)
            fnDestoryElement(&(pijan->ijan_pjaeElement));

        /* adjust the list */
        if (pijanPrevious != NULL)
            pijanPrevious->ijan_pijanNext = pijan->ijan_pijanNext;
        else
            pija->ija_pijanElements = pijan->ijan_pijanNext;

        pija->ija_u32ArraySize = pija->ija_u32ArraySize - 1;

        jf_mem_free((void **)&pijan);
    }

    return u32Ret;
}

static u32 _insertElementAt(
    internal_jf_array_t * pija, u32 u32Index, jf_array_element_t * pjae)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_array_node_t *pijanPrevious, *pijan, *pijanNew;
    u32 u32Pos;

    u32Pos = 0;
    pijanPrevious = NULL;
    pijan = pija->ija_pijanElements;
    while ((u32Pos < u32Index) && (pijan != NULL))
    {
        pijanPrevious = pijan;
        pijan = pijan->ijan_pijanNext;
        u32Pos = u32Pos + 1;
    }

    u32Ret = jf_mem_alloc((void **)&pijanNew, sizeof(internal_jf_array_node_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pijanNew->ijan_pjaeElement = pjae;

        /* adjust the list */
        if (pijanPrevious == NULL)
            pija->ija_pijanElements = pijanNew;
        else
            pijanPrevious->ijan_pijanNext = pijanNew;

        pijanNew->ijan_pijanNext = pijan;

        pija->ija_u32ArraySize = pija->ija_u32ArraySize + 1;
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 jf_array_create(jf_array_t ** ppja)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_array_t * pija;

    u32Ret = jf_mem_alloc((void **)&pija, sizeof(internal_jf_array_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pija->ija_u32ArraySize = 0;
        pija->ija_pijanElements = NULL;

        *ppja = (jf_array_t *) pija;
    }

    return u32Ret;
}

u32 jf_array_destroy(jf_array_t ** ppja)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_array_t * pija;

    assert((ppja != NULL) && (*ppja != NULL));

    pija = (internal_jf_array_t *) *ppja;

    u32Ret = jf_array_removeAllElements(pija);

    jf_mem_free(ppja);

    return u32Ret;
}

u32 jf_array_getSize(jf_array_t * pja)
{
    internal_jf_array_t *pija;

    assert(pja != NULL);

    pija = (internal_jf_array_t *) pja;

    return pija->ija_u32ArraySize;
}

u32 jf_array_getElementAt(
    jf_array_t * pja, u32 u32Index, jf_array_element_t ** ppjae)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_array_t *pija;

    assert(pja != NULL);

    pija = (internal_jf_array_t *) pja;
    if (u32Index >= pija->ija_u32ArraySize)
        u32Ret = JF_ERR_OUT_OF_RANGE;
    else
        u32Ret = _getElementAt(pija, u32Index, ppjae);

    return u32Ret;
}

u32 jf_array_removeElementAt(jf_array_t * pja, u32 u32Index)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_array_t *pija;

    assert(pja != NULL);

    pija = (internal_jf_array_t *) pja;
    if (u32Index >= pija->ija_u32ArraySize)
        u32Ret = JF_ERR_OUT_OF_RANGE;
    else
    {
        u32Ret = _removeElementAt(pija, u32Index, NULL);
    }

    return u32Ret;
}

u32 jf_array_removeElement(jf_array_t * pja, jf_array_element_t * pjae)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_array_t * pija;
    internal_jf_array_node_t * pijan, * pijanPrevious;
    u32 u32Index, u32Size;
    boolean_t bFound;

    assert(pja != NULL);

    pija = (internal_jf_array_t *) pja;
    u32Index = 0;
    u32Size = pija->ija_u32ArraySize;
    bFound = FALSE;
    pijan = pija->ija_pijanElements;
    pijanPrevious = NULL;
    while ((u32Index < u32Size) && (bFound == FALSE))
    {
        if (pijan->ijan_pjaeElement == pjae)
        {
            bFound = TRUE;
        }
        else
        {
            u32Index = u32Index + 1;
            pijanPrevious = pijan;
            pijan = pijan->ijan_pijanNext;
        }
    }

    if (bFound == FALSE)
        u32Ret = JF_ERR_NOT_FOUND;
    else
    {
        if (pijanPrevious != NULL)
            pijanPrevious->ijan_pijanNext = pijan->ijan_pijanNext;
        else
            pija->ija_pijanElements = pijan->ijan_pijanNext;
        pija->ija_u32ArraySize = u32Size - 1;
        jf_mem_free((void **)&pijan);
    }

    return u32Ret;
}

u32 jf_array_removeAllElements(jf_array_t * pja)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index, u32Size, u32Ret2;
    internal_jf_array_t *pija;

    assert(pja != NULL);

    pija = (internal_jf_array_t *) pja;
    u32Index = 0;
    u32Size = pija->ija_u32ArraySize;
    while (u32Index < u32Size)
    {
        u32Ret2 = _removeElementAt(pija, 0, NULL);
        if (u32Ret2 != JF_ERR_NO_ERROR)
            u32Ret = u32Ret2;
        u32Index = u32Index + 1;
    }

    return u32Ret;
}

u32 jf_array_insertElementAt(
    jf_array_t * pja, u32 u32Index, jf_array_element_t * pjae)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_array_t *pija;

    assert(pja != NULL);

    pija = (internal_jf_array_t *) pja;
    u32Ret = _insertElementAt(pija, u32Index, pjae);

    return u32Ret;
}

u32 jf_array_appendElementTo(jf_array_t * pja, jf_array_element_t * pjae)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_array_t *pija;

    assert(pja != NULL);

    pija = (internal_jf_array_t *) pja;
    u32Ret = _insertElementAt(pija, pija->ija_u32ArraySize, pjae);

    return u32Ret;
}

u32 jf_array_destroyAllElements(
    jf_array_t * pja, jf_array_fnDestroyElement_t fnDestroyElement)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index, u32Size;
    internal_jf_array_t * pija;

    assert(pja != NULL);

    pija = (internal_jf_array_t *) pja;
    u32Index = 0;
    u32Size = pija->ija_u32ArraySize;
    while (u32Index < u32Size)
    {
        _removeElementAt(pija, 0, fnDestroyElement);

        u32Index ++;
    }

    return u32Ret;
}

u32 jf_array_destroyArrayAndElements(
    jf_array_t ** ppja, jf_array_fnDestroyElement_t fnDestroyElement)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_jf_array_t * pija = (internal_jf_array_t *)*ppja;

    assert((ppja != NULL) && (*ppja != NULL));

    u32Ret = jf_array_destroyAllElements(pija, fnDestroyElement);

    jf_mem_free(ppja);

    return u32Ret;
}

u32 jf_array_findElement(
    jf_array_t * pja, jf_array_element_t ** ppElement,
    jf_array_fnFindElement_t fnFindElement, void * pKey)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    u32 u32Index, u32Size;
    internal_jf_array_t * pija;
    internal_jf_array_node_t * pijan;

    assert(pja != NULL);

    pija = (internal_jf_array_t *) pja;
    pijan = pija->ija_pijanElements;
    u32Index = 0;
    u32Size = pija->ija_u32ArraySize;

    while (u32Index < u32Size)
    {
        if (fnFindElement(pijan->ijan_pjaeElement, pKey))
        {
            *ppElement = pijan->ijan_pjaeElement;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        pijan = pijan->ijan_pijanNext;

        u32Index ++;
    }

    return u32Ret;
}

u32 jf_array_traverse(
    jf_array_t * pja, jf_array_fnOpOnElement_t fnOpOnElement, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index, u32Size;
    internal_jf_array_t * pija;
    internal_jf_array_node_t * pijan;

    assert(pja != NULL);

    pija = (internal_jf_array_t *) pja;
    pijan = pija->ija_pijanElements;
    u32Index = 0;
    u32Size = pija->ija_u32ArraySize;

    while (u32Index < u32Size)
    {
        fnOpOnElement(pijan->ijan_pjaeElement, pData);

        pijan = pijan->ijan_pijanNext;

        u32Index ++;
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/

