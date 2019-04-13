/**
 *  @file huffman.c
 *
 *  @brief huffman encoding and decoding implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_encode.h"
#include "huffman.h"
#include "jf_err.h"
#include "jf_mem.h"
#include "jf_bitarray.h"

/* --- private data/data structure section ------------------------------------------------------ */
#define HUFFMAN_NONE      (-1)

typedef struct huffman_node
{
    jf_encode_huffman_code_t * hn_pjehcCode;
    u64 hn_u64Freq;
    /** if TRUE, already handled or no need to handle */
    boolean_t hn_bIgnore;
    /** depth in tree (root is 0) */
    olint_t hn_nLevel;

    /** pointer to children and parent. Parent is only useful if non-recursive
     *  methods are used to search the huffman tree.
     */
    struct huffman_node * hn_phnLeft, * hn_phnRight, * hn_phnParent;
} huffman_node_t;

typedef struct
{
    huffman_node_t * hnp_phnNodes;
    u32 hnp_u32MaxNode;
    u32 hnp_u32NumOfAlloc;
} huffman_node_pool_t;

/* --- private routine section ------------------------------------------------------------------ */

static u32 _destroyHuffmanNodePool(huffman_node_pool_t ** ppPool)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    huffman_node_pool_t * phnp = *ppPool;

    if (phnp->hnp_phnNodes != NULL)
        jf_mem_free((void **)&(phnp->hnp_phnNodes));

    jf_mem_free((void **)ppPool);

    return u32Ret;
}

