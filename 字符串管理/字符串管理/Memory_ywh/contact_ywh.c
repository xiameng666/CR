// contact_ywh.c : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#define MAX_NAME_LENGTH 10
#define MAX_PHONE_LENGTH 15
#define MAX_ADDRESS_LENGTH 100
#define MAX_CONTACTS 100

#define MAX_FILE_SIZE 5*1024*1024  // 最大存储空间大小5MB
#define MAX_STRING_SIZE 100    // 单个字符串最大长度
#define MAX_STRING_COUNT 100   // 最大字符串存储数量
#define DATA_FILE "contacts.txt"

#define SAFE_CLOSE(fp) if(fp){fclose(fp);fp=NULL;}

typedef struct MemoryBlock
{ 
    char* cAddress;  // 内存块的起始地址
    int nSize;       // 内存块的大小
    int nIsFree;     // 标记该内存块是否空闲，1 表示空闲，0 表示已占用
} MemoryBlock;

typedef struct Contact
{
    int id; // 联系人编号
    int size;
    char name[MAX_NAME_LENGTH];
    char phone[MAX_PHONE_LENGTH];
    char address[MAX_ADDRESS_LENGTH];
    int isDeleted; // 标记是否已删除
} Contact;


/**
 * @brief 结构体用于存储字符串数据
 */
typedef struct StrData
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

static Contact g_contacts[MAX_CONTACTS];//存储联系人
static int g_contactCount = 0;//联系人计数

FILE* g_file = NULL;

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

    // 初始化联系人数据
    for (int i = 0; i < MAX_CONTACTS; i++) {
        g_contacts[i].isDeleted = 1;  // 初始化为已删除状态
    }

    //预分配5MB空间 
    errno_t err = fopen_s(&g_file, DATA_FILE, "wb+"); // Open the file in binary read/write mode

    if (err == 0 && g_file) { // Check if the file was opened successfully
        fseek(g_file, MAX_FILE_SIZE - 1, SEEK_SET); // Move to the desired position
        fputc('\0', g_file); // Write a null byte
        fflush(g_file); // Flush the buffer
        fclose(g_file); // Close the file
    }
}



void ShowAllContacts(void) 
{
    int contactFound = 0;

    printf("所有联系人:\n");
    for (int i = 0; i < g_contactCount; i++) 
    {
        if (g_contacts[i].isDeleted == 0)
        {
            printf("ID: %d\n", g_contacts[i].id);
            printf("姓名: %s", g_contacts[i].name); // 姓名在一行
            printf("电话: %s", g_contacts[i].phone); // 电话和地址在下一行
            printf("地址: %s\n", g_contacts[i].address);
            contactFound = 1;
        }
    }

    if (!contactFound) 
    {
        printf("没有找到任何联系人。\n");
    }
}

void AddContact(void)
{
    if (g_contactCount >= MAX_CONTACTS)
    {
        printf("联系人数量已达上限\n");
        return;
    }

    Contact newContact;
    newContact.id = g_contactCount + 1;
    printf("请输入姓名: ");
    fgets(newContact.name, MAX_NAME_LENGTH, stdin);
    printf("请输入电话: ");
    fgets(newContact.phone, MAX_PHONE_LENGTH, stdin);
    printf("请输入地址: ");
    fgets(newContact.address, MAX_ADDRESS_LENGTH, stdin);

    newContact.isDeleted = 0; // 未删除
    g_contacts[g_contactCount] = newContact;

    // 更新文件中的数据
    FILE* g_file = fopen(DATA_FILE, "rb+");
    if (g_file == NULL)
    {
        printf("无法打开文件\n");
        return;
    }
    else
    {
        // 定位到文件中对应的位置
        fseek(g_file, g_contactCount * sizeof(Contact), SEEK_SET);
        fwrite(&newContact, sizeof(Contact), 1, g_file); // 写入单个Contact结构体
        fclose(g_file);
        printf("添加成功\n");
    }

    g_contactCount++; // 更新联系人数量
}


void DelectContact()
{
    int nDeletedId = 0;
    printf("输入删除的联系人编号：");
    scanf_s("%d", &nDeletedId);
    for (int i = 0; i < g_contactCount; i++)
    {
        if (g_contacts[i].isDeleted == 0 && g_contacts[i].id == nDeletedId)
        {
            //将删除标识位置1
            g_contacts[i].isDeleted = 1;

            FILE* g_file = fopen(DATA_FILE, "rb+");
            if (g_file)
            {
                fseek(g_file, i * sizeof(Contact), SEEK_SET);
                fwrite(&g_contacts[i], sizeof(Contact), 1, g_file);
                fclose(g_file);
            }
            printf("联系人ID %d 已删除\n", nDeletedId);

            return;  // 退出DelectContact函数
        }
    }
    printf("未找到 ID 为 %d 的联系人\n", nDeletedId);

}

