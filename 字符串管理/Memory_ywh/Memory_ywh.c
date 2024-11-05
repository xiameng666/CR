// Memory_ywh.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。

#include <stdio.h>
#include "Memory_ywh.h"


#define MAX_FILE_SIZE 1000  // 最大存储空间大小
#define MAX_STRING_SIZE 100    // 单个字符串最大长度
#define MAX_STRING_COUNT 100   // 最大字符串存储数量

/**
 * @brief 结构体用于管理内存块
 */
typedef struct 
{
    char* cAddress;  // 内存块的起始地址
    int nSize;       // 内存块的大小
    int nIsFree;     // 标记该内存块是否空闲，1 表示空闲，0 表示已占用
} MemoryBlock;

/**
 * @brief 结构体用于存储字符串数据
 */
typedef struct 
{
    char cContent[MAX_STRING_SIZE];  // 字符串内容
    int nIndex;                       // 字符串索引 编号
} StrData;

// 全局变量
static char g_memorySpace[MAX_FILE_SIZE];  // 内存空间
static MemoryBlock g_blocks[MAX_STRING_COUNT];  // 内存块数组
static StrData g_str[MAX_STRING_COUNT];  // 字符串数据数组
static int g_numberOfStrings = 0;  // 当前字符串数量
static size_t g_usedMemory = 0;    // 修改为 size_t 类型


// 自定义的内存分配函数
void* Mymalloc(size_t nSize)
{
    // 检查是否有足够的可用空间
    if (g_usedMemory + nSize > MAX_FILE_SIZE)
    {
        printf("没有足够的可用空间\n");
        return NULL;
    }

    // 寻找第一个空闲的内存块
    int i;
    for (i = 0; i < MAX_STRING_COUNT; i++)
    {
        if (g_blocks[i].nIsFree == 1 && g_blocks[i].nSize >= nSize)
        {
            break;
        }
    }

    // 如果找到空闲的内存块
    if (i < MAX_STRING_COUNT)
    {
        // 分配内存块
        g_blocks[i].cAddress = g_memorySpace + g_usedMemory;
        g_blocks[i].nSize = nSize;
        g_blocks[i].nIsFree = 0;
        g_usedMemory += nSize;
        return g_blocks[i].cAddress;
    }
    else
    {
        // 没有找到空闲的内存块
        printf("没有找到足够大的空闲内存块\n");
        return NULL;
    }
}

// 自定义的内存释放函数
void Myfree(void* ptr)
{
    // 检查指针是否为空
    if (ptr == NULL)
    {
        return;
    }

    // 找到内存块
    int i;
    for (i = 0; i < MAX_STRING_COUNT; i++)
    {
        if (g_blocks[i].cAddress == ptr && g_blocks[i].nIsFree == 0)
        {
            break;
        }
    }

    // 如果找到内存块
    if (i < MAX_STRING_COUNT)
    {
        // 释放内存块
        g_blocks[i].nIsFree = 1;
    }
    else
    {
        // 没有找到内存块
        printf("没有找到要释放的内存块\n");
    }
}

// 自定义的strnlen
size_t MyStrnlen(const char* str, size_t max_len) {
    size_t len = 0;
    while (len < max_len && str[len] != '\0') {
        len++;
    }
    return len;
}

int MyStrncpy_s(char* dest, size_t dest_size, const char* src, size_t count) {
    size_t i;

    if (dest == NULL || src == NULL || dest_size == 0) {
        return -1; // 返回错误
    }

    for (i = 0; i < count && i < dest_size - 1; i++) {
        dest[i] = src[i];
        if (src[i] == '\0') {
            break;
        }
    }

    // 确保字符串以 '\0' 结尾
    dest[i] = '\0';
    return 0; // 返回成功
}

/**
 * @brief 初始化相关数据结构
 */
