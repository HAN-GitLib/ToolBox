#include "HAN_VideoH264Addr.h"

static const uint8_t sg_pluma8x8BlkIdxNMap[4][2] = {
    [0] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 1, [VIDEO_H264_NEIGHBOUR_ABOVE] = 2, },
    [1] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 0, [VIDEO_H264_NEIGHBOUR_ABOVE] = 3, },
    [2] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 3, [VIDEO_H264_NEIGHBOUR_ABOVE] = 0, },
    [3] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 2, [VIDEO_H264_NEIGHBOUR_ABOVE] = 1, },
};
static const uint8_t sg_pluma4x4BlkIdxNMap[16][2] = {
    [0] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 5, [VIDEO_H264_NEIGHBOUR_ABOVE] = 10, },
    [1] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 0, [VIDEO_H264_NEIGHBOUR_ABOVE] = 11, },
    [2] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 7, [VIDEO_H264_NEIGHBOUR_ABOVE] = 0, },
    [3] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 2, [VIDEO_H264_NEIGHBOUR_ABOVE] = 1, },
    [4] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 1, [VIDEO_H264_NEIGHBOUR_ABOVE] = 14, },
    [5] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 4, [VIDEO_H264_NEIGHBOUR_ABOVE] = 15, },
    [6] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 3, [VIDEO_H264_NEIGHBOUR_ABOVE] = 4, },
    [7] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 6, [VIDEO_H264_NEIGHBOUR_ABOVE] = 5, },
    [8] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 13, [VIDEO_H264_NEIGHBOUR_ABOVE] = 2, },
    [9] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 8, [VIDEO_H264_NEIGHBOUR_ABOVE] = 3, },
    [10] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 15, [VIDEO_H264_NEIGHBOUR_ABOVE] = 8, },
    [11] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 10, [VIDEO_H264_NEIGHBOUR_ABOVE] = 9, },
    [12] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 9, [VIDEO_H264_NEIGHBOUR_ABOVE] = 6, },
    [13] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 12, [VIDEO_H264_NEIGHBOUR_ABOVE] = 7, },
    [14] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 11, [VIDEO_H264_NEIGHBOUR_ABOVE] = 12, },
    [15] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 14, [VIDEO_H264_NEIGHBOUR_ABOVE] = 13, },
};
static const uint8_t sg_pchroma4x4BlkIdxNMap[8][2] = { // 仅针对 4:2:2 采样，如果是 4:2:0，请使用 sg_pluma8x8BlkIdxNMap
    [0] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 1, [VIDEO_H264_NEIGHBOUR_ABOVE] = 6, }, [1] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 0, [VIDEO_H264_NEIGHBOUR_ABOVE] = 7, },
    [2] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 3, [VIDEO_H264_NEIGHBOUR_ABOVE] = 0, }, [3] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 2, [VIDEO_H264_NEIGHBOUR_ABOVE] = 1, },
    [4] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 5, [VIDEO_H264_NEIGHBOUR_ABOVE] = 2, }, [5] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 4, [VIDEO_H264_NEIGHBOUR_ABOVE] = 3, },
    [6] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 7, [VIDEO_H264_NEIGHBOUR_ABOVE] = 4, }, [7] = { [VIDEO_H264_NEIGHBOUR_LEFT] = 6, [VIDEO_H264_NEIGHBOUR_ABOVE] = 5, },
};
static const uint8_t sg_pPartitionLocationMap[2][2][4][2] = { /* 查询分区的 x、y 的索引
     * 4 个维度分别是 [MbPartWidth >> 4][MbPartHeight >> 4][mbPartIdx][x/y]，
     * 输出是所在行的索引，需要根据场景计算像素
     */
    [1][1] = { // MbPartWidth = 16, MbPartHeight = 16
        [0] = { [VIDEO_H264_LOCATION_X] = 0, [VIDEO_H264_LOCATION_Y] = 0, },
    },
    [1][0] = { // MbPartWidth = 16, MbPartHeight = 8
        [0] = { [VIDEO_H264_LOCATION_X] = 0, [VIDEO_H264_LOCATION_Y] = 0, },
        [1] = { [VIDEO_H264_LOCATION_X] = 0, [VIDEO_H264_LOCATION_Y] = 1, },
    },
    [0][1] = { // MbPartWidth = 8, MbPartHeight = 16
        [0] = { [VIDEO_H264_LOCATION_X] = 0, [VIDEO_H264_LOCATION_Y] = 0, },
        [1] = { [VIDEO_H264_LOCATION_X] = 1, [VIDEO_H264_LOCATION_Y] = 0, },
    },
    [0][0] = { // MbPartWidth = 8, MbPartHeight = 8
        [0] = { [VIDEO_H264_LOCATION_X] = 0, [VIDEO_H264_LOCATION_Y] = 0, },
        [1] = { [VIDEO_H264_LOCATION_X] = 1, [VIDEO_H264_LOCATION_Y] = 0, },
        [2] = { [VIDEO_H264_LOCATION_X] = 0, [VIDEO_H264_LOCATION_Y] = 1, },
        [3] = { [VIDEO_H264_LOCATION_X] = 1, [VIDEO_H264_LOCATION_Y] = 1, },
    },
};

