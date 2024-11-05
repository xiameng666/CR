#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>

#define DATA_FILE "contacts.bin"
#define META_STORAGE (5 * 1024 * 1024) // 元结构体区域，固定5MB大小  5MB/32B=163840条记录


//32字节
typedef struct MetaStructs {
    unsigned long id;           // ID
    unsigned long start;          // 记录在文件中的开始位置
    unsigned long end;            // 记录在文件中的结束位置
    unsigned long flagOffset;   // 删除标志位的相对偏移量
    unsigned long idOffset;     // ID相对偏移量
    unsigned long nameOffset;     // 姓名的相对偏移量
    unsigned long phoneOffset;    // 电话的相对偏移量
    unsigned long addressOffset;  // 地址的相对偏移量
} MetaStructs;

typedef struct Contact {
    char flag;                  // 删除标志
    unsigned short id;          // 联系人ID
    char* name;                 // 姓名
    char* phone;                // 电话
    char* address;              // 地址
} Contact;

// 初始化文件
void initializeFile() 
{
    FILE* file = fopen(DATA_FILE, "wb+");
    if (file == NULL) 
    {
        perror("Failed");
        exit(EXIT_FAILURE);
    }

    char zero = 0; // 用于填充的零字节
    fseek(file, META_STORAGE - 1, SEEK_SET); // 移动到5MB
    fputc(zero, file); // 创建5MB的文件
    fclose(file);
    printf("初始化完成，元数据区大小为5MB。\n");
}

// 获取当前最大 ID
unsigned short getMaxId() {
    FILE* file = fopen(DATA_FILE, "rb+");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    unsigned short maxId = 0;
    MetaStructs meta;

    for (unsigned short i = 0; i < (META_STORAGE / sizeof(MetaStructs)); i++) 
    {
        fseek(file, i * sizeof(MetaStructs), SEEK_SET); // 定位到当前元结构体位置
        size_t read_size = fread(&meta, sizeof(MetaStructs), 1, file); // 读取元结构体

        // 检查是否成功读取
        if (read_size != 1) 
        {
            printf("id读取失败");
            break; // 如果没有读取到有效数据，跳出循环
        }

        // 如果 ID 为 0，直接返回当前最大 ID
        if (meta.id == 0) 
        {
            fclose(file);
            return maxId; // 返回已经获取的最大 ID
        }

        // 更新最大 ID
        if (meta.id > maxId) 
        {
            maxId = meta.id; // 更新最大 ID
        }
    }

    fclose(file);
    return maxId;
}

// 清除输入缓冲区
void clearInputBuffer() 
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// 安全读取输入
void safeFgets(char* buffer, size_t size) 
{
    while (1) 
    {
        if (fgets(buffer, size, stdin) != NULL) 
        {
            size_t len = strcspn(buffer, "\n");
            if (buffer[len] == '\n') {
                buffer[len] = '\0'; // 去掉换行符
                return;
            }
            else 
            {
                clearInputBuffer(); // 清除缓冲区
                printf("输入超过了 %zu 字节，请重新输入: ", size - 1);
            }
        }
        else 
        {
            printf("输入读取失败，请重新输入: ");
        }
    }
}

