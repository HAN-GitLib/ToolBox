#include <stdio.h>

#include "HAN_ConsoleCmd.h"
#include "..\HexView\HAN_Hex.h"
#include "..\BinView\HAN_Bin.h"
#include "..\AscView\HAN_Asc.h"

#define HAN_HEX_BIN_STRLEN      1024

typedef enum {
    HANDATA_CMD_NONE,
    HANDATA_CMD_HELP,
    HANDATA_CMD_CF,
    HANDATA_CMD_OH,         // Hex 窗口下打开 Hex 文件
    HANDATA_CMD_HSB,        // Hex 窗口下保存 Bin 文件
    HANDATA_CMD_HSFH,       // Hex 窗口下保存填充后的 Hex 文件
    HANDATA_CMD_HLH,        // Hex 窗口下连接 Hex 文件
    HANDATA_CMD_HSL,        // Hex 窗口下保存拼接后 Hex 文件
    HANDATA_CMD_OS,         // Srec 窗口下打开 Srec 文件
    HANDATA_CMD_OB,         // Bin 窗口下打开 Bin 文件
    HANDATA_CMD_BSA,        // Bin 窗口下设置起始地址
    HANDATA_CMD_BSL,        // Bin 窗口下设置长度
    HANDATA_CMD_BSDP,       // Bin 窗口下导入数据的位置
    HANDATA_CMD_BSDL,       // Bin 窗口下导入数据的长度
    HANDATA_CMD_BSD,        // Bin 窗口下导入 Bin 文件
    HANDATA_CMD_BSCR,       // Bin 窗口下的选择 crc 程序
    HANDATA_CMD_BSCS,       // Bin 窗口下设置 crc 校验范围的起始地址
    HANDATA_CMD_BSCE,       // Bin 窗口下设置 crc 校验范围的结束地址
    HANDATA_CMD_BSCP,       // Bin 窗口下设置 crc 的存放位置
    HANDATA_CMD_BSC,        // Bin 窗口下计算 crc
    HANDATA_CMD_BSH,        // Bin 窗口下保存 Hex 文件
    HANDATA_CMD_OA,         // Asc 窗口下打开 Asc 文件
    HANDATA_CMD_ASIF,       // Asc 窗口下设置 ID 过滤
    HANDATA_CMD_ASCF,       // Asc 窗口下设置通道过滤
    HANDATA_CMD_ASC,        // Asc 窗口下保存 CSV 文件
} HEXCMD;

typedef enum {
    HANDATACMD_OK,
    HANDATACMD_CMDNONE,
    HANDATACMD_NOTPATH,
} HANDATACMDERR;

typedef struct tagHEXCMDINFO {
    WCHAR               pCmd[100];
    uint32_t            nNeedParamCnt;
    HANDATACMDERR       (*CmdAction)(char** pCmdParam);
    void                (*PrintHelp)(void);
} HANDATACMDINFO, * PHANDATACMDINFO;

static HANDATACMDERR CmdAction_help(char** pCmdParam);
static HANDATACMDERR CmdAction_cf(char** pCmdParam);
static HANDATACMDERR CmdAction_oh(char** pCmdParam);
static HANDATACMDERR CmdAction_hsb(char** pCmdParam);
static HANDATACMDERR CmdAction_hsfh(char** pCmdParam);
static HANDATACMDERR CmdAction_hlh(char** pCmdParam);
static HANDATACMDERR CmdAction_hsl(char** pCmdParam);
static HANDATACMDERR CmdAction_os(char** pCmdParam);
static HANDATACMDERR CmdAction_ob(char** pCmdParam);
static HANDATACMDERR CmdAction_bsa(char** pCmdParam);
static HANDATACMDERR CmdAction_bsl(char** pCmdParam);
static HANDATACMDERR CmdAction_bsdp(char** pCmdParam);
static HANDATACMDERR CmdAction_bsdl(char** pCmdParam);
static HANDATACMDERR CmdAction_bsd(char** pCmdParam);
static HANDATACMDERR CmdAction_bscr(char** pCmdParam);
static HANDATACMDERR CmdAction_bscs(char** pCmdParam);
static HANDATACMDERR CmdAction_bsce(char** pCmdParam);
static HANDATACMDERR CmdAction_bscp(char** pCmdParam);
static HANDATACMDERR CmdAction_bsc(char** pCmdParam);
static HANDATACMDERR CmdAction_bsh(char** pCmdParam);
static HANDATACMDERR CmdAction_oa(char** pCmdParam);
static HANDATACMDERR CmdAction_asif(char** pCmdParam);
static HANDATACMDERR CmdAction_ascf(char** pCmdParam);
static HANDATACMDERR CmdAction_asc(char** pCmdParam);