HANSIZE H264GetprevMbAddr(PVIDEOH264DECODEINFO pDecodeInfo)
{
    HANSIZE addrRet;
    HANSIZE CurrMbAddr = pDecodeInfo->CurrMbAddr;
    HANSIZE addrMin = pDecodeInfo->pPicInfo->slice.slice_header.first_mb_in_slice;

    if (CurrMbAddr == addrMin) { addrRet = VIDEO_H264_MACRO_BLOCK_ADDR_INVALID; }
    else { addrRet = CurrMbAddr - 1; }

    return addrRet;
}

void DerivationProcessForNeighbouringMacroblocks(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2])
{
    HANSIZE CurrMbAddr = pDecodeInfo->CurrMbAddr;
    HANSIZE addrMin = pDecodeInfo->pPicInfo->slice.slice_header.first_mb_in_slice;
    uint32_t PicWidthInMbs = pDecodeInfo->pPicInfo->slice.pSPS->PicWidthInMbs;

    /* mbAddrA */
    if ((0 == CurrMbAddr) || (0 == (CurrMbAddr % PicWidthInMbs))) { mbAddrN[VIDEO_H264_NEIGHBOUR_LEFT] = VIDEO_H264_MACRO_BLOCK_ADDR_INVALID; }
    else { mbAddrN[VIDEO_H264_NEIGHBOUR_LEFT] = CurrMbAddr - 1; }
    if (mbAddrN[VIDEO_H264_NEIGHBOUR_LEFT] < addrMin) { mbAddrN[VIDEO_H264_NEIGHBOUR_LEFT] = VIDEO_H264_MACRO_BLOCK_ADDR_INVALID; }
    /* mbAddrB */
    addrMin += PicWidthInMbs;
    if (CurrMbAddr < addrMin) { mbAddrN[VIDEO_H264_NEIGHBOUR_ABOVE] = VIDEO_H264_MACRO_BLOCK_ADDR_INVALID; }
    else { mbAddrN[VIDEO_H264_NEIGHBOUR_ABOVE] = CurrMbAddr - PicWidthInMbs; }
}

