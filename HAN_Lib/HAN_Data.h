#ifndef HAN_DATA_H
#define HAN_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "HAN_def.h"

#define HAN_DATA_INDEX_END          ((size_t)(-1))

typedef enum {
    HAN_DATA_CMP_EQUAL,
    HAN_DATA_CMP_LSB,
    HAN_DATA_CMP_MSB,
} HANDATACMP;

typedef struct tagHANALLOCDATA {
    void* (*AllocOperation)(void* pOldData, size_t nNewSize, void* pParam);
    void* pParam;
} HANALLOCDATA, HANPALLOCDATA;
typedef const HANALLOCDATA* HANPCALLOCDATA;
typedef struct tagHANFREEDATA {
    void (*FreeOperation)(void* pData, void* pParam);
    void* pParam;
} HANFREEDATA, HANPFREEDATA;
typedef const HANFREEDATA* HANPCFREEDATA;


/******************** Á´±í ********************/
typedef struct tagHANLIST {
    void*   pNode;
    size_t  nLen;
    void*   pFor;
} HANLIST, * HANPLIST;
typedef const HANLIST* HANPCLIST;

static inline HANLIST HANListInit(void)
{
    HANLIST listRet = {
        .pNode = NULL,
        .nLen = 0,
        .pFor = NULL,
    };
    return listRet;
}

HANERROR HANListAppend(HANPLIST pList, void* pData, HANPCALLOCDATA allocNode);
HANERROR HANListInsert(HANPLIST pList, size_t nIndex, void* pData, HANPCALLOCDATA allocNode);

static inline size_t HANListGetLen(HANPCLIST pList) { return pList->nLen; }
void* HANListGetData(HANPCLIST pList, size_t nIndex);
static inline void HANListReForData(HANPLIST pList) { pList->pFor = NULL; }
void* HANListForData(HANPLIST pList);

HANERROR HANListDelete(HANPLIST pList, size_t nIndex, HANPCFREEDATA freeNode, HANPCFREEDATA freeData);
void HANListDeleteAll(HANPLIST pList, HANPCFREEDATA freeNode, HANPCFREEDATA freeData);

void HANListPrint(HANPLIST pList, void (*PrintOperation)(void* pData));


/******************** ¶þ²æÊ÷ ********************/
typedef struct tagHANTREE {
    void*       pNode;
    HANDATACMP  nSortRule;
    size_t      nLen;
    void*       pFor;
} HANTREE, * HANPTREE;
typedef const HANTREE* HANPCTREE;

typedef HANDATACMP (*HANTREECMPOPERATION)(void* pData1, void* pData2);

static inline HANTREE HANTreeInit(HANDATACMP nSortRule)
{
    HANTREE treeRet = {
        .pNode = NULL,
        .nSortRule = nSortRule,
        .nLen = 0,
        .pFor = NULL,
    };
    return treeRet;
}

HANERROR HANTreeAddNewData(HANPTREE pTree, void* pData, HANTREECMPOPERATION CompareOperation, HANPCALLOCDATA allocNode);

void HANTreePrint(HANPTREE pTree, void (*PrintOperation)(void* pData));

#ifdef __cplusplus
}
#endif

#endif