// 添加联系人
void addContact() 
{
    FILE* file = fopen(DATA_FILE, "r+b");
    if (file == NULL) 
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    Contact contact;
    contact.flag = 0; // 自动设置为0

    unsigned short temp_id = getMaxId();
    contact.id = temp_id + 1; // 设置 ID

    char buffer[101]; // 100字节 + 1字节用于存放终止符

    // 输入姓名
    printf("请输入联系人姓名 (最多100字节): ");
    safeFgets(buffer, sizeof(buffer));
    contact.name = _strdup(buffer);

    // 输入电话
    printf("请输入联系人电话 (最多100字节): ");
    safeFgets(buffer, sizeof(buffer));
    contact.phone = _strdup(buffer);

    // 输入地址
    printf("请输入联系人地址 (最多100字节): ");
    safeFgets(buffer, sizeof(buffer));
    contact.address = _strdup(buffer);

    // 定位到文件末尾并写入联系人数据
    fseek(file, 0, SEEK_END);
    long contact_start = ftell(file); // 获取当前联系人开始位置

    fwrite(&contact.flag, sizeof(char), 1, file);
    fwrite(&contact.id, sizeof(unsigned short), 1, file);
    fwrite(contact.name, sizeof(char), strlen(contact.name) + 1, file);
    fwrite(contact.phone, sizeof(char), strlen(contact.phone) + 1, file);
    fwrite(contact.address, sizeof(char), strlen(contact.address) + 1, file);

    long contact_end = ftell(file); // 获取当前联系人结束位置

    // 更新元结构体
    MetaStructs meta;
    meta.id = contact.id; // 设置 ID
    meta.start = contact_start; // 设置开始位置
    meta.end = contact_end; // 设置结束位置
    meta.flagOffset = 0;
    meta.idOffset = sizeof(char);
    meta.nameOffset = sizeof(char) + sizeof(unsigned short);
    meta.phoneOffset = meta.nameOffset + strlen(contact.name) + 1;
    meta.addressOffset = meta.phoneOffset + strlen(contact.phone) + 1;

    // 计算元结构体在文件中的位置
    fseek(file, (meta.id - 1) * sizeof(MetaStructs), SEEK_SET);
    fwrite(&meta, sizeof(MetaStructs), 1, file); // 更新元结构体

    free(contact.name);
    free(contact.phone);
    free(contact.address);
    fclose(file);
}

// 删除联系人
void deleteContact() 
{
    unsigned short idToDelete;
    printf("请输入要删除的联系人 ID: ");
    if (scanf("%hu", &idToDelete) != 1) 
    {
        printf("输入格式不正确，请输入一个有效的 ID。\n");
        clearInputBuffer();
        return;
    }
    getchar(); // 清除换行符

    FILE* file = fopen(DATA_FILE, "r+b");
    if (file == NULL) 
    {
        perror("Failed to open file");
        return;
    }

    MetaStructs meta;
    size_t read_size;

    // 查找对应 ID 的元结构体
    fseek(file, (idToDelete - 1) * sizeof(MetaStructs), SEEK_SET);
    read_size = fread(&meta, sizeof(MetaStructs), 1, file);

    // 检查是否成功读取
    if (read_size != 1 || meta.id == 0) 
    {
        printf("未找到 ID 为 %hu 的联系人。\n", idToDelete);
        fclose(file);
        return;
    }

    // 将 flag 设置为 1，表示删除
    // 计算具体数据的地址
    fseek(file, meta.start + meta.flagOffset, SEEK_SET);
    char delete_flag = 1; // 设置为删除标志
    fwrite(&delete_flag, sizeof(char), 1, file); // 更新 flag

    printf("联系人 ID %hu 已被删除。\n", idToDelete);
    fclose(file);
}

// 显示所有联系人
void readContacts() 
{
    FILE* file = fopen(DATA_FILE, "rb");
    if (file == NULL) 
    {
        perror("Failed to open file");
        return;
    }

    MetaStructs meta;
    size_t readSize;

    printf("所有联系人:\n");

    // 遍历元结构体区域
    for (unsigned short i = 0; i < (META_STORAGE / sizeof(MetaStructs)); i++) 
    {
        fseek(file, i * sizeof(MetaStructs), SEEK_SET); // 定位到当前元结构体位置
        readSize = fread(&meta, sizeof(MetaStructs), 1, file); // 读取元结构体

        // 检查是否成功读取
        if (readSize != 1 || meta.id == 0) {
            break; // 如果没有读取到有效数据，跳过
        }

        size_t idlength = meta.nameOffset - meta.idOffset;
        size_t namelength = meta.phoneOffset - meta.nameOffset;
        size_t phonelength = meta.addressOffset - meta.phoneOffset;
        size_t addresslength = meta.end - meta.addressOffset;
      
        // 读取联系人信息
        char flag;
        fseek(file, meta.start, SEEK_SET); // 定位到联系人数据开始位置
        fread(&flag, sizeof(char), 1, file); // 读取 flag

        // 检查 flag 是否为删除标志
        if (flag == 1) {
            continue; // 已删除，跳过
        }

        unsigned short id;
        fread(&id, sizeof(char), idlength, file); // 读取 ID

        // 读取姓名
        char name[256];
        fread(name, sizeof(char), namelength, file);

        // 读取电话
        char phone[256];
        fread(phone, sizeof(char), phonelength, file);

        // 读取地址
        char address[256];
        fread(address, sizeof(char), addresslength, file);

        // 打印联系人信息
        printf("ID: %hu, 姓名: %s, 电话: %s, 地址: %s\n", id, name, phone, address);
    }

    fclose(file);
}

