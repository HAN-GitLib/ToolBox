#include <ctype.h>

#include "HAN_Data.h"

/******************** 链表 ********************/
typedef struct tagHANLISTNODE {
    void* pData;
    struct tagHANLISTNODE* pPrev;
    struct tagHANLISTNODE* pNext;
} HANLISTNODE, * HANPLISTNODE;
typedef const HANLISTNODE* HANPCLISTNODE;

HANERROR HANListAppend(HANPLIST pList, void* pData, HANPCALLOCDATA allocNode)
{
    HANERROR eRet = RET_OK;
    HANPLISTNODE pHeadNode = NULL;
    HANPLISTNODE pLastNode = NULL;
    HANPLISTNODE pNewNode = NULL;

    /* 申请内存 */
    if (RET_OK == eRet)
    {
        pNewNode = allocNode->AllocOperation(NULL, sizeof(HANLISTNODE), allocNode->pParam);
        if (NULL == pNewNode) { eRet = NOT_ENOUGH_MEMORY; }
    }
    /* 处理节点 */
    if (RET_OK == eRet)
    {
        /* 头节点为 NULL，长度也必为 0 */
        if (NULL == pList->pNode)
        {
            pList->pNode = pNewNode;
            pLastNode = pNewNode;
        }
        else
        {
            pHeadNode = pList->pNode;
            pLastNode = pHeadNode->pPrev;
            pHeadNode->pPrev = pNewNode;
            pLastNode->pNext = pNewNode;
        }
        pNewNode->pPrev = pLastNode;
        pNewNode->pNext = NULL;
        pNewNode->pData = pData;
        pList->nLen++;
    }

    return eRet;
}
HANERROR HANListInsert(HANPLIST pList, size_t nIndex, void* pData, HANPCALLOCDATA allocNode)
{
    HANERROR eRet = RET_OK;
    size_t nTarIndex = nIndex;
    HANPLISTNODE pPrevNode = NULL;
    HANPLISTNODE pNextNode = NULL;
    HANPLISTNODE pNewNode = NULL;
    
    /* 判断序号是否合法 */
    if ((pList->nLen <= nTarIndex) && (HAN_DATA_INDEX_END != nTarIndex))
    {
        if (pList->nLen == nTarIndex) { nTarIndex = HAN_DATA_INDEX_END; }
        else { eRet = INDEX_OUT_OF_RANGE; }
    }
    /* 处理尾部追加节点 */
    if (HAN_DATA_INDEX_END == nTarIndex)
    {
        eRet = HANListAppend(pList, pData, allocNode);
    }
    else
    {
        /* 申请内存 */
        if (RET_OK == eRet)
        {
            pNewNode = allocNode->AllocOperation(NULL, sizeof(HANLISTNODE), allocNode->pParam);
            if (NULL == pNewNode) { eRet = NOT_ENOUGH_MEMORY; }
        }
        /* 处理节点 */
        if (RET_OK == eRet)
        {
            pNextNode = pList->pNode;
            /* 头节点为 NULL，长度也必为 0，已经在 Append 里处理好了 */
            /* 头部插入新节点 */
            if (0 == nTarIndex)
            {
                pPrevNode = pNextNode->pPrev;
                pList->pNode = pNewNode;

                pNextNode->pPrev = pNewNode;
                pPrevNode->pNext = NULL;
            }
            /* 任意位置插入新的节点 */
            else
            {
                for (size_t iLoop = 0; iLoop < nTarIndex; iLoop++)
                {
                    pNextNode = pNextNode->pNext;
                }
                pPrevNode = pNextNode->pPrev;

                pNextNode->pPrev = pNewNode;
                pPrevNode->pNext = pNewNode;
            }

            pNewNode->pPrev = pPrevNode;
            pNewNode->pNext = pNextNode;
            pNewNode->pData = pData;
            pList->nLen++;
        }
    }

    return eRet;
}

