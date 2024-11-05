#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME_LENGTH 10
#define MAX_PHONE_LENGTH 15
#define MAX_ADDRESS_LENGTH 100
#define MAX_CONTACTS 100

#define MAX_FILE_SIZE 5*1024*1024  // 最大存储空间大小5MB
#define DATA_FILE "contacts.txt"

#define SAFE_CLOSE(fp) if(fp){fclose(fp);fp=NULL;}

typedef struct {
    int id;             // 联系人编号
    int size;           // 整个结构体（包括元数据和柔性数组）的大小
    int nameLength;     // 姓名长度
    int phoneLength;    // 电话长度
    int addressLength;  // 地址长度
    int isDeleted;      // 标记是否已删除
    char data[];        // 柔性数组，用于存储姓名、电话和地址
} Contact;

void AddContact(FILE* file) {
    char name[MAX_NAME_LENGTH];
    char phone[MAX_PHONE_LENGTH];
    char address[MAX_ADDRESS_LENGTH];

    printf("请输入姓名 (最多10字节): ");
    fgets(name, MAX_NAME_LENGTH, stdin);
    printf("请输入电话 (最多15字节): ");
    fgets(phone, MAX_PHONE_LENGTH, stdin);
    printf("请输入地址 (最多100字节): ");
    fgets(address, MAX_ADDRESS_LENGTH, stdin);

    // 去除换行符
    name[strcspn(name, "\n")] = '\0';
    phone[strcspn(phone, "\n")] = '\0';
    address[strcspn(address, "\n")] = '\0';

    // 输出输入的内容以供调试
    printf("姓名: %s, 电话: %s, 地址: %s\n", name, phone, address);

    int nameLength = strlen(name);
    int phoneLength = strlen(phone);
    int addressLength = strlen(address);
    int dataLength = nameLength + phoneLength + addressLength;
    int totalSize = sizeof(Contact) + dataLength;

    // 找到当前最大ID
    int id = 1;
    fseek(file, 0, SEEK_SET);
    Contact tempContact;
    while (fread(&tempContact, sizeof(Contact), 1, file) == 1) {
        if (!tempContact.isDeleted && tempContact.id >= id) {
            id = tempContact.id + 1;
        }
        fseek(file, tempContact.size - sizeof(Contact), SEEK_CUR);
    }

    Contact* newContact = (Contact*)malloc(totalSize);
    newContact->id = id;
    newContact->size = totalSize;
    newContact->nameLength = nameLength;
    newContact->phoneLength = phoneLength;
    newContact->addressLength = addressLength;
    newContact->isDeleted = 0;

    memcpy(newContact->data, name, nameLength);
    memcpy(newContact->data + nameLength, phone, phoneLength);
    memcpy(newContact->data + nameLength + phoneLength, address, addressLength);

    // 找到第一个空闲块并写入数据
    fseek(file, 0, SEEK_END); // 写入文件末尾
    fwrite(newContact, totalSize, 1, file);
    fflush(file);

    free(newContact);
    printf("添加成功\n");
}

void DeleteContact(FILE* file, int id) {
    fseek(file, 0, SEEK_SET); // 从文件开头开始查找
    Contact contact;
    while (fread(&contact, sizeof(Contact), 1, file) == 1) {
        if (contact.id == id && contact.isDeleted == 0) {
            contact.isDeleted = 1;
            fseek(file, -contact.size, SEEK_CUR); // 回到结构体开头
            fwrite(&contact, sizeof(Contact), 1, file);
            fflush(file);
            printf("联系人ID %d 已删除\n", id);
            return;
        }
        fseek(file, contact.size - sizeof(Contact), SEEK_CUR); // 跳到下一个联系人位置
    }
    printf("未找到 ID 为 %d 的联系人\n", id);
}