// 查找联系人
void searchContacts() 
{
    char keyword[256];
    printf("请输入查找的关键词（姓名、电话或地址）: ");
    fgets(keyword, sizeof(keyword), stdin);
    keyword[strcspn(keyword, "\n")] = 0; // 去掉换行符

    FILE* file = fopen(DATA_FILE, "rb");
    if (file == NULL)
    {
        perror("Failed to open file");
        return;
    }

    MetaStructs meta;
    size_t readSize;

    printf("查找结果:\n");

    // 遍历元结构体区域
    for (unsigned short i = 0; i < (META_STORAGE / sizeof(MetaStructs)); i++)
    {
        fseek(file, i * sizeof(MetaStructs), SEEK_SET); // 定位到当前元结构体位置
        readSize = fread(&meta, sizeof(MetaStructs), 1, file); // 读取元结构体

        // 检查是否成功读取
        if (readSize != 1 || meta.id == 0) 
        {
            break; // 如果没有读取到有效数据，跳出循环
        }

        // 读取联系人信息
        char flag;
        fseek(file, meta.start, SEEK_SET); // 定位到联系人数据开始位置
        fread(&flag, sizeof(char), 1, file); // 读取 flag

        size_t idLength = meta.nameOffset - meta.idOffset;
        size_t nameLength = meta.phoneOffset - meta.nameOffset;
        size_t phoneLength = meta.addressOffset - meta.phoneOffset;
        size_t addressLength = meta.end - meta.addressOffset;

        // 检查 flag 是否为删除标志
        if (flag == 1)
        {
            continue; // 已删除，跳过
        }

        unsigned short id;
        fread(&id, sizeof(char), idLength, file); // 读取 ID

        // 读取姓名
        char name[256];
        fread(name, sizeof(char), nameLength, file);

        // 读取电话
        char phone[256];
        fread(phone, sizeof(char), phoneLength, file);

        // 读取地址
        char address[256];
        fread(address, sizeof(char), addressLength, file);


        // 检查是否包含关键词
        if (strstr(name, keyword) || strstr(phone, keyword) || strstr(address, keyword)) {
            // 打印匹配的联系人信息
            printf("ID: %hu, 姓名: %s, 电话: %s, 地址: %s\n", id, name, phone, address);
        }
    }

    fclose(file);
}