void* HANListGetData(HANPCLIST pList, size_t nIndex)
{
    void* pRet = NULL;
    size_t nTarIndex = nIndex;
    HANPLISTNODE pNode = pList->pNode;

    if (HAN_DATA_INDEX_END == nTarIndex)
    {
        pNode = pNode->pPrev;
    }
    else if (nTarIndex < pList->nLen)
    {
        for (size_t iLoop = 0; iLoop < nTarIndex; iLoop++)
        {
            pNode = pNode->pNext;
        }
    }
    else
    {
        pNode = NULL;
    }

    if (NULL != pNode)
    {
        pRet = pNode->pData;
    }

    return pRet;
}
void* HANListForData(HANPLIST pList)
{
    void* pRet = NULL;
    HANPLISTNODE pNode = pList->pFor;

    if (NULL == pNode) { pNode = pList->pNode; }
    else { pNode = pNode->pNext; }
    if (NULL != pNode)
    {
        pRet = pNode->pData;
    }
    pList->pFor = pNode;

    return pRet;
}

HANERROR HANListDelete(HANPLIST pList, size_t nIndex, HANPCFREEDATA freeNode, HANPCFREEDATA freeData)
{
    HANERROR eRet = RET_OK;
    size_t nTarIndex = nIndex;
    HANPLISTNODE pTarNode = NULL;
    HANPLISTNODE pPrevNode = NULL;
    HANPLISTNODE pNextNode = NULL;
    HANPLISTNODE pHeadNode = NULL;

    if (HAN_DATA_INDEX_END == nTarIndex)
    {
        nTarIndex = pList->nLen - 1;
    }
    if (pList->nLen <= nTarIndex)
    {
        eRet = INDEX_OUT_OF_RANGE;
    }

    if (RET_OK == eRet)
    {
        pHeadNode = pList->pNode;
        pTarNode = pHeadNode;
        for (size_t iLoop = 0; iLoop < nTarIndex; iLoop++)
        {
            pTarNode = pTarNode->pNext;
        }
        pPrevNode = pTarNode->pPrev;
        pNextNode = pTarNode->pNext;

        if (NULL != pPrevNode) { pPrevNode->pNext = pNextNode; }
        if (NULL != pNextNode) { pNextNode->pPrev = pPrevNode; }
        if (0 == nTarIndex)
        {
            pList->pNode = pNextNode;
            if (NULL != pPrevNode) { pPrevNode->pNext = NULL; }
        }
        if ((pList->nLen - 1) == nTarIndex)
        {
            pHeadNode->pPrev = pPrevNode;
        }
        if (NULL != pTarNode)
        {
            if (NULL != freeData) { freeData->FreeOperation(pTarNode->pData, freeData->pParam); }
            if (NULL != freeNode) { freeNode->FreeOperation(pTarNode, freeNode->pParam); }
        }
        pList->nLen--;
    }

    return eRet;
}
void HANListDeleteAll(HANPLIST pList, HANPCFREEDATA freeNode, HANPCFREEDATA freeData)
{
    HANPLISTNODE pNode = pList->pNode;
    HANPLISTNODE pNext;

    while (NULL != pNode)
    {
        pNext = pNode->pNext;
        if (NULL != freeData) { freeData->FreeOperation(pNode->pData, freeData->pParam); }
        if (NULL != freeNode) { freeNode->FreeOperation(pNode, freeNode->pParam); }
        pNode = pNext;
    }

    pList->nLen = 0;
    pList->pNode = NULL;
    pList->pFor = NULL;
}

void HANListPrint(HANPLIST pList, void (*PrintOperation)(void* pData))
{
    HANPLISTNODE pNode = pList->pNode;

    while (NULL != pNode)
    {
        PrintOperation(pNode->pData);
        pNode = pNode->pNext;
    }
}


/******************** 二叉树 ********************/
typedef struct tagHANTREENODE {
    void*                   pData;
    struct tagHANTREENODE*  pParent;
    struct tagHANTREENODE*  pLeft;
    struct tagHANTREENODE*  pRight;
} HANTREENODE, * HANPTREENODE;
typedef HANTREENODE* HANPCTREENODE;

typedef struct tagHANTREEINSERTNODE {
    void*                   pData;
    HANDATACMP              nCmpRule;
    HANTREECMPOPERATION     CompareOperation;
    HANPCALLOCDATA          allocNode;
} HANTREEINSERTNODE, * HANPTREEINSERTNODE;
typedef const HANTREEINSERTNODE* HANPCTREEINSERTNODE;