static void PrintHelp_help(void);
static void PrintHelp_cf(void);
static void PrintHelp_oh(void);
static void PrintHelp_hsb(void);
static void PrintHelp_hsfh(void);
static void PrintHelp_hlh(void);
static void PrintHelp_hsl(void);
static void PrintHelp_os(void);
static void PrintHelp_ob(void);
static void PrintHelp_bsa(void);
static void PrintHelp_bsl(void);
static void PrintHelp_bsdp(void);
static void PrintHelp_bsdl(void);
static void PrintHelp_bsd(void);
static void PrintHelp_bscr(void);
static void PrintHelp_bscs(void);
static void PrintHelp_bsce(void);
static void PrintHelp_bscp(void);
static void PrintHelp_bsc(void);
static void PrintHelp_bsh(void);
static void PrintHelp_oa(void);
static void PrintHelp_asif(void);
static void PrintHelp_ascf(void);
static void PrintHelp_asc(void);

HANDATACMDINFO g_pCmdInfo[] = {
    [HANDATA_CMD_HELP] = { .pCmd = L"help", .nNeedParamCnt = 0, .CmdAction = CmdAction_help, .PrintHelp = PrintHelp_help },
    [HANDATA_CMD_CF] = { .pCmd = L"cf", .nNeedParamCnt = 1, .CmdAction = CmdAction_cf, .PrintHelp = PrintHelp_cf },
    [HANDATA_CMD_OH] = { .pCmd = L"oh", .nNeedParamCnt = 1, .CmdAction = CmdAction_oh, .PrintHelp = PrintHelp_oh },
    [HANDATA_CMD_HSB] = { .pCmd = L"hsb", .nNeedParamCnt = 1, .CmdAction = CmdAction_hsb, .PrintHelp = PrintHelp_hsb },
    [HANDATA_CMD_HSFH] = { .pCmd = L"hsfh", .nNeedParamCnt = 1, .CmdAction = CmdAction_hsfh, .PrintHelp = PrintHelp_hsfh },
    [HANDATA_CMD_HLH] = { .pCmd = L"hlh", .nNeedParamCnt = 1, .CmdAction = CmdAction_hlh, .PrintHelp = PrintHelp_hlh },
    [HANDATA_CMD_HSL] = { .pCmd = L"hsl", .nNeedParamCnt = 1, .CmdAction = CmdAction_hsl, .PrintHelp = PrintHelp_hsl },
    [HANDATA_CMD_OS] = { .pCmd = L"os", .nNeedParamCnt = 1, .CmdAction = CmdAction_os, .PrintHelp = PrintHelp_os },
    [HANDATA_CMD_OB] = { .pCmd = L"ob", .nNeedParamCnt = 1, .CmdAction = CmdAction_ob, .PrintHelp = PrintHelp_ob },
    [HANDATA_CMD_BSA] = { .pCmd = L"bsa", .nNeedParamCnt = 1, .CmdAction = CmdAction_bsa, .PrintHelp = PrintHelp_bsa },
    [HANDATA_CMD_BSL] = { .pCmd = L"bsl", .nNeedParamCnt = 1, .CmdAction = CmdAction_bsl, .PrintHelp = PrintHelp_bsl },
    [HANDATA_CMD_BSDP] = { .pCmd = L"bsdp", .nNeedParamCnt = 1, .CmdAction = CmdAction_bsdp, .PrintHelp = PrintHelp_bsdp },
    [HANDATA_CMD_BSDL] = { .pCmd = L"bsdl", .nNeedParamCnt = 1, .CmdAction = CmdAction_bsdl, .PrintHelp = PrintHelp_bsdl },
    [HANDATA_CMD_BSD] = { .pCmd = L"bsd", .nNeedParamCnt = 1, .CmdAction = CmdAction_bsd, .PrintHelp = PrintHelp_bsd },
    [HANDATA_CMD_BSCR] = { .pCmd = L"bscr", .nNeedParamCnt = 1, .CmdAction = CmdAction_bscr, .PrintHelp = PrintHelp_bscr },
    [HANDATA_CMD_BSCS] = { .pCmd = L"bscs", .nNeedParamCnt = 1, .CmdAction = CmdAction_bscs, .PrintHelp = PrintHelp_bscs },
    [HANDATA_CMD_BSCE] = { .pCmd = L"bsce", .nNeedParamCnt = 1, .CmdAction = CmdAction_bsce, .PrintHelp = PrintHelp_bsce },
    [HANDATA_CMD_BSCP] = { .pCmd = L"bscp", .nNeedParamCnt = 1, .CmdAction = CmdAction_bscp, .PrintHelp = PrintHelp_bscp },
    [HANDATA_CMD_BSC] = { .pCmd = L"bsc", .nNeedParamCnt = 0, .CmdAction = CmdAction_bsc, .PrintHelp = PrintHelp_bsc },
    [HANDATA_CMD_BSH] = { .pCmd = L"bsh", .nNeedParamCnt = 1, .CmdAction = CmdAction_bsh, .PrintHelp = PrintHelp_bsh },
    [HANDATA_CMD_OA] = { .pCmd = L"oa", .nNeedParamCnt = 1, .CmdAction = CmdAction_oa, .PrintHelp = PrintHelp_oa },
    [HANDATA_CMD_ASIF] = { .pCmd = L"asif", .nNeedParamCnt = 1, .CmdAction = CmdAction_asif, .PrintHelp = PrintHelp_asif },
    [HANDATA_CMD_ASCF] = { .pCmd = L"ascf", .nNeedParamCnt = 1, .CmdAction = CmdAction_ascf, .PrintHelp = PrintHelp_ascf },
    [HANDATA_CMD_ASC] = { .pCmd = L"asc", .nNeedParamCnt = 1, .CmdAction = CmdAction_asc, .PrintHelp = PrintHelp_asc },
};