void DerivationProcessForNeighbouring8x8LumaBlock(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t luma8x8BlkIdx, HANSIZE mbAddrN[2], uint8_t luma8x8BlkIdxN[2])
{
    HANSIZE CurrMbAddr = pDecodeInfo->CurrMbAddr;

    if (0 == pDecodeInfo->pPicInfo->slice.MbaffFrameFlag) // 普通模式
    {
        /* 标准中的公式太抽象，这里改用逻辑判断
         * 亮度块的 mbAddrN 是指该亮度块的相邻亮度块所在的宏块地址
         * luma8x8BlkIdxN 是指该亮度块的相邻亮度块的 luma8x8BlkIdx
         * luma8x8BlkIdx 就是某个 8x8 亮度块在其所属的宏块内的位置编号，按从左到右，从上到下的顺序
         * 如下图（A 和 B 的亮度块图里没画出来，但和右下角的数字排列一致）：
         * ┌───────┬───────┐
         * │       │       │
         * │       │   B   │
         * │       │       │
         * ├───────┼───┬───┤
         * │       │ 0 │ 1 │
         * │   A   ├───┼───┤
         * │       │ 2 │ 3 │
         * └───────┴───┴───┘
         * 对亮度块 0 来说，它的左邻亮度块对应 A 的 1，上邻亮度块对应 B 的 2，所以它的 mbAddrA 和 mbAddrB 就是宏块 A 和 B 的地址，luma8x8BlkIdxA = 1，luma8x8BlkIdxB = 2
         * 对亮度块 1 来说，它的左邻亮度块对应所在宏块的 0，上邻亮度块对应 B 的 3，所以它的 mbAddrA = CurrMbAddr，mbAddrB 就是宏块 B 的地址，luma8x8BlkIdxA = 0，luma8x8BlkIdxB = 3
         * 亮度块 2、3 同理
         */
        DerivationProcessForNeighbouringMacroblocks(pDecodeInfo, mbAddrN); // 先当普通宏块计算 mbAddrN
        switch (luma8x8BlkIdx) { // 修正宏块地址为 CurrMbAddr 的情况
            case 1: { mbAddrN[VIDEO_H264_NEIGHBOUR_LEFT] = CurrMbAddr; } break;
            case 2: { mbAddrN[VIDEO_H264_NEIGHBOUR_ABOVE] = CurrMbAddr; } break;
            case 3: { mbAddrN[VIDEO_H264_NEIGHBOUR_LEFT] = CurrMbAddr; mbAddrN[VIDEO_H264_NEIGHBOUR_ABOVE] = CurrMbAddr; } break;
            default: { } break;
        }
        for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
        {
            /* 如果 mbAddrN 无效，则 luma8x8BlkIdxN 也无效
             * 否则查表输出 luma8x8BlkIdxN
             */
            if (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID == mbAddrN[iLoop]) { luma8x8BlkIdxN[iLoop] = VIDEO_H264_BLOCK_IDX_INVALID; }
            else { luma8x8BlkIdxN[iLoop] = sg_pluma8x8BlkIdxNMap[luma8x8BlkIdx][iLoop]; }
        }
    }
    else // 自适应帧场模式
    {
        printf("自适应帧场模式尚未开发\n");
    }
}

void DerivationProcessForNeighbouring4x4LumaBlock(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t luma4x4BlkIdx, HANSIZE mbAddrN[2], uint8_t luma4x4BlkIdxN[2])
{
    HANSIZE CurrMbAddr = pDecodeInfo->CurrMbAddr;

    if (0 == pDecodeInfo->pPicInfo->slice.MbaffFrameFlag) // 普通模式
    {
        /* 原理和 8x8LumaBlock 相同，4x4 将每个 8x8 进一步分成 4 个小块，并且编号在每个小块内是连续的，如下图
        * ┌───┬───┬───┬───┐
        * │ 0 │ 1 │ 4 │ 5 │
        * │───┼───┼───┼───│
        * │ 2 │ 3 │ 6 │ 7 │
        * ├───┼───┼───┼───┤
        * │ 8 │ 9 │ 12│ 13│
        * │───┼───┼───┼───┤
        * │ 10│ 11│ 14│ 15│
        * └───┴───┴───┴───┘
        * 在 DerivationProcessForNeighbouring8x8LumaBlock 函数中有说明，8x8 的序号顺序是 1 左上，2 右上，3 左下，4 右下
        * 在序号为 1 的 8x8 内，按同样的顺序对 4x4 连续编号，并且在序号为 2 的 8x8 内，继续以同样的规则编号下去
        */
        DerivationProcessForNeighbouringMacroblocks(pDecodeInfo, mbAddrN); // 先当普通宏块计算 mbAddrN
        // 修正宏块地址为 CurrMbAddr 的情况
        if (0 != (luma4x4BlkIdx & 0b0101)) { mbAddrN[VIDEO_H264_NEIGHBOUR_LEFT] = CurrMbAddr; } // 非 0、2、8、10（它们的特征是与 0b0101 位与后是 0，其它数字不会）的左邻是当前宏块
        if (0 != (luma4x4BlkIdx & 0b1010)) { mbAddrN[VIDEO_H264_NEIGHBOUR_ABOVE] = CurrMbAddr; } // 非 0、1、4、5（它们的特征是与 0b1010 位与后是 0，其它数字不会）的上邻是当前宏块
        for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
        {
            /* 如果 mbAddrN 无效，则 luma4x4BlkIdxN 也无效
             * 否则查表输出 luma4x4BlkIdxN
             */
            if (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID == mbAddrN[iLoop]) { luma4x4BlkIdxN[iLoop] = VIDEO_H264_BLOCK_IDX_INVALID; }
            else { luma4x4BlkIdxN[iLoop] = sg_pluma4x4BlkIdxNMap[luma4x4BlkIdx][iLoop]; }
        }
    }
    else // 自适应帧场模式
    {
        printf("自适应帧场模式尚未开发\n");
    }
}