// 修改联系人
void modifyContact() {
    unsigned short idToModify;
    printf("请输入要修改的联系人 ID: ");
    scanf_s("%hu", &idToModify);
    getchar(); // 清除换行符

    FILE* file = fopen(DATA_FILE, "r+b");
    if (file == NULL) 
    {
        perror("Failed to open file");
        return;
    }

    MetaStructs meta;
    size_t readSize;

    // 查找对应 ID 的元结构体
    fseek(file, (idToModify - 1) * sizeof(MetaStructs), SEEK_SET);
    readSize = fread(&meta, sizeof(MetaStructs), 1, file);

    // 检查是否成功读取
    if (readSize != 1 || meta.id == 0) 
    {
        printf("未找到 ID 为 %hu 的联系人。\n", idToModify);
        fclose(file);
        return;
    }

    size_t idlength = meta.nameOffset - meta.idOffset;
    size_t namelength = meta.phoneOffset - meta.nameOffset;
    size_t phonelength = meta.addressOffset - meta.phoneOffset;
    size_t addresslength = meta.end - meta.addressOffset;

    // 读取当前联系人信息
    char flag;
    fseek(file, meta.start, SEEK_SET); // 定位到联系人数据开始位置
    fread(&flag, sizeof(char), 1, file); // 读取 flag

    if(flag == 1)
    {
        printf("联系人已删除");
        return;
    }

    unsigned short id;
    fread(&id, sizeof(char), idlength, file); // 读取 ID

    // 读取姓名
    char name[256];
    fread(name, sizeof(char), namelength, file);

    // 读取电话
    char phone[256];
    fread(phone, sizeof(char), phonelength, file);

    // 读取地址
    char address[256];
    fread(address, sizeof(char), addresslength, file);

    // 输入新的联系人数据
    char buffer[256];
    printf("当前姓名: %s\n", name);
    printf("请输入新的姓名 (留空则不修改): ");
    fgets(buffer, sizeof(buffer), stdin);
    if (buffer[0] != '\n') 
    {
        buffer[strcspn(buffer, "\n")] = 0; // 去掉换行符
        strcpy(name, buffer);
    }

    printf("当前电话: %s\n", phone);
    printf("请输入新的电话 (留空则不修改): ");
    fgets(buffer, sizeof(buffer), stdin);
    if (buffer[0] != '\n') 
    {
        buffer[strcspn(buffer, "\n")] = 0; // 去掉换行符
        strcpy(phone, buffer);
    }

    printf("当前地址: %s\n", address);
    printf("请输入新的地址 (留空则不修改): ");
    fgets(buffer, sizeof(buffer), stdin);
    if (buffer[0] != '\n')
    {
        buffer[strcspn(buffer, "\n")] = 0; // 去掉换行符
        strcpy(address, buffer);
    }

    // 计算新数据的长度
    long newDataLength = sizeof(char) + sizeof(unsigned short) + strlen(name) + 1 + strlen(phone) + 1 + strlen(address) + 1;

    // 判断新数据长度与旧数据长度的关系
    long oldDataLength = meta.end - meta.start;

    if (newDataLength > oldDataLength) {
        // 新数据长度大于旧数据长度，标记原数据为删除，并在文件末尾添加新数据
        char delete_flag = 1; // 设置为删除标志
        fseek(file, meta.start + meta.flagOffset, SEEK_SET);
        fwrite(&delete_flag, sizeof(char), 1, file); // 更新 flag

        // 将新数据写入文件末尾
        fseek(file, 0, SEEK_END);
        long newContactStart = ftell(file); // 获取新联系人开始位置

     
        fwrite(&flag, sizeof(char), 1, file); // 读取到的原flag 应该为0
        fwrite(&idToModify, sizeof(unsigned short), 1, file); // 写入 ID
        fwrite(name, sizeof(char), strlen(name) + 1, file); // 写入姓名
        fwrite(phone, sizeof(char), strlen(phone) + 1, file); // 写入电话
        fwrite(address, sizeof(char), strlen(address) + 1, file); // 写入地址

        long newContactEnd = ftell(file); // 获取新联系人结束位置

        // 更新元结构体
        meta.id = idToModify; // 设置 ID
        meta.start = newContactStart; // 设置开始位置
        meta.end = newContactEnd; // 设置结束位置

        // 更新偏移量
        meta.flagOffset = 0;
        meta.idOffset = sizeof(char);
        meta.nameOffset = sizeof(char) + sizeof(unsigned short);
        meta.phoneOffset = meta.nameOffset + strlen(name) + 1;
        meta.addressOffset = meta.phoneOffset + strlen(phone) + 1;

        // 更新元结构体在文件中的位置
        fseek(file, (meta.id - 1) * sizeof(MetaStructs), SEEK_SET);
        fwrite(&meta, sizeof(MetaStructs), 1, file); // 更新元结构体
    }
    else
    {
        // 新数据长度小于等于旧数据长度，直接在原位置写入
        fseek(file, meta.start, SEEK_SET); // 定位到联系人数据开始位置
        fwrite(&flag, sizeof(char), 1, file); // 更新 flag
        fwrite(&id, sizeof(unsigned short), 1, file); // 更新 ID
        fwrite(name, sizeof(char), strlen(name) + 1, file); // 更新姓名
        fwrite(phone, sizeof(char), strlen(phone) + 1, file); // 更新电话
        fwrite(address, sizeof(char), strlen(address) + 1, file); // 更新地址

        // 更新元结构体
        meta.end = meta.start + newDataLength; // 更新结束位置
        // 更新偏移量
        meta.flagOffset = 0;
        meta.idOffset = sizeof(char);
        meta.nameOffset = sizeof(char) + sizeof(unsigned short);
        meta.phoneOffset = meta.nameOffset + strlen(name) + 1;
        meta.addressOffset = meta.phoneOffset + strlen(phone) + 1;

        fseek(file, (meta.id - 1) * sizeof(MetaStructs), SEEK_SET);
        fwrite(&meta, sizeof(MetaStructs), 1, file); // 更新元结构体
    }
    fclose(file);
    printf("联系人 ID %hu 已被修改。\n", idToModify);
}