static void Initialize(void)
{
    // 手动初始化 g_memorySpace 数组为 0
    for (int i = 0; i < MAX_FILE_SIZE; i++)
    {
        g_memorySpace[i] = 0;
    }

    // 手动初始化 g_blocks 数组
    for (int i = 0; i < MAX_STRING_SIZE; i++)
    {
        g_blocks[i].cAddress = NULL;
        g_blocks[i].nSize = 0;
        g_blocks[i].nIsFree = 1;  // 初始化为未使用状态
    }

    // 手动初始化 g_str 数组
    for (int i = 0; i < MAX_STRING_SIZE; i++)
    {
        g_str[i].cContent[0] = '\0';  // 初始化为空字符串
        g_str[i].nIndex = -1;
    }
}

void AddString(void)
{
    printf("请添加字符串：");
    //检查字符串数量，如果超过限制数量报错
    if (g_numberOfStrings >= MAX_STRING_COUNT)
    {
        printf("字符串数量已达上限\n");
        return;
    }

    char temp[MAX_STRING_SIZE + 1] = { 0 }; //变量temp临时储存输入的字符串
  
    scanf_s("%100s", temp, (unsigned)(sizeof(temp) / sizeof(temp[0])));
 
    size_t length = MyStrnlen(temp, MAX_STRING_SIZE + 1);
    if (length >= MAX_STRING_SIZE)
    {
        printf("字符串长度超过最大限度\n");

        // 清空输入缓冲区，以防止影响后续输入
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF);
        
        return;
    }

    if (g_usedMemory + length > MAX_FILE_SIZE)
    {
        printf("没有足够的可用空间\n");
    }

    //if ((g_usedMemory > 0) && (g_memorySpace[g_usedMemory - 1] != '\0')) 
    //{
    //    // 确保之前的字符串以\0终止
    //    g_memorySpace[g_usedMemory] = '\0';
    //    g_usedMemory++;
    //}

    //g_memorySpace + g_usedMemory指向未被使用的数组起始位置
    strncpy_s(g_memorySpace + g_usedMemory, MAX_FILE_SIZE - g_usedMemory, temp, length);

    //设置存储数据块信息
    g_blocks[g_numberOfStrings].nIsFree = 0;
    g_blocks[g_numberOfStrings].nSize = (int)length;
    g_blocks[g_numberOfStrings].cAddress = g_memorySpace + g_usedMemory;

    //设置字符串信息
    g_str[g_numberOfStrings].nIndex = g_numberOfStrings;
    strncpy_s(g_str[g_numberOfStrings].cContent, MAX_STRING_SIZE, temp, length);

    //更新内存使用情况
    g_usedMemory += length;
    g_numberOfStrings++;

    printf("字符串添加成功。\n");
   
}

void DisplayAllStrings(void)
{
    if (g_numberOfStrings == 0)
    {
        printf("没有存储的字符串。\n");
        return;
    }

    printf("所有存储的字符串:\n");
    for (int i = 0; i < g_numberOfStrings; i++)
    {
        printf("编号 %d: %s\n", g_str[i].nIndex, g_str[i].cContent);
    }
}

void DeleteString(void)
{
    printf("请输入要删除的索引: ");
    int IndexToDelete;
    scanf_s("%d", &IndexToDelete);
    if (IndexToDelete < 0 || IndexToDelete > g_numberOfStrings)
    {
        printf("请输入正确字符串编号\n");
        return;
    }

    printf("删除编号为 %d 的字符串\n", IndexToDelete);

    // 释放内存块
    g_blocks[IndexToDelete].nIsFree = 1;

    // 清空字符串数据
    g_str[IndexToDelete].cContent[0] = '\0';
    g_str[IndexToDelete].nIndex = -1;

    //for (int i = IndexToDelete; i < g_numberOfStrings - 1; i++)
    //{
    //    strcpy(g_str[i].cContent, g_str[i + 1].cContent);
    //    g_str[i].nIndex = i;
    //}

    //g_str[g_numberOfStrings - 1].cContent[0] = '\0';
    //g_str[g_numberOfStrings - 1].nIndex = -1;

    //g_numberOfStrings--;  // 更新字符串数量
    printf("删除成功\n");
}