void DerivationProcessForNeighbouring4x4ChromaBlock(PVIDEOH264DECODEINFO pDecodeInfo, uint8_t chroma4x4BlkIdx, HANSIZE mbAddrN[2], uint8_t chroma4x4BlkIdxN[2])
{
    HANSIZE CurrMbAddr = pDecodeInfo->CurrMbAddr;
    
    /* 标准中的公式比较抽象，下面总结一下
     * 4x4 色度块仅 4:2:0 或 4:2:2 会用到，它们横向都是 2 倍降采样，所以总宽度只有 8，布局如下：
     * ┌───┬───┐
     * │ 0 │ 1 │
     * ├───┼───┤
     * │ 2 │ 3 │
     * ├───┼───┤
     * │ 4 │ 5 │
     * │───┼───┤
     * │ 6 │ 7 │
     * └───┴───┘
     * 其中上半部分两种采样都有效，下半部分仅 4:2:2 有效
     * 剩下的相邻块的计算原理就和 8x8LumaBlock 一样了
     * 
     * 所以计算 chroma4x4BlkIdxN 需要根据采样率分情况计算
     */
    DerivationProcessForNeighbouringMacroblocks(pDecodeInfo, mbAddrN); // 先当普通宏块计算 mbAddrN
    /* 然后修正落在当前宏块的索引 */
    if (0 != (chroma4x4BlkIdx & 1)) { mbAddrN[VIDEO_H264_NEIGHBOUR_LEFT] = CurrMbAddr; } // 奇数列的左邻都落在当前宏块
    if (1 < chroma4x4BlkIdx) { mbAddrN[VIDEO_H264_NEIGHBOUR_ABOVE] = CurrMbAddr; } // 大于 1 的上邻都落在当前宏块
    /* 计算 chroma4x4BlkIdxN */
    switch (pDecodeInfo->pPicInfo->slice.pSPS->ChromaArrayType) {
        case 1: { // 4:2:0，只有上半部分
            for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
            {
                if (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID == mbAddrN[iLoop]) { chroma4x4BlkIdxN[iLoop] = VIDEO_H264_BLOCK_IDX_INVALID; }
                else { chroma4x4BlkIdxN[iLoop] = sg_pluma8x8BlkIdxNMap[chroma4x4BlkIdx][iLoop]; } // 由于索引布局和 luma8x8 完全一样，所以这里直接用了 8x8 亮度表
            }
        } break;
        case 2: { // 4:2:2，上下两部分都有
            for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
            {
                if (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID == mbAddrN[iLoop]) { chroma4x4BlkIdxN[iLoop] = VIDEO_H264_BLOCK_IDX_INVALID; }
                else { chroma4x4BlkIdxN[iLoop] = sg_pchroma4x4BlkIdxNMap[chroma4x4BlkIdx][iLoop]; } // 使用色度表
            }
        } break;
        default: { } break;
    }
}

