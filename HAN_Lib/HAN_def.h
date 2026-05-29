#ifndef HAN_DEF_H
#define HAN_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

// 错误表
typedef enum {
    RET_OK = 0,             // 函数执行正常
    NOT_ENOUGH_MEMORY,      // 内存不足
    /* FILE */
    OPEN_FILE_ERR,          // 打开文件出错，如果要知道具体是哪种错，需要系统提供的函数确定
    BLANK_LINE_EOF,         // 使用 GetStrLine 从文件读取一行数据时，如果读取的是文件尾的空行，会返回该值并提代一个空字符串。如果文件尾不是空行，会正常读取并返回 RET_OK
    /* HAN_data_structure */
    INDEX_OUT_OF_RANGE,     // 读取列表数据时提供的索引值大于列表长度
    END_OF_LIST,            // 使用 ForList 函数遍历列表时，遍历结束时会返回该值
    TREE_CMP_EQUAL,         // 向二叉树插入数据时，存在相等的数据

    KEY_NOT_EXIST,          // 读取字典数据时提供的键值不存在
    END_OF_DICT,            // 使用 ForDict 函数遍历字典时，遍历结束时会返回该值
    TITLE_EXIST,            // 标题已存在
    TITLE_NOT_EXIST,        // 标题不存在
    CONVERT_TYPE_ERR,       // 转换类型失败
    END_OF_DATA_SHEET,      // 使用 ForRow 函数遍历数据表时，遍历结束时会返回该值
    CSV_FORMAT_ERR,         // CSV 数据的格式不正确
    EMPTY_DATA_SHEET,       // 空白表格
    INCORRECT_TYPE,         // 类型不正确
    /* HAN_Hex */
    HEX_FILE_FORMAT_ERR,    // Hex 文件格式错误
    SREC_FILE_FORMAT_ERR,   // Srec 文件格式错误
    /* HAN_windows */
    CREATE_COM_FILE_ERR,    // 创建串口文件错误，可能是串口名错误
    SET_COM_QUEUE_ERR,      // 设置缓冲区大小失败，缓冲区大小应是偶数
    SET_COM_TIME_OUT_ERR,   // 设置超时等待失败
    SET_COM_STATE_ERR,      // 设置串口参数失败，请检查波特率，字节长度，校验位，停止位是否是合法参数
    READ_COM_ERR,           // 读取串口失败，请检查串口是否插紧
    /* multi list */
    HML_EMPTY,              // 多级列表为空
    HML_ID_OUT_OF_RANGE,    // ID 超出范围
    HML_NO_CHILD,           // 当前操作中的多级列表无子节点
    /* gdi */
    HCOM_PORT_ID_NOT_EXIST, // 端口号不存在
    HCOM_NOT_OPEN,          // 端口未打开
    HCOM_WRITING,           // 端口正在发送数据
    /* plot */
    PLOT_VAR_NOT_EXIST,     // 变量不存在
    CREATE_OBJECT_ERR,      // 创建对象失败
    CREATE_PEN_ERR,         // 创建画笔失败
    CREATE_BRUSH_ERR,       // 创建画刷失败
    CREATE_FONT_ERR,        // 创建字体失败
    PLOT_INVALID_HPOINT,    // 非法的点句柄
    HBC_VAL_NEG_ERR,        // 非普通条形图有负值
    HPL_VAR_HPOS_EXIST,     // 给折线图添加变量时指定位置为顶部或底部的水平位置时，该位置已存在变量，不可继续添加
    HPL_RCINVH_ERR,         // 设置绘图区留白高度的值错误
    HST_VAR_POS_ERR,        // 给散点图导入数据时变量位置不匹配
    HST_MARK_DIS_ERR,       // 散点图标线间隔错误（必须大于 0 ）
} HANERROR;

#define ArrLen(arr)                             (sizeof(arr) / sizeof((arr)[0]))  // 数组元素个数
#define StructMemberOffset(tagStruct, member)   ((size_t)(&(((tagStruct*)0)->member)))
#define StructMemberSize(tagStruct, member)     ((size_t)sizeof(((tagStruct*)0)->member))

#define HANCPRIVATE
#define HANCPUBLIC

typedef int             HANINT;
typedef float           HANFLOAT;
typedef double          HANDOUBLE;

#ifdef __cplusplus
}
#endif

#endif
