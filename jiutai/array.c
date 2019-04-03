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
#include "olbasic.h"
#include "errcode.h"
#include "array.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */

/* a single list */
typedef struct internal_basic_array_node_t
{
    basic_array_element_t * iban_pbaeElement;
    struct internal_basic_array_node_t * iban_pibanNext;
} internal_basic_array_node_t;

typedef struct
{
    u32 iba_u32ArraySize;
    internal_basic_array_node_t * iba_pibanElements;
} internal_basic_array_t;

/* --- private routine section ------------------------------------------------ */

static u32 _getElementAt(internal_basic_array_t * piba, u32 u32Index,
	      basic_array_element_t ** ppbae)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_basic_array_node_t *piban;
    u32 u32Pos;

    u32Pos = 0;
    piban = piba->iba_pibanElements;
    while ((u32Pos < u32Index) && (piban != NULL))
    {
        piban = piban->iban_pibanNext;
        u32Pos = u32Pos + 1;
    }

    if ((u32Pos == u32Index) && (piban != NULL))
        *ppbae = piban->iban_pbaeElement;
    else
        u32Ret = JF_ERR_OUT_OF_RANGE;

    return u32Ret;
}

static u32 _removeElementAt(internal_basic_array_t * piba, u32 u32Index,
    fnDestroyBasicArrayElement_t fnDestoryElement)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_basic_array_node_t * pibanPrevious, * piban;
    u32 u32Pos;

    u32Pos = 0;
    pibanPrevious = NULL;
    piban = piba->iba_pibanElements;
    while ((u32Pos < u32Index) && (piban != NULL))
    {
        pibanPrevious = piban;
        piban = piban->iban_pibanNext;
        u32Pos = u32Pos + 1;
    }

    if ((u32Pos != u32Index) || (piban == NULL))
        u32Ret = JF_ERR_OUT_OF_RANGE;
    else
    {
        if (fnDestoryElement != NULL)
            fnDestoryElement(&(piban->iban_pbaeElement));

        /* adjust the list */
        if (pibanPrevious != NULL)
            pibanPrevious->iban_pibanNext = piban->iban_pibanNext;
        else
            piba->iba_pibanElements = piban->iban_pibanNext;

        piba->iba_u32ArraySize = piba->iba_u32ArraySize - 1;

        xfree((void **)&piban);
    }

    return u32Ret;
}

static u32 _insertElementAt(internal_basic_array_t * piba, u32 u32Index,
		 basic_array_element_t * pbae)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_basic_array_node_t *pibanPrevious, *piban, *pibanNew;
    u32 u32Pos;

    u32Pos = 0;
    pibanPrevious = NULL;
    piban = piba->iba_pibanElements;
    while ((u32Pos < u32Index) && (piban != NULL))
    {
        pibanPrevious = piban;
        piban = piban->iban_pibanNext;
        u32Pos = u32Pos + 1;
    }

    u32Ret = xmalloc((void **)&pibanNew, sizeof(internal_basic_array_node_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pibanNew->iban_pbaeElement = pbae;

        /* adjust the list */
        if (pibanPrevious == NULL)
            piba->iba_pibanElements = pibanNew;
        else
            pibanPrevious->iban_pibanNext = pibanNew;

        pibanNew->iban_pibanNext = piban;

        piba->iba_u32ArraySize = piba->iba_u32ArraySize + 1;
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 createBasicArray(basic_array_t ** ppba)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_basic_array_t * piba;

    u32Ret = xmalloc((void **)&piba, sizeof(internal_basic_array_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        piba->iba_u32ArraySize = 0;
        piba->iba_pibanElements = NULL;

        *ppba = (basic_array_t *) piba;
    }

    return u32Ret;
}

u32 destroyBasicArray(basic_array_t ** ppba)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_basic_array_t * piba;

    assert((ppba != NULL) && (*ppba != NULL));

    piba = (internal_basic_array_t *) *ppba;

    u32Ret = removeAllBasicArrayElements(piba);

    xfree(ppba);

    return u32Ret;
}

u32 getBasicArraySize(basic_array_t * pba)
{
    internal_basic_array_t *piba;

    assert(pba != NULL);

    piba = (internal_basic_array_t *) pba;

    return piba->iba_u32ArraySize;
}

u32 getAtBasicArray(basic_array_t * pba, u32 u32Index,
		basic_array_element_t ** ppbae)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_basic_array_t *piba;

    assert(pba != NULL);

    piba = (internal_basic_array_t *) pba;
    if (u32Index >= piba->iba_u32ArraySize)
        u32Ret = JF_ERR_OUT_OF_RANGE;
    else
        u32Ret = _getElementAt(piba, u32Index, ppbae);

    return u32Ret;
}

u32 removeAtBasicArray(basic_array_t * pba, u32 u32Index)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_basic_array_t *piba;

    assert(pba != NULL);

    piba = (internal_basic_array_t *) pba;
    if (u32Index >= piba->iba_u32ArraySize)
        u32Ret = JF_ERR_OUT_OF_RANGE;
    else
    {
        u32Ret = _removeElementAt(piba, u32Index, NULL);
    }

    return u32Ret;
}