void DerivationProcessForNeighbouringPartitions(PVIDEOH264DECODEINFO pDecodeInfo, HANSIZE mbAddrN[2], uint8_t mbPartIdxN[2], uint8_t subMbPartIdxN[2])
{
    HANSIZE CurrMbAddr = pDecodeInfo->CurrMbAddr;
    uint8_t mbPartIdx = pDecodeInfo->mbPartIdx;
    PVIDEOH264PICTUREINFO pPicInfo = pDecodeInfo->pPicInfo;
    PVIDEOH264MACROBLOCK pMacroBlock = pPicInfo->pMacroBlock;
    PVIDEOH264MACROBLOCK pCurrentMb = &pMacroBlock[CurrMbAddr];
    PVIDEOH264MACROBLOCK pNeighbourMb;

    uint8_t MbPartWidthDiv16 = pCurrentMb->MbPartWidth >> 4;
    uint8_t MbPartHeightDiv16 = pCurrentMb->MbPartHeight >> 4;
    uint8_t x;
    uint8_t y;
    uint8_t xS;
    uint8_t yS;
    uint8_t xxS;
    uint8_t yyS;
    int16_t xN[2];
    int16_t yN[2];
    int16_t xW;
    int16_t yW;

    /* 纯用逻辑判断分块的地址会很复杂，所以这里使用逻辑和标准提供的公式结合的方式实现，以下先对分块的原理作简单说明
     * 分块有以下 4 种情况：
     *  1. 16x16        2. 16x8         3. 8x16         4. 8x8
     * ┌───────┐       ┌───────┐       ┌───┬───┐       ┌───┬───┐
     * │       │       │   0   │       │   │   │       │ 0 │ 1 │
     * │   0   │       ├───────┤       │ 0 │ 1 │       ├───┼───┤
     * │       │       │   1   │       │   │   │       │ 2 │ 3 │
     * └───────┘       └───────┘       └───┴───┘       └───┴───┘
     * 图中的数字就是 mbPartIdx 或 subMbPartIdx 其中：
     * mbPartIdx 是对宏块分区后的 ID 编号
     * subMbPartIdx 是分区后对块进一步分成子块的 ID 编号
     * 以上规则也说明了，只有分块为 4 个 8x8 块的宏块（如 P_8x8, P_8x8ref0 或 B_8x8 类型的宏块）才可能进入划分子块的可能（毕竟只有正方形才能分块）
     * 
     * 标准一共分 7 步计算，后续在每一步的代码中说明
     */
    /* 先确定相邻宏块的地址 */
    DerivationProcessForNeighbouringMacroblocks(pDecodeInfo, mbAddrN);
    /* 标准中的第 1 步是计算 mbPartIdx 所对应的分块的左上角像素在 16x16 的宏块中的位置 */
    x = sg_pPartitionLocationMap[MbPartWidthDiv16][MbPartHeightDiv16][mbPartIdx][VIDEO_H264_LOCATION_X] << 3;
    y = sg_pPartitionLocationMap[MbPartWidthDiv16][MbPartHeightDiv16][mbPartIdx][VIDEO_H264_LOCATION_Y] << 3;
    /* 标准中的第 2 步是计算 subMbPartIdx 所对应的子分块的左上角像素在分块中的位置，和 mbPartIdx 的区别是，这里是在 8x8 的分区中的位置 */
    xS = sg_pPartitionLocationMap[MbPartWidthDiv16][MbPartHeightDiv16][mbPartIdx][VIDEO_H264_LOCATION_X] << 2;
    yS = sg_pPartitionLocationMap[MbPartWidthDiv16][MbPartHeightDiv16][mbPartIdx][VIDEO_H264_LOCATION_Y] << 2;
    /* 标准中的第 3 步计算 predPartWidth 仅在计算位置 C（右上块）时会用到，这里不做多余计算 */
    /* 标准中的第 4 步和第 5 步合并，直接计算 xN 和 yN */
    xxS = x + xS;
    yyS = y + yS;
    xN[VIDEO_H264_NEIGHBOUR_LEFT] = xxS - 1;
    yN[VIDEO_H264_NEIGHBOUR_LEFT] = yyS;
    xN[VIDEO_H264_NEIGHBOUR_ABOVE] = xxS;
    yN[VIDEO_H264_NEIGHBOUR_ABOVE] = yyS - 1;
    /* 标准中的第 6 步是通过 xN 和 yN 确定落在哪个宏块，对 mbAddrA 来说，如果 xN < 0，则是 mbAddrA，如果大于 0，则是 CurrMbAddr，mbAddrB 同理 */
    if (0 < xN[VIDEO_H264_NEIGHBOUR_LEFT]) { mbAddrN[VIDEO_H264_NEIGHBOUR_LEFT] = pDecodeInfo->CurrMbAddr; }
    if (0 < yN[VIDEO_H264_NEIGHBOUR_ABOVE]) { mbAddrN[VIDEO_H264_NEIGHBOUR_ABOVE] = pDecodeInfo->CurrMbAddr; }
    /* 标准中的第 7 步 xW 和 yW 的计算简单来讲就是：如果 xN < 0，xW = 宏块的宽度 - 1（标准 6.4.11.7 明确分区按亮度采样大小，6.4.12明确此时 maxW = maxH = 16，所以此时 xW 必是 15），否则 xW = xN，yW 同理 */
    for (HANSIZE iLoop = 0; iLoop < 2; iLoop++)
    {
        if (VIDEO_H264_MACRO_BLOCK_ADDR_INVALID == mbAddrN[iLoop])
        {
            mbPartIdxN[iLoop] = VIDEO_H264_PARTITION_IDX_INVALID;
            if (NULL != subMbPartIdxN) { subMbPartIdxN[iLoop] = VIDEO_H264_PARTITION_IDX_INVALID; }
        }
        else
        {
            if (xN[iLoop] < 0) { xW = 15; }
            else { xW = xN[iLoop]; }
            if (yN[iLoop] < 0) { yW = 15; }
            else { yW = yN[iLoop]; }
            pNeighbourMb = &(pMacroBlock[mbPartIdxN[iLoop]]);
            mbPartIdxN[iLoop] = (16 / pNeighbourMb->MbPartWidth) * ((yW / pNeighbourMb->MbPartHeight) + (xW / pNeighbourMb->MbPartWidth));
            if (NULL != subMbPartIdxN)
            {
                if ((VIDEO_H264_MB_TYPE_P_8x8 != pNeighbourMb->type) &&
                    (VIDEO_H264_MB_TYPE_P_8x8ref0 != pNeighbourMb->type) &&
                    (VIDEO_H264_MB_TYPE_B_8x8 != pNeighbourMb->type) &&
                    (VIDEO_H264_MB_TYPE_B_Skip != pNeighbourMb->type) &&
                    (VIDEO_H264_MB_TYPE_B_Direct_16x16 != pNeighbourMb->type)
                )
                { subMbPartIdxN[iLoop] = 0; }
                else if ((VIDEO_H264_MB_TYPE_B_Skip != pNeighbourMb->type) && (VIDEO_H264_MB_TYPE_B_Direct_16x16 != pNeighbourMb->type))
                { subMbPartIdxN[iLoop] = 2 * ((yW % 8) / 4) + ((xW % 8) / 4); }
                else
                {
                    subMbPartIdxN[iLoop] =
                        (8 / pNeighbourMb->SubMbPartWidth[mbPartIdx]) *
                        (
                            ((yW % 8) / pNeighbourMb->SubMbPartHeight[mbPartIdx]) +
                            ((xW % 8) / pNeighbourMb->SubMbPartHeight[mbPartIdx])
                        );
                }
            }
        }
    }
}