static u32 _createHuffmanNodePool(
    huffman_node_pool_t ** ppPool, u16 u16NumOfCode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    huffman_node_pool_t * phnp = NULL;

    u32Ret = jf_mem_calloc((void **)&phnp, sizeof(huffman_node_pool_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        phnp->hnp_u32MaxNode = 2 * u16NumOfCode;

        u32Ret = jf_mem_calloc(
            (void **)&(phnp->hnp_phnNodes),
            sizeof(huffman_node_t) * phnp->hnp_u32MaxNode);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {

    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppPool = phnp;
    else if (phnp != NULL)
        _destroyHuffmanNodePool(&phnp);

    return u32Ret;
}

static huffman_node_t * _getHuffmanNode(huffman_node_pool_t * phnp)
{
    huffman_node_t * phn = NULL;

    if (phnp->hnp_u32NumOfAlloc < phnp->hnp_u32MaxNode)
    {
        phn = &(phnp->hnp_phnNodes[phnp->hnp_u32NumOfAlloc]);

        phnp->hnp_u32NumOfAlloc ++;
    }

    assert(phn != NULL);
    
    return phn;
}

/** Allocates and initializes memory for a composite node (tree entry for
 *  multiple characters) in a huffman tree. The number of occurrences for a
 *  composite is the sum of occurrences of its children.
 *
 *  @param pPool [in] pointer to the huffman node pool
 *  @param left [in] left child in tree
 *  @param right [in] right child in tree
 *  @param ppNode [out] the composite note
 *
 *  @return the error code
 */
static u32 _getHuffmanCompositeNode(
    huffman_node_pool_t * pPool, huffman_node_t * left, huffman_node_t * right,
    huffman_node_t ** ppNode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    huffman_node_t * phn = NULL;

    phn = _getHuffmanNode(pPool);

    phn->hn_bIgnore = FALSE;
    /* sum of children */
    phn->hn_u64Freq = left->hn_u64Freq + right->hn_u64Freq;
    phn->hn_nLevel = MAX(left->hn_nLevel, right->hn_nLevel) + 1;

    /* attach children */
    phn->hn_phnLeft = left;
    phn->hn_phnLeft->hn_phnParent = phn;
    phn->hn_phnRight = right;
    phn->hn_phnRight->hn_phnParent = phn;
    phn->hn_phnParent = NULL;

    *ppNode = phn;

    return u32Ret;
}

/** Searches an array of huffman_node_t to find the active (hn_bIgnore == FALSE)
 *  element with the smallest frequency count. In order to keep the tree
 *  shallow, if two nodes have the same count, the node with the lower level
 *  selected.
 *
 *  @param ppNode [in] pointer to the note array to be searched
 *  @param u16NumOfCode [in] number of elements in the array
 *
 *  @return the index of the active element with the smallest count.
 *  @return HUFFMAN_NONE if no minimum is found.
 */
static olint_t _findMinimumFreq(huffman_node_t ** ppNode, u16 u16NumOfCode)
{
    olint_t i;                          /* array index */
    olint_t currentIndex = HUFFMAN_NONE;/* index with lowest count seen so far*/
    u64 currentCount = U64_MAX;         /* lowest count seen so far */
    olint_t currentLevel = OLINT_MAX;   /* level of lowest count seen so far */

    /* sequentially search array */
    for (i = 0; i < u16NumOfCode; i++)
    {
        /* check for lowest count (or equally as low, but not as deep) */
        if ((ppNode[i] != NULL) && (! ppNode[i]->hn_bIgnore) &&
            ((ppNode[i]->hn_u64Freq < currentCount) ||
             (ppNode[i]->hn_u64Freq == currentCount &&
              ppNode[i]->hn_nLevel < currentLevel)))
        {
            currentIndex = i;
            currentCount = ppNode[i]->hn_u64Freq;
            currentLevel = ppNode[i]->hn_nLevel;
        }
    }

    return currentIndex;
}

/** Builds a huffman tree from an array of huffman_node_t.
 *
 *  @param ppNode [in/out] array of the huffman nodes
 *  @param pPool [in] the huffman node pool
 *  @param u16NumOfCode [in] number of code
 *  @param ppRoot [out] the root of the huffman tree
 *
 *  @return the error code
 */
static u32 _buildHuffmanTree(
    huffman_node_t ** ppNode,
    huffman_node_pool_t * pPool, u16 u16NumOfCode, huffman_node_t ** ppRoot)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t min1, min2;

    /* keep looking until no more nodes can be found */
    for ( ; u32Ret == JF_ERR_NO_ERROR; )
    {
        /* find node with lowest count */
        min1 = _findMinimumFreq(ppNode, u16NumOfCode);
        if (min1 == HUFFMAN_NONE)
        {
            /* no more nodes to combine */
            break;
        }
        /* remove from consideration */
        ppNode[min1]->hn_bIgnore = TRUE;

        /* find node with second lowest count */
        min2 = _findMinimumFreq(ppNode, u16NumOfCode);
        if (min2 == HUFFMAN_NONE)
        {
            /* no more nodes to combine */
            break;
        }
        /* remove from consideration */
        ppNode[min2]->hn_bIgnore = TRUE;

        /* combine nodes into a tree */
        u32Ret = _getHuffmanCompositeNode(
            pPool, ppNode[min1], ppNode[min2], &ppNode[min1]);

        ppNode[min2] = NULL;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (min1 == HUFFMAN_NONE)
            u32Ret = JF_ERR_FAIL_BUILD_HUFFMAN_TREE;
        else
            *ppRoot = ppNode[min1];
    }
    
    return u32Ret;
}

static u32 _initHuffmanTreeLeaf(
    huffman_node_pool_t * pPool,
    huffman_node_t ** ppNode, jf_encode_huffman_code_t * pjehc, u16 u16NumOfCode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u16 u16Index;

    for (u16Index = 0; u16Index < u16NumOfCode; u16Index ++)
    {
        pjehc[u16Index].jehc_u16CodeLen = 0;

        if (pjehc[u16Index].jehc_u32Freq != 0)
        {
            ppNode[u16Index] = _getHuffmanNode(pPool);

            ppNode[u16Index]->hn_pjehcCode = &(pjehc[u16Index]);
            ppNode[u16Index]->hn_u64Freq = (u64)pjehc[u16Index].jehc_u32Freq;
        }
    }

    return u32Ret;
}

/** Uses a huffman tree to build a list of codes and their length for each
 *  encoded symbol.
 *
 *  @param pRoot [in] the pointer to root of huffman tree
 *  @param u16NumOfCode [in] number of code
 *
 *  @return the error code
 */
static u32 _genHuffmanCode(huffman_node_t * pRoot, u16 u16NumOfCode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_bitarray_t code[JF_ENCODE_MAX_HUFFMAN_CODE_LEN];
    u8 depth = 0;

    JF_BITARRAY_INIT(code);

    for( ; u32Ret == JF_ERR_NO_ERROR; )
    {
        /* follow this branch all the way left */
        while (pRoot->hn_phnLeft != NULL)
        {
            JF_BITARRAY_LSHIFT(code, 1);
            pRoot = pRoot->hn_phnLeft;
            depth ++;
        }

        if (pRoot->hn_pjehcCode != NULL)
        {
            /* enter results in list */
            pRoot->hn_pjehcCode->jehc_u16CodeLen = depth;
            JF_BITARRAY_COPY(pRoot->hn_pjehcCode->jehc_jbCode, code);
            JF_BITARRAY_LSHIFT(
                pRoot->hn_pjehcCode->jehc_jbCode, JF_BITARRAY_BITS(code) - depth);
        }

        while (pRoot->hn_phnParent != NULL)
        {
            if (pRoot != pRoot->hn_phnParent->hn_phnRight)
            {
                /* try the parent's right */
                jf_bitarray_setBit(code, JF_BITARRAY_BITS(code) - 1);
                pRoot = pRoot->hn_phnParent->hn_phnRight;
                break;
            }
            else
            {
                /* parent's right tried, go up one level yet */
                depth --;
                JF_BITARRAY_RSHIFT(code, 1);
                pRoot = pRoot->hn_phnParent;
            }
        }

        if (pRoot->hn_phnParent == NULL)
        {
            /* we're at the top with nowhere to go */
            break;
        }
    }

    return u32Ret;
}

/** Accepts a list of symbol sorted by their code lengths, and assigns a
 *  canonical huffman code to each symbol.
 *
 *  @param ppjehc [in] sorted list of symbols to have code values assigned
 *  @param u16NumOfCode [in] number of elements in the array
 *
 *  @return the error code
 */
static u32 _assignCanonicalCodes(jf_encode_huffman_code_t ** ppjehc, u16 u16NumOfCode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t i;
    u8 length;
    jf_bitarray_t code[JF_ENCODE_MAX_HUFFMAN_CODE_LEN];

    /* assign the new codes */
    JF_BITARRAY_INIT(code);

    length = ppjehc[(u16NumOfCode - 1)]->jehc_u16CodeLen;

    for (i = (u16NumOfCode - 1); i >= 0; i--)
    {
        /* fail if we hit a zero len code */
        if (ppjehc[i]->jehc_u16CodeLen == 0)
        {
            break;
        }
        /* adjust code if this length is shorter than the previous */
        if (ppjehc[i]->jehc_u16CodeLen < length)
        {
            /* right shift the code */
            JF_BITARRAY_RSHIFT(code, (length - ppjehc[i]->jehc_u16CodeLen));
            length = ppjehc[i]->jehc_u16CodeLen;
        }

        /* assign left justified code */
        JF_BITARRAY_COPY(ppjehc[i]->jehc_jbCode, code);
        JF_BITARRAY_LSHIFT(ppjehc[i]->jehc_jbCode, JF_BITARRAY_BITS(code) - length);

        /* increase 1 */
        JF_BITARRAY_INCREMENT(code);
    }

    return u32Ret;
}

/** Used by qsort for sorting canonical list items by code length. In the event
 *  of equal lengths, the symbol value will be used.
 *
 *  @param item1 [in] pointer canonical list item
 *  @param item2 [in] pointer canonical list item
 *
 *  @return the result of comparision
 *  @retval 1 if item1 > item2
 *  @retval -1 if item1 < item 2
 *  @retval 0 if something went wrong (means item1 == item2)
 */
static olint_t _compareByCodeLen(const void * item1, const void * item2)
{
    jf_encode_huffman_code_t * pjehc1 = *((jf_encode_huffman_code_t **)item1);
    jf_encode_huffman_code_t * pjehc2 = *((jf_encode_huffman_code_t **)item2);

    if (pjehc1->jehc_u16CodeLen > pjehc2->jehc_u16CodeLen)
    {
        /* item1 > item2 */
        return 1;
    }
    else if (pjehc1->jehc_u16CodeLen < pjehc2->jehc_u16CodeLen)
    {
        /* item1 < item2 */
        return -1;
    }
    else
    {
        /* using symbol */
        if (pjehc1->jehc_u16Symbol > pjehc2->jehc_u16Symbol)
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }

    return 0;
}

/** Uses a huffman tree to build a list of codes and their length for each
 *  encoded symbol. This simplifies the encoding process instead of traversing
 *  a tree to in search of the code for any symbol, the code maybe found by
 *  accessing pjehc.jehc_jbCode.
 *
 *  @param pRoot [in] pointer to root of huffman tree
 *  @param pjehc [in] array for huffman code
 *  @param u16NumOfCode [in] number of elements in the array
 *
 *  @return the error code
 */
static u32 _genCanonicalHuffmanCode(
    huffman_node_t * pRoot, jf_encode_huffman_code_t * pjehc, u16 u16NumOfCode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 depth = 0;

    for( ; u32Ret == JF_ERR_NO_ERROR; )
    {
        /* follow this branch all the way left */
        while (pRoot->hn_phnLeft != NULL)
        {
            pRoot = pRoot->hn_phnLeft;
            depth ++;
        }

        if (pRoot->hn_pjehcCode != NULL)
        {
            /* handle one symbol trees */
            if (depth == 0)
                depth ++;

            /* enter results in list */
            pRoot->hn_pjehcCode->jehc_u16CodeLen = depth;
        }

        while (pRoot->hn_phnParent != NULL)
        {
            if (pRoot != pRoot->hn_phnParent->hn_phnRight)
            {
                /* try the parent's right */
                pRoot = pRoot->hn_phnParent->hn_phnRight;
                break;
            }
            else
            {
                /* parent's right tried, go up one level yet */
                depth --;
                pRoot = pRoot->hn_phnParent;
            }
        }
        if (pRoot->hn_phnParent == NULL)
        {
            /* we're at the top with nowhere to go */
            break;
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_encode_genCanonicalHuffmanCodeByCodeLen(pjehc, u16NumOfCode);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_encode_genHuffmanCode(jf_encode_huffman_code_t * pjehc, u16 u16NumOfCode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    huffman_node_t ** ppLeaf = NULL;
    huffman_node_t * pRoot = NULL;
    huffman_node_pool_t * pPool;

    u32Ret = jf_mem_calloc(
        (void **)&ppLeaf, sizeof(huffman_node_t *) * u16NumOfCode);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _createHuffmanNodePool(&pPool, u16NumOfCode);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _initHuffmanTreeLeaf(pPool, ppLeaf, pjehc, u16NumOfCode);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _buildHuffmanTree(ppLeaf, pPool, u16NumOfCode, &pRoot);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _genHuffmanCode(pRoot, u16NumOfCode);
    }

    if (ppLeaf != NULL)
        jf_mem_free((void **)ppLeaf);

    if (pPool != NULL)
        _destroyHuffmanNodePool(&pPool);

    return u32Ret;
}

u32 jf_encode_genCanonicalHuffmanCode(
    jf_encode_huffman_code_t * pjehc, u16 u16NumOfCode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    huffman_node_t ** ppLeaf = NULL;
    huffman_node_t * pRoot = NULL;
    huffman_node_pool_t * pPool;

    u32Ret = jf_mem_calloc(
        (void **)&ppLeaf, sizeof(huffman_node_t *) * u16NumOfCode);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _createHuffmanNodePool(&pPool, u16NumOfCode);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _initHuffmanTreeLeaf(pPool, ppLeaf, pjehc, u16NumOfCode);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _buildHuffmanTree(ppLeaf, pPool, u16NumOfCode, &pRoot);
    }
    
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _genCanonicalHuffmanCode(pRoot, pjehc, u16NumOfCode);
    }

    if (ppLeaf != NULL)
        jf_mem_free((void **)&ppLeaf);

    if (pPool != NULL)
        _destroyHuffmanNodePool(&pPool);

    return u32Ret;
}

u32 jf_encode_genCanonicalHuffmanCodeByCodeLen(
    jf_encode_huffman_code_t * pjehc, u16 u16NumOfCode)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_encode_huffman_code_t ** ppjehc = NULL;
    u16 u16Index;

    u32Ret = jf_mem_calloc(
        (void **)&ppjehc, sizeof(jf_encode_huffman_code_t *) * u16NumOfCode);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (u16Index = 0; u16Index < u16NumOfCode; u16Index ++)
        {
            ppjehc[u16Index] = &(pjehc[u16Index]);
        }

        /* sort by code length */
        qsort(
            ppjehc, u16NumOfCode, sizeof(jf_encode_huffman_code_t *),
            _compareByCodeLen);

        u32Ret = _assignCanonicalCodes(ppjehc, u16NumOfCode);

        jf_mem_free((void **)&ppjehc);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