static HEXCMD           g_hcCmd = HANDATA_CMD_CF;
static HANDLE           g_hHeap;
static HEXFILE          g_hfHexFile;
static BINFILE          g_bfBinFile;
static ASCFILE          g_afAscFile;
static uint32_t         g_cBinDataStartAddr = DEFAULT_START_ADDR;
static uint32_t         g_nBinSetDataPos = DEFAULT_DATA_POS_VALUE;
static uint32_t         g_nBinSetDataLen = 0;
static uint32_t         g_nCrcId = DEFAULT_CRC_ID;
static uint32_t         g_nCrcStart = DEFAULT_CRC_START_VALUE;
static uint32_t         g_nCrcEnd = DEFAULT_CRC_END_VALUE;
static uint32_t         g_nCrcPos = DEFAULT_CRC_POS_VALUE;
static HANCHAR          g_pLinkList[LINK_FILE_CNT_MAX * PATH_STR_SIZE];
static uint32_t         g_pLinkCnt = 0;
static uint32_t         g_ascIdFilter;
static uint32_t         g_ascChFilter;
static uint32_t*        g_pAscIdFilter = NULL;
static uint32_t*        g_pAscChFilter = NULL;

static void RunCmd(PWCHAR pCmd);

BOOL HanConsoleCmd(HANPSTR pCmd)
{
    BOOL bRet = TRUE;
    
    if (HAN_strlen(pCmd) == 0)
    {
        bRet = FALSE;
    }

    if (TRUE == bRet)
    {
        AttachConsole(-1);
        (void)freopen("conout$","w",stdout);

        g_hHeap = GetProcessHeap();
        g_hfHexFile.amBinAddrMap.cStartAddr = (uint32_t)DEFAULT_START_ADDR;
        g_hfHexFile.amBinAddrMap.cEndAddr = (uint32_t)DEFAULT_END_ADDR;

        int argCnt;
        LPWSTR *pArgList;
        WCHAR pWideCmd[2048];
        int nUnicodeLen = HANStrToWStr(CP_ACP, 0, pCmd, -1, NULL, 0);
        HANStrToWStr(CP_ACP, 0, pCmd, -1, pWideCmd, nUnicodeLen);
        pArgList = CommandLineToArgvW(pWideCmd, &argCnt);
        for (int i = 0; i < argCnt; i++)
        {
            RunCmd(pArgList[i]);
        }
        LocalFree(pArgList);
    }
    
    return bRet;
}