u32 removeBasicArrayElement(basic_array_t * pba, basic_array_element_t * pbae)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_basic_array_t * piba;
    internal_basic_array_node_t * piban, * pibanPrevious;
    u32 u32Index, u32Size;
    boolean_t bFound;

    assert(pba != NULL);

    piba = (internal_basic_array_t *) pba;
    u32Index = 0;
    u32Size = piba->iba_u32ArraySize;
    bFound = FALSE;
    piban = piba->iba_pibanElements;
    pibanPrevious = NULL;
    while ((u32Index < u32Size) && (bFound == FALSE))
    {
        if (piban->iban_pbaeElement == pbae)
        {
            bFound = TRUE;
        }
        else
        {
            u32Index = u32Index + 1;
            pibanPrevious = piban;
            piban = piban->iban_pibanNext;
        }
    }

    if (bFound == FALSE)
        u32Ret = JF_ERR_NOT_FOUND;
    else
    {
        if (pibanPrevious != NULL)
            pibanPrevious->iban_pibanNext = piban->iban_pibanNext;
        else
            piba->iba_pibanElements = piban->iban_pibanNext;
        piba->iba_u32ArraySize = u32Size - 1;
        xfree((void **)&piban);
    }

    return u32Ret;
}

u32 removeAllBasicArrayElements(basic_array_t * pba)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index, u32Size, u32Ret2;
    internal_basic_array_t *piba;

    assert(pba != NULL);

    piba = (internal_basic_array_t *) pba;
    u32Index = 0;
    u32Size = piba->iba_u32ArraySize;
    while (u32Index < u32Size)
    {
        u32Ret2 = _removeElementAt(piba, 0, NULL);
        if (u32Ret2 != JF_ERR_NO_ERROR)
            u32Ret = u32Ret2;
        u32Index = u32Index + 1;
    }

    return u32Ret;
}

u32 insertAtBasicArray(basic_array_t * pba, u32 u32Index,
		   basic_array_element_t * pbae)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_basic_array_t *piba;

    assert(pba != NULL);

    piba = (internal_basic_array_t *) pba;
    u32Ret = _insertElementAt(piba, u32Index, pbae);

    return u32Ret;
}

u32 appendToBasicArray(basic_array_t * pba, basic_array_element_t * pbae)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_basic_array_t *piba;

    assert(pba != NULL);

    piba = (internal_basic_array_t *) pba;
    u32Ret = _insertElementAt(piba, piba->iba_u32ArraySize, pbae);

    return u32Ret;
}

u32 destroyAllBasicArrayElements(basic_array_t * pba,
    fnDestroyBasicArrayElement_t fnDestroyElement)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index, u32Size;
    internal_basic_array_t * piba;

    assert(pba != NULL);

    piba = (internal_basic_array_t *) pba;
    u32Index = 0;
    u32Size = piba->iba_u32ArraySize;
    while (u32Index < u32Size)
    {
        _removeElementAt(piba, 0, fnDestroyElement);

        u32Index ++;
    }

    return u32Ret;
}

u32 destroyBasicArrayAndElements(basic_array_t ** ppba,
    fnDestroyBasicArrayElement_t fnDestroyElement)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_basic_array_t * piba = (internal_basic_array_t *)*ppba;

    assert((ppba != NULL) && (*ppba != NULL));

    u32Ret = destroyAllBasicArrayElements(piba, fnDestroyElement);

    xfree(ppba);

    return u32Ret;
}

u32 findBasicArrayElement(basic_array_t * pba,
    basic_array_element_t ** ppElement,
    fnFindBasicArrayElement_t fnFindElement, void * pKey)
{
    u32 u32Ret = JF_ERR_NOT_FOUND;
    u32 u32Index, u32Size;
    internal_basic_array_t * piba;
    internal_basic_array_node_t * piban;

    assert(pba != NULL);

    piba = (internal_basic_array_t *) pba;
    piban = piba->iba_pibanElements;
    u32Index = 0;
    u32Size = piba->iba_u32ArraySize;

    while (u32Index < u32Size)
    {
        if (fnFindElement(piban->iban_pbaeElement, pKey))
        {
            *ppElement = piban->iban_pbaeElement;
            u32Ret = JF_ERR_NO_ERROR;
            break;
        }

        piban = piban->iban_pibanNext;

        u32Index ++;
    }

    return u32Ret;
}

u32 traverseBasicArray(basic_array_t * pba,
    fnOpOnBasicArrayElement_t fnOpOnElement, void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index, u32Size;
    internal_basic_array_t * piba;
    internal_basic_array_node_t * piban;

    assert(pba != NULL);

    piba = (internal_basic_array_t *) pba;
    piban = piba->iba_pibanElements;
    u32Index = 0;
    u32Size = piba->iba_u32ArraySize;

    while (u32Index < u32Size)
    {
        fnOpOnElement(piban->iban_pbaeElement, pData);

        piban = piban->iban_pibanNext;

        u32Index ++;
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/