void RefactorMemory(void)
{
    size_t NewUsedMemory = 0;
    int nNewIndex = 0; // 用于重新编号

    for (int i = 0; i < g_numberOfStrings; i++)
    {
        if (g_blocks[i].nIsFree == 0)  // 只处理已占用的内存块
        {
            char* currentAddress = g_blocks[i].cAddress;
            int nSize = g_blocks[i].nSize;

            // 如果当前内存块不在紧凑区域
            if (currentAddress != g_memorySpace + NewUsedMemory)
            {
                // 逐个字符复制到新位置
                for (int j = 0; j < nSize; j++)
                {
                    g_memorySpace[NewUsedMemory + j] = currentAddress[j];
                }

                // 更新内存块地址
                g_blocks[i].cAddress = g_memorySpace + NewUsedMemory;
            }

            // 更新内存使用量
            NewUsedMemory += nSize;

            // 更新编号
            g_str[nNewIndex].nIndex = nNewIndex;
            g_blocks[nNewIndex] = g_blocks[i];
            strncpy_s(g_str[nNewIndex].cContent, MAX_STRING_SIZE, g_str[i].cContent, nSize);
            nNewIndex++;
        }
    }

    // 清空未使用的内存空间
    for (size_t i = NewUsedMemory; i < MAX_FILE_SIZE; i++)
    {
        g_memorySpace[i] = 0;
    }

    // 更新全局内存使用情况和字符串数量
    g_usedMemory = NewUsedMemory;
    g_numberOfStrings = nNewIndex;

    // 将剩余的块标记为空闲
    for (int i = nNewIndex; i < MAX_STRING_COUNT; i++)
    {
        g_blocks[i].cAddress = NULL;
        g_blocks[i].nSize = 0;
        g_blocks[i].nIsFree = 1;
        g_str[i].cContent[0] = '\0';
        g_str[i].nIndex = -1;
    }

    printf("内存碎片整理和字符串编号重新排列完成。\n");
}

void ModifyString(void)
{
    int modifyIndex;
    printf("请输入要修改的字符串编号: ");
    scanf_s("%d", &modifyIndex);
    if (modifyIndex < 0 || modifyIndex > g_numberOfStrings)
    {
        printf("输入错误或编号超出范围。\n");
    }
    else 
    {
        char newContent[MAX_STRING_SIZE];
        printf("请输入新的字符串内容: ");
        scanf_s(" %[^\n]", newContent, (unsigned)(sizeof(newContent) / sizeof(newContent[0])));

        size_t newContentLength = strlen(newContent);

        if (newContentLength >= MAX_STRING_SIZE)
        {
            printf("单个字符串长度超过最大限度\n");
            return;
        }

        // 获取旧字符串的内存块信息
        size_t oldContentLength = g_blocks[modifyIndex].nSize;
        char* oldAddress = g_blocks[modifyIndex].cAddress;

        if (newContentLength > oldContentLength)
        {
            // 检查是否有足够的内存空间来存储新字符串
            if (g_usedMemory + newContentLength > MAX_FILE_SIZE)
            {
                printf("没有足够的可用空间存储新的字符串。\n");
                return;
            }

            // 将旧字符串的内存块标记为已释放
            g_blocks[modifyIndex].nIsFree = 1;

            // 在内存中寻找新的空闲位置存储新字符串
            strncpy_s(g_memorySpace + g_usedMemory, MAX_FILE_SIZE - g_usedMemory, newContent, newContentLength);

            // 更新内存块信息和字符串内容
            g_blocks[modifyIndex].cAddress = g_memorySpace + g_usedMemory;
            g_blocks[modifyIndex].nSize = (int)newContentLength;
            g_blocks[modifyIndex].nIsFree = 0;

            strncpy_s(g_str[modifyIndex].cContent, MAX_STRING_SIZE, newContent, newContentLength);

            g_usedMemory += newContentLength;
        }//if (newContentLength > oldContentLength)
        else
        {
            // 如果新字符串长度小于或等于旧字符串长度，则可以直接替换
            strncpy_s(oldAddress, oldContentLength, newContent, newContentLength);
            g_blocks[modifyIndex].nSize = (int)newContentLength;
            strncpy_s(g_str[modifyIndex].cContent, MAX_STRING_SIZE, newContent, newContentLength);
        }
        printf("字符串 %d 修改成功。\n", modifyIndex);
    }// if (modifyIndex < 0 || modifyIndex > g_numberOfStrings)else
}

