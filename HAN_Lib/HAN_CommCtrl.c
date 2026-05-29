#include "HAN_CommCtrl.h"

/* ListView */
void ListViewDeleteAllColumn(HWND hListView)
{
    HWND hHeader = ListView_GetHeader(hListView);
    int nColumnCnt = ListView_GetItemCount(hHeader);
    for (int iLoop = 0; iLoop < nColumnCnt; iLoop++)
    { ListView_DeleteColumn(hListView, 0); }
}