static void RunCmd(PWCHAR pCmd)
{
    BOOL bInfoCmd = FALSE;
    for (uint32_t i = 1; i < ArrLen(g_pCmdInfo); i++)
    {
        if (0 == wcscmp(pCmd, g_pCmdInfo[i].pCmd))
        {
            if (0 < g_pCmdInfo[i].nNeedParamCnt)
            {
                g_hcCmd = i;
            }
            else
            {
                g_pCmdInfo[i].CmdAction(NULL);
            }
            bInfoCmd = TRUE;
            break;
        }
    }
    if (FALSE == bInfoCmd)
    {
        char pMultiByte[2048];
        char* pParam = pMultiByte;
        int nMultiByteLen = WideCharToMultiByte(CP_ACP, 0, pCmd, -1, NULL, 0, NULL, NULL);
        WideCharToMultiByte(CP_ACP, 0, pCmd, -1, pMultiByte, nMultiByteLen, NULL, NULL);

        g_pCmdInfo[g_hcCmd].CmdAction(&pParam);
    }
}

static HANDATACMDERR CmdAction_help(char** pCmdParam)
{
    (void)pCmdParam;

    for (uint32_t i = 1; i < ArrLen(g_pCmdInfo); i++)
    {
        printf("%-12ls", g_pCmdInfo[i].pCmd);
        g_pCmdInfo[i].PrintHelp();
    }

    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_cf(char** pCmdParam)
{
    printf("解析数据文件：%s\n", *pCmdParam);

    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_oh(char** pCmdParam)
{
    HANCHAR pPath[PATH_STR_SIZE];

    printf("打开 hex 文件：%s\n", *pCmdParam);
    HAN_snprintf(pPath, PATH_STR_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, *pCmdParam);
    HANOpenHexFile(&g_hfHexFile, pPath, g_hHeap);

    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_hsb(char** pCmdParam)
{
    HANDLE hFile;
    HANCHAR pPath[PATH_STR_SIZE];
    uint32_t cHexStartAddr = g_hfHexFile.amBinAddrMap.cStartAddr;
    uint32_t cHexEndAddr = g_hfHexFile.amBinAddrMap.cEndAddr;

    printf("保存为 bin 文件：%s\n起始地址：%08X\n结束地址：%08X\n",
        *pCmdParam, cHexStartAddr, cHexEndAddr);
    HexViewHexToBin(&g_hfHexFile, &(g_hfHexFile.amBinAddrMap), g_hHeap);

    HAN_snprintf(pPath, PATH_STR_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, *pCmdParam);

    hFile = CreateFile(pPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (INVALID_HANDLE_VALUE != hFile)
    {
        PHEXFILE pHexFile = &g_hfHexFile;
        DWORD nLen = pHexFile->amBinAddrMap.cEndAddr - pHexFile->amBinAddrMap.cStartAddr + (uint32_t)1;
        WriteFile(hFile, pHexFile->pBinData, nLen, NULL, NULL);
        CloseHandle(hFile);
    }

    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_hsfh(char** pCmdParam)
{
    (void)pCmdParam;
    
    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_hlh(char** pCmdParam)
{
    printf("链接 hex 文件：%s\n", *pCmdParam);
    HAN_snprintf(&g_pLinkList[g_pLinkCnt * PATH_STR_SIZE], PATH_STR_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, *pCmdParam);
    g_pLinkCnt++;

    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_hsl(char** pCmdParam)
{
    HANCHAR pPath[PATH_STR_SIZE];

    printf("保存链接的 hex 文件到：%s\n", *pCmdParam);
    HAN_snprintf(pPath, PATH_STR_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, *pCmdParam);
    LinkHexFile(pPath, g_pLinkList, g_pLinkCnt);
    g_pLinkCnt = 0;

    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_os(char** pCmdParam)
{
    (void)pCmdParam;
    
    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_ob(char** pCmdParam)
{
    HANCHAR pPath[PATH_STR_SIZE];

    printf("打开 bin 文件：%s\n", *pCmdParam);
    HAN_snprintf(pPath, PATH_STR_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, *pCmdParam);
    HANOpenBinFile(&g_bfBinFile, pPath, g_hHeap);

    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_bsa(char** pCmdParam)
{
    g_cBinDataStartAddr = strtoul(*pCmdParam, NULL, 16);
    printf("设置 bin 文件的起始地址：0x%08X\n", g_cBinDataStartAddr);

    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_bsl(char** pCmdParam)
{
    uint32_t nLen = strtoul(*pCmdParam, NULL, 16);
    if (0 != nLen) { g_bfBinFile.nBinLen = nLen; }
    printf("设置 bin 文件的长度：0x%08X\n", g_bfBinFile.nBinLen);

    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_bsdp(char** pCmdParam)
{
    g_nBinSetDataPos = strtoul(*pCmdParam, NULL, 16);
    printf("设置导入数据的偏移量：0x%X\n", g_nBinSetDataPos);

    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_bsdl(char** pCmdParam)
{
    g_nBinSetDataLen = strtoul(*pCmdParam, NULL, 16);
    if (0 == g_nBinSetDataLen) { printf("设置导入数据的长度：全文件\n"); }
    else { printf("设置导入数据的长度：0x%X\n", g_nBinSetDataLen); }

    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_bsd(char** pCmdParam)
{
    HANCHAR pPath[PATH_STR_SIZE];

    printf("写入数据文件：%s\n", *pCmdParam);
    HANBinFileSetData(g_bfBinFile.pBinData, g_bfBinFile.nBinLen, pPath, g_nBinSetDataPos, g_nBinSetDataLen);
    
    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_bscr(char** pCmdParam)
{
    uint32_t nCrcId = strtoul(*pCmdParam, NULL, 10);
    HANCHAR pCrcName[CRC_NAME_MAX_CNT];
    BOOL bCrcRet = HANBinViewGetCrcRoutine(nCrcId, pCrcName, ArrLen(pCrcName), NULL);
    if (TRUE == bCrcRet)
    {
        printf("设置 crc 程序：" PCHAR_PRINT_HANPSTR_FORMAT "\n", pCrcName);
        g_nCrcId = nCrcId;
    }
    else
    {
        HANBinViewGetCrcRoutine(g_nCrcId, pCrcName, ArrLen(pCrcName), NULL);
        printf("未找到 crc 程序，使用原 crc 程序：" PCHAR_PRINT_HANPSTR_FORMAT "\n", pCrcName);
    }
    
    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_bscs(char** pCmdParam)
{
    g_nCrcStart = strtoul(*pCmdParam, NULL, 16);
    printf("设置 crc 检验范围：0x%X ~ 0x%X\n", g_nCrcStart, g_nCrcEnd);
    
    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_bsce(char** pCmdParam)
{
    g_nCrcEnd = strtoul(*pCmdParam, NULL, 16);
    printf("设置 crc 检验范围：0x%X ~ 0x%X\n", g_nCrcStart, g_nCrcEnd);
    
    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_bscp(char** pCmdParam)
{
    g_nCrcPos = strtoul(*pCmdParam, NULL, 16);
    printf("设置 crc 存放位置：0x%X\n", g_nCrcPos);
    
    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_bsc(char** pCmdParam)
{
    (void)pCmdParam;
    
    printf("写入 crc\n    校验范围：0x%X ~ 0x%X\n    写入位置：0x%X\n", g_nCrcStart, g_nCrcEnd, g_nCrcPos);
    uint32_t nDataLen = g_nCrcEnd - g_nCrcStart + 1;
    uint32_t nCrcSize;
    uint32_t cCrcValue = HANBinFileSetCrc(
        g_bfBinFile.pBinData,
        nDataLen,
        g_nCrcId,
        &(g_bfBinFile.pBinData[g_nCrcPos]));
    char pCrcValueFormat[100];
    HANBinViewGetCrcRoutine(g_nCrcId, NULL, 0, &nCrcSize);
    nCrcSize *= 2;
    snprintf(pCrcValueFormat, ArrLen(pCrcValueFormat), "    校验码：%%0%uX\n", nCrcSize);
    printf(pCrcValueFormat, cCrcValue);
    
    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_bsh(char** pCmdParam)
{
    HANCHAR pPath[PATH_STR_SIZE];

    HAN_snprintf(pPath, PATH_STR_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, *pCmdParam);
    HANDLE hFile = CreateFile(pPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (INVALID_HANDLE_VALUE != hFile)
    {
        printf("保存 hex 文件：%s\n", *pCmdParam);
        BinDataToHexFile(hFile, g_cBinDataStartAddr, g_bfBinFile.pBinData, g_bfBinFile.nBinLen);
    }
    
    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_oa(char** pCmdParam)
{
    HANCHAR pPath[PATH_STR_SIZE];

    printf("打开 asc 文件：%s\n", *pCmdParam);
    HAN_snprintf(pPath, PATH_STR_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, *pCmdParam);
    HANOpenAscFile(&g_afAscFile, pPath, g_hHeap);
    
    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_asif(char** pCmdParam)
{
    g_pAscIdFilter = &g_ascIdFilter;
    g_ascIdFilter = strtoul(*pCmdParam, NULL, 16);
    printf("设置 asc 文件 ID 过滤：%X\n", g_ascIdFilter);
    
    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_ascf(char** pCmdParam)
{
    g_pAscChFilter = &g_ascChFilter;
    g_ascChFilter = strtoul(*pCmdParam, NULL, 10);
    printf("设置 asc 文件通道过滤：%X\n", g_ascChFilter);
    
    return HANDATACMD_OK;
}
static HANDATACMDERR CmdAction_asc(char** pCmdParam)
{
    HANCHAR pPath[PATH_STR_SIZE];

    printf("保存 hex 文件：%s\n", *pCmdParam);
    HAN_snprintf(pPath, PATH_STR_SIZE, HANPSTR_PRINT_PCHAR_FORMAT, *pCmdParam);
    AscViewAscToCsv(&g_afAscFile, pPath, g_pAscIdFilter, g_pAscChFilter);
    
    return HANDATACMD_OK;
}

static void PrintHelp_help(void)
{
    printf("打印支持的指令\n");
}
static void PrintHelp_cf(void)
{
    printf("解析数据文件，需要跟一个文件名参数\n");
}
static void PrintHelp_oh(void)
{
    printf("打开 hex 文件，需要跟一个文件名参数\n");
}
static void PrintHelp_hsb(void)
{
    printf("hex 页面下的保存 bin 文件，需要跟一个文件名参数\n");
}
static void PrintHelp_hsfh(void)
{
    printf("hex 页面下的保存填充后的 hex 文件，需要跟一个文件名参数\n");
}
static void PrintHelp_hlh(void)
{
    printf("hex 页面下的添加链接文件（加号按钮），需要跟一个文件名参数\n");
}
static void PrintHelp_hsl(void)
{
    printf("hex 页面下的保存拼接后的 hex 文件，需要跟一个文件名参数\n");
}
static void PrintHelp_os(void)
{
    printf("打开 srec 文件，需要跟一个文件名参数\n");
}
static void PrintHelp_ob(void)
{
    printf("打开 bin 文件，需要跟一个文件名参数\n");
}
static void PrintHelp_bsa(void)
{
    printf("bin 页面下的设置起始地址，需要跟一个地址参数\n");
}
static void PrintHelp_bsl(void)
{
    printf("bin 页面下的设置长度（0表示原文件大小），需要跟一个长度参数\n");
}
static void PrintHelp_bsdp(void)
{
    printf("bin 页面下的设置写入数据的偏移量，需要跟一个偏移量参数\n");
}
static void PrintHelp_bsdl(void)
{
    printf("bin 页面下的设置写入数据的长度，需要跟一个长度参数\n");
}
static void PrintHelp_bsd(void)
{
    printf("bin 页面下的导入 bin 文件，需要跟一个文件名参数\n");
}
static void PrintHelp_bscr(void)
{
    printf("bin 页面下的选择 crc 程序，需要跟一个 10 进制程序编号参数，支持的编号有：\n");
    HANCHAR pCrcName[CRC_NAME_MAX_CNT];
    uint32_t iFor = 0;
    while (TRUE == HANBinViewGetCrcRoutine(iFor, pCrcName, ArrLen(pCrcName), NULL))
    {
        printf("                %-6u" PCHAR_PRINT_HANPSTR_FORMAT "\n", iFor, pCrcName);
        iFor++;
    }
}
static void PrintHelp_bscs(void)
{
    printf("bin 页面下的设置 crc 校验范围的起始地址，需要跟一个 16 进制参数\n");
}
static void PrintHelp_bsce(void)
{
    printf("bin 页面下的设置 crc 校验范围的结束地址，需要跟一个 16 进制参数\n");
}
static void PrintHelp_bscp(void)
{
    printf("bin 页面下的设置 crc 的存放位置，需要跟一个 16 进制参数\n");
}
static void PrintHelp_bsc(void)
{
    printf("bin 页面下的计算 crc\n");
}
static void PrintHelp_bsh(void)
{
    printf("bin 页面下的保存 hex 文件，需要跟一个文件名参数\n");
}
static void PrintHelp_oa(void)
{
    printf("打开 asc 文件，需要跟一个文件名参数\n");
}
static void PrintHelp_asif(void)
{
    printf("asc 页面下设置 ID 过滤，需要跟一个 16 进制 ID 参数\n");
}
static void PrintHelp_ascf(void)
{
    printf("asc 页面下设置通道过滤，需要跟一个 10 进制参数\n");
}
static void PrintHelp_asc(void)
{
    printf("asc 页面下保存 csv 文件，需要跟一个文件名参数\n");
}
