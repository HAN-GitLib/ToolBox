#include <stdio.h>
#include <Windows.h>
#include <CommCtrl.h>
#include <shlwapi.h>

#include "HAN_Lib\HAN_windows.h"
#include "ToolBox\GlobalVariables.h"
#include "ToolBox\ToolBoxMain.h"
#include "ToolBox\ConsoleCmd\HAN_ConsoleCmd.h"

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, HANPSTR lpCmdLine, int nCmdShow)
{
    (void)hPrevInstance;

    int ret = 0;

    if (FALSE == HanConsoleCmd(lpCmdLine))
    {
        // 注册窗口类
        WNDCLASSEX wcex = {
            .cbSize         = sizeof(WNDCLASSEX),
            .style          = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc    = ToolBoxMainWndProc,
            .cbClsExtra     = 0,
            .cbWndExtra     = 0,
            .hInstance      = hInstance,
            .hIcon          = LoadIcon(NULL, IDI_APPLICATION),
            .hCursor        = LoadCursor(NULL, IDC_ARROW),
            .hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1),
            .lpszMenuName   = NULL,
            .lpszClassName  = TEXT("ToolBox"),
            .hIconSm        = NULL,
        };
        RegisterClassEx(&wcex);

        // 创建窗口
        HWND hWnd = CreateWindow(TEXT("ToolBox"), TEXT("ToolBox"),
            WS_OVERLAPPEDWINDOW | WS_SYSMENU,
            CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
            NULL, NULL, hInstance, NULL);

        // 显示窗口
        ShowWindow(hWnd, nCmdShow);
        UpdateWindow(hWnd);

        // 主消息循环
        MSG msg;
        while (0 != GetMessage(&msg, NULL, 0, 0))
        {
            if (FALSE == ToolBoxTranslateAcceleratorCallback(&msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        ret = (int)msg.wParam;
    }

#ifdef HAN_CHECK_ALLOC_LEAK
    HANAllocPrintLeakInfo();
    system("pause");
#endif

    return ret;
}