// 统计每个字母的出现次数和比例
void count_letters() 
{
    FILE* file = fopen(DATA_FILE, "rb");
    if (file == NULL) 
    {
        perror("Failed to open file");
        return;
    }

    // 定位到文件的5MB位置
    fseek(file, META_STORAGE, SEEK_SET);

    // 初始化字母计数
    int letter_count[26] = { 0 }; // 用于存储每个字母的计数
    long total_letters = 0; // 总字母计数

    char ch;
    while (fread(&ch, sizeof(char), 1, file) == 1)
    {
        // 转换为小写字母
        ch = tolower(ch);
        // 检查是否为字母
        if (ch >= 'a' && ch <= 'z') 
        {
            letter_count[ch - 'a']++;
            total_letters++;
        }
    }

    fclose(file);

    // 打印结果
    printf("字母出现次数和比例:\n");
    for (int i = 0; i < 26; i++) 
    {
        if (letter_count[i] > 0) 
        {
            double proportion = (double)letter_count[i] / total_letters * 100; // 计算比例
            printf("%c: %d (%.2f%%)\n", 'a' + i, letter_count[i], proportion);
        }
    }
}

// 显示存储信息
void displayStorageInfo() 
{
    FILE* file = fopen(DATA_FILE, "rb");
    if (file == NULL) 
    {
        perror("无法打开文件");
        return;
    }

    // 定位到文件末尾，获取文件当前大小
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, META_STORAGE, SEEK_SET); // 定位到5MB

    // 从5MB之后的长度
    long storageSize = fileSize - META_STORAGE;

    // 创建一个与5MB之后的长度相同的存储变量，初始化为全F
    char* storage = (char*)malloc(storageSize);
    if (storage == NULL)
    {
        perror("内存分配失败");
        fclose(file);
        return;
    }
    memset(storage, 'F', storageSize); // 全部初始化为 'F'

    MetaStructs meta;
    size_t readSize;

    // 遍历元结构体区域
    for (unsigned short i = 0; i < (META_STORAGE / sizeof(MetaStructs)); i++) 
    {
        fseek(file, i * sizeof(MetaStructs), SEEK_SET); // 定位到当前元结构体位置
        readSize = fread(&meta, sizeof(MetaStructs), 1, file); // 读取元结构体

        // 检查是否成功读取
        if (readSize != 1) 
        {
            break; 
        }

        // 如果 ID 为 0，说明已经访问完了元数据区
        if (meta.id == 0)
        {
            break; // 直接退出循环
        }

        // 读取联系人数据的 flag
        char flag;
        fseek(file, meta.start, SEEK_SET); // 定位到联系人数据开始位置
        fread(&flag, sizeof(char), 1, file); // 读取 flag

        // 根据 flag 更新存储变量
        if (flag == 0)
        {
            // 将已使用区域标记为 'U'
            for (long j = meta.start - META_STORAGE; j < meta.end - META_STORAGE; j++)
            {
                if (j >= 0 && j < storageSize) { // 确保索引在范围内
                    storage[j] = 'U'; // 已使用区域
                }
            }
        }
    }

    // 输出存储变量
    printf("存储资源分布:\n");
    for (size_t i = 0; i < storageSize; i++) 
    {
        printf("%c", storage[i]);
        if ((i + 1) % 50 == 0) 
        { 
            printf("\n");
        }
    }
    printf("\n");

    free(storage);
    fclose(file);
}