void DisplayMemory()
{
    printf("存储空间:\n");
    char layout[MAX_FILE_SIZE];

    // 初始化 layout 数组为 'F'（空闲）
    for (int i = 0; i < MAX_FILE_SIZE; i++)
    {
        layout[i] = 'F';
    }

    // 根据 g_blocks 数组中的信息标记内存块
    for (int i = 0; i < g_numberOfStrings; i++)
    {
        if (g_blocks[i].nIsFree == 0)  // 只标记已占用的内存块
        {
            char* start = g_blocks[i].cAddress;
            int nSize = g_blocks[i].nSize;
            int offset = start - g_memorySpace; // 计算内存块起始地址相对于g_memorySpace的偏移量

            if (offset >= 0 && offset + nSize <= MAX_FILE_SIZE)
            {
                for (int j = 0; j < nSize; j++)
                {
                    layout[offset + j] = 'U'; // 标记内存块为已使用
                }
            }
            else
            {
                printf("警告:偏移量错误\n");
            }
        }
    }

    // 打印内存布局
    for (int i = 0; i < MAX_FILE_SIZE; i++)
    {
        printf("%c", layout[i]);
        if ((i + 1) % 50 == 0)  // 每50个字符换一行
        {
            printf("\n");
        }
    }
    printf("\n");

    // 调试输出
    printf("g_usedMemory = %zu\n", g_usedMemory);
    printf("MAX_FILE_SIZE = %d\n", MAX_FILE_SIZE);
}

void SearchByIndex(void);
void SearchByAddress(void);
void SearchByContent(void);

void SearchString(void)
{
    int searchOption;
    printf("\n");
    printf("请选择查询方式:\n");
    printf("1. 按编号查询\n");
    printf("2. 按地址查询\n");
    printf("3. 按内容查询\n");
    printf("4. 显示所有字符串\n");
    printf("请输入你的选择: ");

    scanf_s("%d", &searchOption);
    switch (searchOption)
    {
    case 1:
        SearchByIndex();
        break;
    case 2:
        SearchByAddress();
        break;
    case 3:
        SearchByContent();
        break;
    case 4:
        DisplayAllStrings();
        break;
    default:
        printf("无效的选项。\n");
        break;
    }
}

// 按编号查询
void SearchByIndex(void)
{
    int nIndex;
    printf("请输入要查询的编号: ");
    if (scanf_s("%d", &nIndex) != 1 || nIndex < 0 || nIndex > g_numberOfStrings)
    {
        printf("输入错误或编号超出范围。\n");
        return;
    }

    if (g_blocks[nIndex].nIsFree == 0)
    {
        printf("编号 %d: %s\n", g_str[nIndex].nIndex, g_str[nIndex].cContent);

    }
    else
    {
        printf("编号 %d 对应的字符串已删除。\n", nIndex);
    }
}

// 按地址查询
void SearchByAddress(void)
{
    void *cAddress;
    printf("请输入要查询的地址: ");
    if (scanf_s("%p", &cAddress) != 1)
    {
        printf("输入错误，请输入正确的地址。\n");
        return;
    }

    for (int i = 0; i < g_numberOfStrings; i++)
    {
        if (g_blocks[i].cAddress == cAddress && g_blocks[i].nIsFree == 0)
        {
            printf("找到字符串：编号 %d: %s\n", g_str[i].nIndex, g_str[i].cContent);
            printf("地址: %p, \n", (void*)g_blocks[i].cAddress);
            return;
        }
    }

    printf("未找到匹配的地址。\n");
}