void ModifyContact()
{
    int nModifyId = 0;
    printf("输入修改的联系人编号：");
    scanf_s("%d", &nModifyId);
    getchar();
    for (int i = 0; i < g_contactCount; i++) {
        if (g_contacts[i].id == nModifyId && g_contacts[i].isDeleted == 0) {
            printf("修改联系人 %d 信息\n", nModifyId);
            printf("请输入新姓名: ");
            fgets(g_contacts[i].name, MAX_NAME_LENGTH, stdin);
            printf("请输入新电话: ");
            fgets(g_contacts[i].phone, MAX_PHONE_LENGTH, stdin);
            printf("请输入新地址: ");
            fgets(g_contacts[i].address, MAX_ADDRESS_LENGTH, stdin);

            FILE* g_file = fopen(DATA_FILE, "rb+");
            if (g_file) {
                fseek(g_file, i * sizeof(Contact), SEEK_SET);
                fwrite(&g_contacts[i], sizeof(Contact), 1, g_file);
                fclose(g_file);
            }
            printf("联系人已修改\n");

            return;
        }
    }
    printf("未找到该联系人\n");
}
void SearchById(int id)
{
    for (int i = 0; i < g_contactCount; i++)
    {
        if (g_contacts[i].isDeleted == 0 && g_contacts[i].id == id)
        {
            printf("ID: %d\n姓名: %s\n电话: %s\n地址: %s\n",
                g_contacts[i].id, g_contacts[i].name,
                g_contacts[i].phone, g_contacts[i].address);
            return;
        }
    }
    printf("未找到 ID 为 %d 的联系人\n", id);
}

void SearchByName(char* name)
{
    for (int i = 0; i < g_contactCount; i++)
    {
        if (g_contacts[i].isDeleted == 0 && strstr(g_contacts[i].name, name) != NULL)
        {
            printf("ID: %d\n姓名: %s\n电话: %s\n地址: %s\n",
                g_contacts[i].id, g_contacts[i].name,
                g_contacts[i].phone, g_contacts[i].address);
            return;
        }
    }
    printf("未找到包含名字 '%s' 的联系人\n", name);
}

void SearchByPhone(char* phone)
{
    for (int i = 0; i < g_contactCount; i++) {
        if (g_contacts[i].isDeleted == 0 && strcmp(g_contacts[i].phone, phone) == 0) {
            printf("ID: %d\n姓名: %s\n电话: %s\n地址: %s\n",
                g_contacts[i].id, g_contacts[i].name,
                g_contacts[i].phone, g_contacts[i].address);
            return;
        }
    }
    printf("未找到电话为 '%s' 的联系人\n", phone);
}

void SearchContact() 
{
    int choice;
    printf("请选择查询方式:\n");
    printf("1. 按编号查询\n");
    printf("2. 按名字查询\n");
    printf("3. 按电话号码查询\n");
    printf("请输入选择: ");
    scanf_s("%d", &choice);
    getchar(); // 清空输入缓冲区

    switch (choice) 
    {
    case 1: 
    {
        int id;
        printf("请输入联系人编号: ");
        scanf_s("%d",&id);
        getchar();  // 清除输入缓冲区
        SearchById(id);
        break;
    }
    case 2:
    {
        char name[MAX_NAME_LENGTH];
        printf("请输入联系人名字或部分名字: ");
        fgets(name, MAX_NAME_LENGTH, stdin);
        name[strcspn(name, "\n")] = '\0'; // 去掉换行符
        SearchByName(name);
        break;
    }
    case 3: 
    {
        char phone[MAX_PHONE_LENGTH];
        printf("请输入联系人电话号码: ");
        fgets(phone, MAX_PHONE_LENGTH, stdin);
        phone[strcspn(phone, "\n")] = '\0'; // 去掉换行符
        SearchByPhone(phone);
        break;
    }
    default:
        printf("无效的选项\n");
        break;
    }
}