void ModifyContact(FILE* file, int id) {
    fseek(file, 0, SEEK_SET);
    Contact contact;
    while (fread(&contact, sizeof(Contact), 1, file) == 1) {
        if (contact.id == id && contact.isDeleted == 0) {
            char name[MAX_NAME_LENGTH];
            char phone[MAX_PHONE_LENGTH];
            char address[MAX_ADDRESS_LENGTH];
            printf("请输入新姓名 (最多10字节): ");
            fgets(name, MAX_NAME_LENGTH, stdin);
            printf("请输入新电话 (最多15字节): ");
            fgets(phone, MAX_PHONE_LENGTH, stdin);
            printf("请输入新地址 (最多100字节): ");
            fgets(address, MAX_ADDRESS_LENGTH, stdin);

            // 去除换行符
            name[strcspn(name, "\n")] = '\0';
            phone[strcspn(phone, "\n")] = '\0';
            address[strcspn(address, "\n")] = '\0';

            int nameLength = strlen(name);
            int phoneLength = strlen(phone);
            int addressLength = strlen(address);
            int newSize = sizeof(Contact) + nameLength + phoneLength + addressLength;

            if (newSize > contact.size) {
                contact.isDeleted = 1;
                fseek(file, -contact.size, SEEK_CUR);
                fwrite(&contact, sizeof(Contact), 1, file);
                fflush(file);
                AddContact(file);
            }
            else {
                contact.nameLength = nameLength;
                contact.phoneLength = phoneLength;
                contact.addressLength = addressLength;
                memcpy(contact.data, name, nameLength);
                memcpy(contact.data + nameLength, phone, phoneLength);
                memcpy(contact.data + nameLength + phoneLength, address, addressLength);
                fseek(file, -contact.size, SEEK_CUR);
                fwrite(&contact, newSize, 1, file);
                fflush(file);
                printf("联系人已修改\n");
            }
            return;
        }
        fseek(file, contact.size - sizeof(Contact), SEEK_CUR);
    }
    printf("未找到 ID 为 %d 的联系人\n", id);
}

void SearchContact(FILE* file, const char* keyword) {
    fseek(file, 0, SEEK_SET);
    Contact contact;
    while (fread(&contact, sizeof(Contact), 1, file) == 1) {
        if (contact.isDeleted == 0) {
            char name[MAX_NAME_LENGTH];
            char phone[MAX_PHONE_LENGTH];
            char address[MAX_ADDRESS_LENGTH];
            memcpy(name, contact.data, contact.nameLength);
            name[contact.nameLength] = '\0';
            memcpy(phone, contact.data + contact.nameLength, contact.phoneLength);
            phone[contact.phoneLength] = '\0';
            memcpy(address, contact.data + contact.nameLength + contact.phoneLength, contact.addressLength);
            address[contact.addressLength] = '\0';

            if (strstr(name, keyword) || strstr(phone, keyword) || strstr(address, keyword)) {
                printf("ID: %d\n姓名: %s\n电话: %s\n地址: %s\n", contact.id, name, phone, address);
            }
        }
        fseek(file, contact.size - sizeof(Contact), SEEK_CUR);
    }
}

void CountLetters(FILE* file) {
    int letterCount[26] = { 0 };
    int totalLetters = 0;
    fseek(file, 0, SEEK_SET);
    Contact contact;
    while (fread(&contact, sizeof(Contact), 1, file) == 1) {
        if (contact.isDeleted == 0) {
            for (int j = 0; j < contact.nameLength + contact.phoneLength + contact.addressLength; j++) {
                char ch = tolower(contact.data[j]);
                if (isalpha(ch)) {
                    letterCount[ch - 'a']++;
                    totalLetters++;
                }
            }
        }
        fseek(file, contact.size - sizeof(Contact), SEEK_CUR);
    }

    printf("字母统计:\n");
    for (int i = 0; i < 26; i++) {
        if (letterCount[i] > 0) {
            double percentage = (double)letterCount[i] / totalLetters * 100.0;
            printf("%c: %d 次，占 %.2f%%\n", 'a' + i, letterCount[i], percentage);
        }
    }
}