// 按内容查询
void SearchByContent(void)
{
    char query[MAX_STRING_SIZE];
    printf("请输入要查询的内容: ");
    scanf_s(" %[^\n]", query, (unsigned)(sizeof(query) / sizeof(query[0])));

    int found = 0;
    for (int i = 0; i < g_numberOfStrings; i++)
    {
        if (g_blocks[i].nIsFree == 0 && strstr(g_str[i].cContent, query) != NULL)
        {
            printf("编号 %d: %s\n", g_str[i].nIndex, g_str[i].cContent);
            printf("地址: %p, \n", (void*)g_blocks[i].cAddress);
            found = 1;
        }
    }

    if (!found)
    {
        printf("未找到匹配的字符串。\n");
    }
}

void CountLetters(void)
{
    int nLetterCount[26] = { 0 };  // 用于存储每个字母的计数
    int nTotalLetters = 0;  // 总字母数

    // 遍历所有存储的字符串
    for (int i = 0; i < g_numberOfStrings; i++)
    {
        if (g_blocks[i].nIsFree == 0)  // 只处理未删除的字符串
        {
            for (int j = 0; g_str[i].cContent[j] != '\0'; j++)
            {
                char ch = g_str[i].cContent[j];

                // 将大写字母转换为小写字母
                if (ch >= 'A' && ch <= 'Z')
                {
                    ch = ch + ('a' - 'A');
                }

                if (ch >= 'a' && ch <= 'z')  // 统计小写字母
                {
                    nLetterCount[ch - 'a']++;
                    nTotalLetters++;
                }
            }
        }
    }

    // 输出统计结果
    printf("字母统计:\n");
    for (int i = 0; i < 26; i++)
    {
        if (nLetterCount[i] > 0)
        {
            double percentage = (double)nLetterCount[i] / nTotalLetters * 100.0;
            printf("%c: %d 次，占 %.2f%%\n", 'a' + i, nLetterCount[i], percentage);
        }
    }

    if (nTotalLetters == 0)
    {
        printf("没有字母被统计到。\n");
    }
}



static void Menu(void)
{
    int nChoice;



    // 第一次执行时显示菜单
    printf("字符串存储管理系统\n");
    printf("0. 显示所有字符串\n");
    printf("1. 添加字符串(限制长度为100)\n");
    printf("2. 删除字符串\n");
    printf("3. 修改字符串\n");
    printf("4. 查询字符串\n");
    printf("5. 字符统计\n");
    printf("6. 内存碎片整理\n");
    printf("7. 显示内存\n");
    printf("8. 退出\n");

    // 进入循环
    while (1)
    {   
        printf("指令: ");
        if (scanf_s("%d", &nChoice) != 1)
        {
            // 清空输入缓冲区以避免无限循环
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);

            printf("输入数据错误\n");
            continue; // 返回菜单，继续等待用户输入
        }

        switch (nChoice)
        {
        case 0:
            DisplayAllStrings();
            break;
        case 1:
            AddString();
            break;
        case 2:
            DeleteString();
            break;
        case 3:
            ModifyString();
            break;
        case 4:
            SearchString();
            break;
        case 5:
            CountLetters();
            break;
        case 6:
            RefactorMemory();
            break;
        case 7:
            DisplayMemory();
            break;
        case 8:
            printf("退出程序。\n");
            return; // 退出循环并结束程序
        default:
            printf("请输入正确的数字。\n");
            break;
        }

        // 显示分割线和菜单内容
        printf("----------------\n");
        printf("字符串存储管理系统\n");
        printf("0. 显示所有字符串\n");
        printf("1. 添加字符串(限制长度为100)\n");
        printf("2. 删除字符串\n");
        printf("3. 修改字符串\n");
        printf("4. 查询字符串\n");
        printf("5. 字符统计\n");
        printf("6. 内存碎片整理\n");
        printf("7. 显示内存\n");
        printf("8. 退出\n");
        printf("----------------\n");
    }
}

int main(void)
{
    Initialize();  // 初始化数组
    Menu();  // 显示菜单并执行功能
    return 0;
}