static HANPTREENODE HANTreeCreateNewNode(void* pData, HANPCALLOCDATA allocNode);
static HANERROR HANTreeInsertNode(HANPTREENODE pParentNode, HANPCTREEINSERTNODE pInsert);
static void HANTreePrintNode(HANPTREENODE pParentNode, void (*PrintOperation)(void* pData));

HANERROR HANTreeAddNewData(HANPTREE pTree, void* pData, HANTREECMPOPERATION CompareOperation, HANPCALLOCDATA allocNode)
{
    HANERROR eRet = RET_OK;
    HANPTREENODE pNewNode;
    HANPTREENODE pTarNode = pTree->pNode;
    HANDATACMP nCmpResult;
    HANTREEINSERTNODE infoInsert = {
        .pData = pData,
        .nCmpRule = pTree->nSortRule,
        .CompareOperation = CompareOperation,
        .allocNode = allocNode,
    };

    if (NULL == pTarNode)
    {
        pNewNode = HANTreeCreateNewNode(pData, allocNode);
        if (NULL == pNewNode) { eRet = NOT_ENOUGH_MEMORY; }
        else
        {
            pTree->pNode = pNewNode;
            pTree->nLen++;
        }
    }
    else
    {
        nCmpResult = CompareOperation(pData, pTarNode->pData);
        if ((HAN_DATA_CMP_LSB != nCmpResult) && (HAN_DATA_CMP_MSB != nCmpResult)) { eRet = TREE_CMP_EQUAL; }
        else
        {
            eRet = HANTreeInsertNode(pTarNode, &infoInsert);
            if (RET_OK == eRet) { pTree->nLen++; }
        }
    }

    return eRet;
}

void HANTreePrint(HANPTREE pTree, void (*PrintOperation)(void* pData))
{
    if (NULL != pTree->pNode) { HANTreePrintNode(pTree->pNode, PrintOperation); }
}

static HANPTREENODE HANTreeCreateNewNode(void* pData, HANPCALLOCDATA allocNode)
{
    HANPTREENODE pNewNode = allocNode->AllocOperation(NULL, sizeof(HANTREENODE), allocNode->pParam);

    if (NULL != pNewNode)
    {
        pNewNode->pData = pData;
        pNewNode->pParent = NULL;
        pNewNode->pLeft = NULL;
        pNewNode->pRight = NULL;
    }

    return pNewNode;
}
static HANERROR HANTreeInsertNode(HANPTREENODE pParentNode, HANPCTREEINSERTNODE pInsert)
{
    HANERROR eRet = RET_OK;
    HANDATACMP nCmpResult = pInsert->CompareOperation(pInsert->pData, pParentNode->pData);
    HANPTREENODE pNewNode;

    if ((HAN_DATA_CMP_LSB != nCmpResult) && (HAN_DATA_CMP_MSB != nCmpResult)) { eRet = TREE_CMP_EQUAL; }
    else
    {
        if (nCmpResult == pInsert->nCmpRule)
        {
            if (NULL == pParentNode->pLeft)
            {
                pNewNode = HANTreeCreateNewNode(pInsert->pData, pInsert->allocNode);
                if (NULL == pNewNode) { eRet = NOT_ENOUGH_MEMORY; }
                else
                {
                    pParentNode->pLeft = pNewNode;
                    pNewNode->pParent = pParentNode;
                }
            }
            else { eRet = HANTreeInsertNode(pParentNode->pLeft, pInsert); }
        }
        else
        {
            if (NULL == pParentNode->pRight)
            {
                pNewNode = HANTreeCreateNewNode(pInsert->pData, pInsert->allocNode);
                if (NULL == pNewNode) { eRet = NOT_ENOUGH_MEMORY; }
                else
                {
                    pParentNode->pRight = pNewNode;
                    pNewNode->pParent = pParentNode;
                }
            }
            else { eRet = HANTreeInsertNode(pParentNode->pRight, pInsert); }
        }
    }

    return eRet;
}
static void HANTreePrintNode(HANPTREENODE pParentNode, void (*PrintOperation)(void* pData))
{
    if (NULL != pParentNode->pLeft) { HANTreePrintNode(pParentNode->pLeft, PrintOperation); }
    PrintOperation(pParentNode->pData);
    if (NULL != pParentNode->pRight) { HANTreePrintNode(pParentNode->pRight, PrintOperation); }
}