void DisplayMemory(FILE* file) {
    fseek(file, 0, SEEK_SET);
    char layout[MAX_FILE_SIZE / sizeof(Contact)];
    memset(layout, 'F', sizeof(layout));

    Contact contact;
    while (fread(&contact, sizeof(Contact), 1, file) == 1) {
        if (contact.isDeleted == 0) {
            int index = ftell(file) / sizeof(Contact);
            layout[index] = 'U';
        }
        fseek(file, contact.size - sizeof(Contact), SEEK_CUR);
    }

    printf("存储空间分布:\n");
    for (int i = 0; i < sizeof(layout); i++) {
        printf("%c", layout[i]);
        if ((i + 1) % 50 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void RefactorMemory(FILE* file) {
    FILE* tempFile = fopen("temp_contacts.txt", "wb");
    if (!tempFile) {
        perror("无法创建临时文件");
        return;
    }

    fseek(file, 0, SEEK_SET);
    Contact contact;
    int id = 1;
    while (fread(&contact, sizeof(Contact), 1, file) == 1) {
        if (contact.isDeleted == 0) {
            int dataLength = contact.nameLength + contact.phoneLength + contact.addressLength;
            int totalSize = sizeof(Contact) + dataLength;

            Contact* newContact = (Contact*)malloc(totalSize);
            newContact->id = id++;
            newContact->size = totalSize;
            newContact->nameLength = contact.nameLength;
            newContact->phoneLength = contact.phoneLength;
            newContact->addressLength = contact.addressLength;
            newContact->isDeleted = 0;

            memcpy(newContact->data, contact.data, dataLength);
            fwrite(newContact, totalSize, 1, tempFile);
            free(newContact);
        }
        fseek(file, contact.size - sizeof(Contact), SEEK_CUR);
    }

    SAFE_CLOSE(file);
    SAFE_CLOSE(tempFile);

    remove(DATA_FILE);
    rename("temp_contacts.txt", DATA_FILE);

    file = fopen(DATA_FILE, "r+b");
    if (!file) {
        perror("无法重新打开数据文件");
    }
}

int main() {
    FILE* file = fopen(DATA_FILE, "r+b");
    if (!file) {
        file = fopen(DATA_FILE, "w+b");
        if (!file) {
            perror("无法创建数据文件");
            return 1;
        }
    }

    int choice;
    while (1) {
        printf("1. 添加联系人\n");
        printf("2. 删除联系人\n");
        printf("3. 修改联系人\n");
        printf("4. 查询联系人\n");
        printf("5. 统计字母出现次数\n");
        printf("6. 显示存储信息\n");
        printf("7. 碎片整理\n");
        printf("0. 退出\n");
        printf("选择操作: ");
        scanf("%d", &choice);
        getchar(); // 清除缓冲区的换行符

        switch (choice) {
        case 1:
            AddContact(file);
            break;
        case 2: {
            int id;
            printf("请输入联系人ID: ");
            scanf("%d", &id);
            getchar(); // 清除缓冲区的换行符
            DeleteContact(file, id);
            break;
        }
        case 3: {
            int id;
            printf("请输入联系人ID: ");
            scanf("%d", &id);
            getchar(); // 清除缓冲区的换行符
            ModifyContact(file, id);
            break;
        }
        case 4: {
            char keyword[MAX_NAME_LENGTH + MAX_PHONE_LENGTH + MAX_ADDRESS_LENGTH];
            printf("请输入查询关键词: ");
            fgets(keyword, sizeof(keyword), stdin);
            keyword[strcspn(keyword, "\n")] = '\0';
            SearchContact(file, keyword);
            break;
        }
        case 5:
            CountLetters(file);
            break;
        case 6:
            DisplayMemory(file);
            break;
        case 7:
            RefactorMemory(file);
            break;
        case 0:
            SAFE_CLOSE(file);
            return 0;
        default:
            printf("无效的选择，请重试。\n");
            break;
        }
    }

    SAFE_CLOSE(file);
    return 0;
}