void defragmentStorage()
{
    FILE* file = fopen(DATA_FILE, "r+b");
    if (file == NULL)
    {
        perror("无法打开原始文件");
        return;
    }

    // 创建临时文件
    FILE* tempFile = fopen("temp_contacts.bin", "wb+");
    if (tempFile == NULL) 
    {
        perror("无法创建临时文件");
        fclose(file);
        return;
    }

    // 填充临时文件到5MB
    char zero = 0; // 用于填充的零字节
    fseek(tempFile, META_STORAGE - 1, SEEK_SET); // 移动到5MB位置
    fputc(zero, tempFile); // 创建5MB的文件

    MetaStructs meta;
    size_t readSize;
    long writePosition = META_STORAGE; // 在临时文件中的写入位置，从META_STORAGE开始

    int newId = 0;
    for (unsigned short i = 0; i < (META_STORAGE / sizeof(MetaStructs)); i++)
    {
        fseek(file, i * sizeof(MetaStructs), SEEK_SET); // 定位到当前元结构体位置
        readSize = fread(&meta, sizeof(MetaStructs), 1, file); // 读取元结构体

        // 检查是否成功读取
        if (readSize != 1) 
        {
            break;
        }

        // 如果 ID 为 0，说明已经访问完了元数据区
        if (meta.id == 0) 
        {
            break; // 直接退出循环
        }

        char flag;
        fseek(file, meta.start, SEEK_SET); // 定位到联系人数据开始位置
        fread(&flag, sizeof(char), 1, file); // 读取 flag

        // 检查 flag 是否为删除标志
        if (flag == 1) 
        {
            continue; // 已删除，跳过
        }

        // 计算实际数据的大小
        long actualSize = meta.end - meta.start;

        // 读取联系人数据
        char* dataBuffer = (char*)malloc(actualSize);
        if (dataBuffer == NULL) 
        {
            perror("内存分配失败");
            fclose(file);
            fclose(tempFile);
            return;
        }

        fseek(file, meta.start, SEEK_SET); // 确保指针在正确位置
        fread(dataBuffer, sizeof(char), actualSize, file); // 读取联系人数据

        // 更新数据中的 ID
        unsigned short* contactIdPtr = (unsigned short*)(dataBuffer + sizeof(char)); // 指向 ID 的位置
        newId++;
        *contactIdPtr = newId; // 更新 ID，确保从1开始

        // 写入到临时文件
        fseek(tempFile, writePosition, SEEK_SET);
        fwrite(dataBuffer, sizeof(char), actualSize, tempFile);

        // 更新元结构体
        meta.start = writePosition;
        meta.end = writePosition + actualSize;
        meta.id = newId; // 更新 ID，确保从1开始

        // 写入更新后的元结构体到临时文件
        fseek(tempFile, (meta.id - 1) * sizeof(MetaStructs), SEEK_SET);
        fwrite(&meta, sizeof(MetaStructs), 1, tempFile);

        // 更新写入位置
        writePosition += actualSize;
        free(dataBuffer);
    }

    fclose(file);
    fclose(tempFile);

    // 替换原文件
    if (remove(DATA_FILE) != 0)
    {
        perror("无法删除原文件");
        return;
    }
    if (rename("temp_contacts.bin", DATA_FILE) != 0)
    {
        perror("无法重命名临时文件");
        return;
    }

    printf("碎片整理完成，联系人编号已重新排列。\n");
}

// 显示菜单
void showMenu() {
    printf("=====================\n");
    printf("菜单选项:\n");
    printf("0. 初始化文件\n");
    printf("1. 添加联系人\n");
    printf("2. 删除联系人\n");
    printf("3. 读取所有联系人\n");
    printf("4. 查找联系人\n");
    printf("5. 修改联系人\n");
    printf("6. 统计字母出现次数和比例\n");
    printf("7. 显示存储信息\n");
    printf("8. 碎片整理\n");
    printf("9. 退出\n");
    printf("=====================\n");
}

// 主函数
int main() {
    int choice;
    do {
        showMenu();
        printf("请输入您的选择: ");
        scanf("%d", &choice);
        getchar(); // 清除换行符

        switch (choice) {
        case 0:
            initializeFile(); // 调用文件初始化函数
            break;
        case 1:
            addContact(); // 添加联系人函数
            break;
        case 2:
            deleteContact(); // 删除联系人函数
            break;
        case 3:
            readContacts(); // 读取所有联系人函数
            break;
        case 4:
            searchContacts(); // 查找联系人函数
            break;
        case 5:
            modifyContact(); // 修改联系人函数
            break;
        case 6:
            count_letters(); // 统计字母函数
            break;
        case 7:
            displayStorageInfo(); // 显示存储信息函数
            break;
        case 8:
            defragmentStorage(); // 碎片整理函数
            break;
        case 9:
            printf("退出程序。\n");
            break;
        default:
            printf("无效的选择，请重试。\n");
        }
    } while (choice != 9); // 退出条件调整为选择9

    return 0;
}