void RefactorContacts(void) {
    FILE* file = fopen(DATA_FILE, "rb");
    if (!file) {
        printf("无法打开文件\n");
        return;
    }

    Contact tempContacts[MAX_CONTACTS];
    int usedCount = 0;

    // 读取联系人并收集有效数据
    while (fread(&tempContacts[usedCount], sizeof(Contact), 1, file) == 1) {
        if (tempContacts[usedCount].isDeleted == 0) {
            usedCount++;
        }
    }
    fclose(file);

    // 重新写入有效联系人并更新编号
    file = fopen(DATA_FILE, "wb");
    if (!file) {
        printf("无法打开文件\n");
        return;
    }

    for (int i = 0; i < usedCount; i++) {
        tempContacts[i].id = i + 1; // 重新分配编号
        if (fwrite(&tempContacts[i], sizeof(Contact), 1, file) != 1) {
            printf("写入联系人时出错\n");
        }
    }

    fclose(file);
    printf("碎片整理完成，联系人编号已更新。\n");
}

void DisplayMemory(void) {
    FILE* file = fopen(DATA_FILE, "rb");
    if (!file) {
        printf("无法打开文件\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file); // 获取文件大小
    fseek(file, 0, SEEK_SET); // 回到文件开头

    char* storageMap = malloc(fileSize + 1); // 为存储图分配内存
    if (!storageMap) {
        printf("内存分配失败\n");
        fclose(file);
        return;
    }

    // 初始化存储图
    for (long i = 0; i < fileSize; i++) {
        Contact contact;
        if (fread(&contact, sizeof(Contact), 1, file) == 1) {
            storageMap[i] = (contact.isDeleted == 0) ? 'U' : 'F'; // U: 已使用, F: 空闲
        }
        else {
            storageMap[i] = 'F'; // 超出文件范围标记为空闲
        }
    }
    storageMap[fileSize] = '\0'; // 结束字符串

    printf("存储资源分布: %s\n", storageMap);

    free(storageMap);
    fclose(file);
}

void CountLetters(void)
{
    int nLetterCount[26] = { 0 };  // 用于存储每个字母的计数
    int nTotalLetters = 0;  // 总字母数

    FILE* g_file = fopen(DATA_FILE, "rb");
    if (g_file == NULL) 
    {
        printf("无法打开文件\n");
        return;
    }

    Contact contact;

    //从文件 file 中不断读取数据，并将其存储到 contact 结构体中，直到没有更多的数据可读
    while (fread(&contact, sizeof(Contact), 1, g_file));//fread 返回成功读取的元素个数,
    {
        if (contact.isDeleted == 0)
        {
            // 统计姓名部分
            for (int j = 0; contact.name[j] != '\0'; j++)
            {
                char ch = tolower(contact.name[j]);  // 转换为小写
                if (isalpha(ch)) 
                {  // 只统计字母
                    nLetterCount[ch - 'a']++;
                    nTotalLetters++;
                }
            }

            // 统计电话部分
            for (int j = 0; contact.phone[j] != '\0'; j++)
            {
                char ch = tolower(contact.phone[j]);  // 转换为小写
                if (isalpha(ch)) 
                { 
                    nLetterCount[ch - 'a']++;
                    nTotalLetters++;
                }
            }

            // 统计地址部分
            for (int j = 0; contact.address[j] != '\0'; j++)
            {
                char ch = tolower(contact.address[j]);  // 转换为小写
                if (isalpha(ch)) 
                { 
                    nLetterCount[ch - 'a']++;
                    nTotalLetters++;
                }
            }
        }
    } 
    fclose(g_file);
    
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
    printf("文件管理系统\n");

    while (1)
    {
        printf("---------------------\n");
        printf("0. 显示所有联系人\n");
        printf("1. 增加联系人\n");
        printf("2. 删除联系人\n");
        printf("3. 修改联系人\n");
        printf("4. 查询联系人\n");
        printf("5. 统计字母出现次数\n");
        printf("6. 显示存储信息\n");
        printf("7. 整理联系人信息\n");
        printf("8. 退出\n");
        printf("---------------------\n");
        printf("指令: ");

        if (scanf_s("%d", &nChoice) != 1) // 如果输入不是整数
        {
            // 清空输入缓冲区以避免无限循环
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);

            printf("输入数据错误，请输入一个有效的数字选项。\n");
            continue; // 返回菜单，继续等待用户输入
        }
        getchar(); // 清除缓冲区中的换行符

        switch (nChoice) {
        case 0:
            ShowAllContacts(); // 显示所有联系人
            break;
        case 1:
            AddContact();
            break;
        case 2:
            DelectContact();
            break;
        case 3:
            ModifyContact();
            break;
        case 4:
            SearchContact();
            break;
        case 5:
            CountLetters();
            break;
        case 6:
            DisplayMemory();
            break;
        case 7:
            RefactorContacts();
            break;
        case 8:
            printf("退出\n");
            return;
        default:
            printf("无效选项，请重新选择。\n");
        }
    }
}

int main(void)
{
    Initialize();  // 初始化数组
    Menu();  // 显示菜单并执行功能
    return 0;
}
