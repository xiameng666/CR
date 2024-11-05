#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>

#define DATA_FILE "contacts.bin"
#define META_STORAGE (5 * 1024 * 1024) // Ԫ�ṹ�����򣬹̶�5MB��С  5MB/32B=163840����¼


//32�ֽ�
typedef struct MetaStructs {
    unsigned long id;           // ID
    unsigned long start;          // ��¼���ļ��еĿ�ʼλ��
    unsigned long end;            // ��¼���ļ��еĽ���λ��
    unsigned long flagOffset;   // ɾ����־λ�����ƫ����
    unsigned long idOffset;     // ID���ƫ����
    unsigned long nameOffset;     // ���������ƫ����
    unsigned long phoneOffset;    // �绰�����ƫ����
    unsigned long addressOffset;  // ��ַ�����ƫ����
} MetaStructs;

typedef struct Contact {
    char flag;                  // ɾ����־
    unsigned short id;          // ��ϵ��ID
    char* name;                 // ����
    char* phone;                // �绰
    char* address;              // ��ַ
} Contact;

// ��ʼ���ļ�
void initializeFile() 
{
    FILE* file = fopen(DATA_FILE, "wb+");
    if (file == NULL) 
    {
        perror("Failed");
        exit(EXIT_FAILURE);
    }

    char zero = 0; // �����������ֽ�
    fseek(file, META_STORAGE - 1, SEEK_SET); // �ƶ���5MB
    fputc(zero, file); // ����5MB���ļ�
    fclose(file);
    printf("��ʼ����ɣ�Ԫ��������СΪ5MB��\n");
}

// ��ȡ��ǰ��� ID
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
        fseek(file, i * sizeof(MetaStructs), SEEK_SET); // ��λ����ǰԪ�ṹ��λ��
        size_t read_size = fread(&meta, sizeof(MetaStructs), 1, file); // ��ȡԪ�ṹ��

        // ����Ƿ�ɹ���ȡ
        if (read_size != 1) 
        {
            printf("id��ȡʧ��");
            break; // ���û�ж�ȡ����Ч���ݣ�����ѭ��
        }

        // ��� ID Ϊ 0��ֱ�ӷ��ص�ǰ��� ID
        if (meta.id == 0) 
        {
            fclose(file);
            return maxId; // �����Ѿ���ȡ����� ID
        }

        // ������� ID
        if (meta.id > maxId) 
        {
            maxId = meta.id; // ������� ID
        }
    }

    fclose(file);
    return maxId;
}

// ������뻺����
void clearInputBuffer() 
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// ��ȫ��ȡ����
void safeFgets(char* buffer, size_t size) 
{
    while (1) 
    {
        if (fgets(buffer, size, stdin) != NULL) 
        {
            size_t len = strcspn(buffer, "\n");
            if (buffer[len] == '\n') {
                buffer[len] = '\0'; // ȥ�����з�
                return;
            }
            else 
            {
                clearInputBuffer(); // ���������
                printf("���볬���� %zu �ֽڣ�����������: ", size - 1);
            }
        }
        else 
        {
            printf("�����ȡʧ�ܣ�����������: ");
        }
    }
}

// �����ϵ��
void addContact() 
{
    FILE* file = fopen(DATA_FILE, "r+b");
    if (file == NULL) 
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    Contact contact;
    contact.flag = 0; // �Զ�����Ϊ0

    unsigned short temp_id = getMaxId();
    contact.id = temp_id + 1; // ���� ID

    char buffer[101]; // 100�ֽ� + 1�ֽ����ڴ����ֹ��

    // ��������
    printf("��������ϵ������ (���100�ֽ�): ");
    safeFgets(buffer, sizeof(buffer));
    contact.name = _strdup(buffer);

    // ����绰
    printf("��������ϵ�˵绰 (���100�ֽ�): ");
    safeFgets(buffer, sizeof(buffer));
    contact.phone = _strdup(buffer);

    // �����ַ
    printf("��������ϵ�˵�ַ (���100�ֽ�): ");
    safeFgets(buffer, sizeof(buffer));
    contact.address = _strdup(buffer);

    // ��λ���ļ�ĩβ��д����ϵ������
    fseek(file, 0, SEEK_END);
    long contact_start = ftell(file); // ��ȡ��ǰ��ϵ�˿�ʼλ��

    fwrite(&contact.flag, sizeof(char), 1, file);
    fwrite(&contact.id, sizeof(unsigned short), 1, file);
    fwrite(contact.name, sizeof(char), strlen(contact.name) + 1, file);
    fwrite(contact.phone, sizeof(char), strlen(contact.phone) + 1, file);
    fwrite(contact.address, sizeof(char), strlen(contact.address) + 1, file);

    long contact_end = ftell(file); // ��ȡ��ǰ��ϵ�˽���λ��

    // ����Ԫ�ṹ��
    MetaStructs meta;
    meta.id = contact.id; // ���� ID
    meta.start = contact_start; // ���ÿ�ʼλ��
    meta.end = contact_end; // ���ý���λ��
    meta.flagOffset = 0;
    meta.idOffset = sizeof(char);
    meta.nameOffset = sizeof(char) + sizeof(unsigned short);
    meta.phoneOffset = meta.nameOffset + strlen(contact.name) + 1;
    meta.addressOffset = meta.phoneOffset + strlen(contact.phone) + 1;

    // ����Ԫ�ṹ�����ļ��е�λ��
    fseek(file, (meta.id - 1) * sizeof(MetaStructs), SEEK_SET);
    fwrite(&meta, sizeof(MetaStructs), 1, file); // ����Ԫ�ṹ��

    free(contact.name);
    free(contact.phone);
    free(contact.address);
    fclose(file);
}

// ɾ����ϵ��
void deleteContact() 
{
    unsigned short idToDelete;
    printf("������Ҫɾ������ϵ�� ID: ");
    if (scanf("%hu", &idToDelete) != 1) 
    {
        printf("�����ʽ����ȷ��������һ����Ч�� ID��\n");
        clearInputBuffer();
        return;
    }
    getchar(); // ������з�

    FILE* file = fopen(DATA_FILE, "r+b");
    if (file == NULL) 
    {
        perror("Failed to open file");
        return;
    }

    MetaStructs meta;
    size_t read_size;

    // ���Ҷ�Ӧ ID ��Ԫ�ṹ��
    fseek(file, (idToDelete - 1) * sizeof(MetaStructs), SEEK_SET);
    read_size = fread(&meta, sizeof(MetaStructs), 1, file);

    // ����Ƿ�ɹ���ȡ
    if (read_size != 1 || meta.id == 0) 
    {
        printf("δ�ҵ� ID Ϊ %hu ����ϵ�ˡ�\n", idToDelete);
        fclose(file);
        return;
    }

    // �� flag ����Ϊ 1����ʾɾ��
    // ����������ݵĵ�ַ
    fseek(file, meta.start + meta.flagOffset, SEEK_SET);
    char delete_flag = 1; // ����Ϊɾ����־
    fwrite(&delete_flag, sizeof(char), 1, file); // ���� flag

    printf("��ϵ�� ID %hu �ѱ�ɾ����\n", idToDelete);
    fclose(file);
}

// ��ʾ������ϵ��
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

    printf("������ϵ��:\n");

    // ����Ԫ�ṹ������
    for (unsigned short i = 0; i < (META_STORAGE / sizeof(MetaStructs)); i++) 
    {
        fseek(file, i * sizeof(MetaStructs), SEEK_SET); // ��λ����ǰԪ�ṹ��λ��
        readSize = fread(&meta, sizeof(MetaStructs), 1, file); // ��ȡԪ�ṹ��

        // ����Ƿ�ɹ���ȡ
        if (readSize != 1 || meta.id == 0) {
            break; // ���û�ж�ȡ����Ч���ݣ�����
        }

        size_t idlength = meta.nameOffset - meta.idOffset;
        size_t namelength = meta.phoneOffset - meta.nameOffset;
        size_t phonelength = meta.addressOffset - meta.phoneOffset;
        size_t addresslength = meta.end - meta.addressOffset;
      
        // ��ȡ��ϵ����Ϣ
        char flag;
        fseek(file, meta.start, SEEK_SET); // ��λ����ϵ�����ݿ�ʼλ��
        fread(&flag, sizeof(char), 1, file); // ��ȡ flag

        // ��� flag �Ƿ�Ϊɾ����־
        if (flag == 1) {
            continue; // ��ɾ��������
        }

        unsigned short id;
        fread(&id, sizeof(char), idlength, file); // ��ȡ ID

        // ��ȡ����
        char name[256];
        fread(name, sizeof(char), namelength, file);

        // ��ȡ�绰
        char phone[256];
        fread(phone, sizeof(char), phonelength, file);

        // ��ȡ��ַ
        char address[256];
        fread(address, sizeof(char), addresslength, file);

        // ��ӡ��ϵ����Ϣ
        printf("ID: %hu, ����: %s, �绰: %s, ��ַ: %s\n", id, name, phone, address);
    }

    fclose(file);
}

// ������ϵ��
void searchContacts() 
{
    char keyword[256];
    printf("��������ҵĹؼ��ʣ��������绰���ַ��: ");
    fgets(keyword, sizeof(keyword), stdin);
    keyword[strcspn(keyword, "\n")] = 0; // ȥ�����з�

    FILE* file = fopen(DATA_FILE, "rb");
    if (file == NULL)
    {
        perror("Failed to open file");
        return;
    }

    MetaStructs meta;
    size_t readSize;

    printf("���ҽ��:\n");

    // ����Ԫ�ṹ������
    for (unsigned short i = 0; i < (META_STORAGE / sizeof(MetaStructs)); i++)
    {
        fseek(file, i * sizeof(MetaStructs), SEEK_SET); // ��λ����ǰԪ�ṹ��λ��
        readSize = fread(&meta, sizeof(MetaStructs), 1, file); // ��ȡԪ�ṹ��

        // ����Ƿ�ɹ���ȡ
        if (readSize != 1 || meta.id == 0) 
        {
            break; // ���û�ж�ȡ����Ч���ݣ�����ѭ��
        }

        // ��ȡ��ϵ����Ϣ
        char flag;
        fseek(file, meta.start, SEEK_SET); // ��λ����ϵ�����ݿ�ʼλ��
        fread(&flag, sizeof(char), 1, file); // ��ȡ flag

        size_t idLength = meta.nameOffset - meta.idOffset;
        size_t nameLength = meta.phoneOffset - meta.nameOffset;
        size_t phoneLength = meta.addressOffset - meta.phoneOffset;
        size_t addressLength = meta.end - meta.addressOffset;

        // ��� flag �Ƿ�Ϊɾ����־
        if (flag == 1)
        {
            continue; // ��ɾ��������
        }

        unsigned short id;
        fread(&id, sizeof(char), idLength, file); // ��ȡ ID

        // ��ȡ����
        char name[256];
        fread(name, sizeof(char), nameLength, file);

        // ��ȡ�绰
        char phone[256];
        fread(phone, sizeof(char), phoneLength, file);

        // ��ȡ��ַ
        char address[256];
        fread(address, sizeof(char), addressLength, file);


        // ����Ƿ�����ؼ���
        if (strstr(name, keyword) || strstr(phone, keyword) || strstr(address, keyword)) {
            // ��ӡƥ�����ϵ����Ϣ
            printf("ID: %hu, ����: %s, �绰: %s, ��ַ: %s\n", id, name, phone, address);
        }
    }

    fclose(file);
}

// �޸���ϵ��
void modifyContact() {
    unsigned short idToModify;
    printf("������Ҫ�޸ĵ���ϵ�� ID: ");
    scanf_s("%hu", &idToModify);
    getchar(); // ������з�

    FILE* file = fopen(DATA_FILE, "r+b");
    if (file == NULL) 
    {
        perror("Failed to open file");
        return;
    }

    MetaStructs meta;
    size_t readSize;

    // ���Ҷ�Ӧ ID ��Ԫ�ṹ��
    fseek(file, (idToModify - 1) * sizeof(MetaStructs), SEEK_SET);
    readSize = fread(&meta, sizeof(MetaStructs), 1, file);

    // ����Ƿ�ɹ���ȡ
    if (readSize != 1 || meta.id == 0) 
    {
        printf("δ�ҵ� ID Ϊ %hu ����ϵ�ˡ�\n", idToModify);
        fclose(file);
        return;
    }

    size_t idlength = meta.nameOffset - meta.idOffset;
    size_t namelength = meta.phoneOffset - meta.nameOffset;
    size_t phonelength = meta.addressOffset - meta.phoneOffset;
    size_t addresslength = meta.end - meta.addressOffset;

    // ��ȡ��ǰ��ϵ����Ϣ
    char flag;
    fseek(file, meta.start, SEEK_SET); // ��λ����ϵ�����ݿ�ʼλ��
    fread(&flag, sizeof(char), 1, file); // ��ȡ flag

    if(flag == 1)
    {
        printf("��ϵ����ɾ��");
        return;
    }

    unsigned short id;
    fread(&id, sizeof(char), idlength, file); // ��ȡ ID

    // ��ȡ����
    char name[256];
    fread(name, sizeof(char), namelength, file);

    // ��ȡ�绰
    char phone[256];
    fread(phone, sizeof(char), phonelength, file);

    // ��ȡ��ַ
    char address[256];
    fread(address, sizeof(char), addresslength, file);

    // �����µ���ϵ������
    char buffer[256];
    printf("��ǰ����: %s\n", name);
    printf("�������µ����� (�������޸�): ");
    fgets(buffer, sizeof(buffer), stdin);
    if (buffer[0] != '\n') 
    {
        buffer[strcspn(buffer, "\n")] = 0; // ȥ�����з�
        strcpy(name, buffer);
    }

    printf("��ǰ�绰: %s\n", phone);
    printf("�������µĵ绰 (�������޸�): ");
    fgets(buffer, sizeof(buffer), stdin);
    if (buffer[0] != '\n') 
    {
        buffer[strcspn(buffer, "\n")] = 0; // ȥ�����з�
        strcpy(phone, buffer);
    }

    printf("��ǰ��ַ: %s\n", address);
    printf("�������µĵ�ַ (�������޸�): ");
    fgets(buffer, sizeof(buffer), stdin);
    if (buffer[0] != '\n')
    {
        buffer[strcspn(buffer, "\n")] = 0; // ȥ�����з�
        strcpy(address, buffer);
    }

    // ���������ݵĳ���
    long newDataLength = sizeof(char) + sizeof(unsigned short) + strlen(name) + 1 + strlen(phone) + 1 + strlen(address) + 1;

    // �ж������ݳ���������ݳ��ȵĹ�ϵ
    long oldDataLength = meta.end - meta.start;

    if (newDataLength > oldDataLength) {
        // �����ݳ��ȴ��ھ����ݳ��ȣ����ԭ����Ϊɾ���������ļ�ĩβ���������
        char delete_flag = 1; // ����Ϊɾ����־
        fseek(file, meta.start + meta.flagOffset, SEEK_SET);
        fwrite(&delete_flag, sizeof(char), 1, file); // ���� flag

        // ��������д���ļ�ĩβ
        fseek(file, 0, SEEK_END);
        long newContactStart = ftell(file); // ��ȡ����ϵ�˿�ʼλ��

     
        fwrite(&flag, sizeof(char), 1, file); // ��ȡ����ԭflag Ӧ��Ϊ0
        fwrite(&idToModify, sizeof(unsigned short), 1, file); // д�� ID
        fwrite(name, sizeof(char), strlen(name) + 1, file); // д������
        fwrite(phone, sizeof(char), strlen(phone) + 1, file); // д��绰
        fwrite(address, sizeof(char), strlen(address) + 1, file); // д���ַ

        long newContactEnd = ftell(file); // ��ȡ����ϵ�˽���λ��

        // ����Ԫ�ṹ��
        meta.id = idToModify; // ���� ID
        meta.start = newContactStart; // ���ÿ�ʼλ��
        meta.end = newContactEnd; // ���ý���λ��

        // ����ƫ����
        meta.flagOffset = 0;
        meta.idOffset = sizeof(char);
        meta.nameOffset = sizeof(char) + sizeof(unsigned short);
        meta.phoneOffset = meta.nameOffset + strlen(name) + 1;
        meta.addressOffset = meta.phoneOffset + strlen(phone) + 1;

        // ����Ԫ�ṹ�����ļ��е�λ��
        fseek(file, (meta.id - 1) * sizeof(MetaStructs), SEEK_SET);
        fwrite(&meta, sizeof(MetaStructs), 1, file); // ����Ԫ�ṹ��
    }
    else
    {
        // �����ݳ���С�ڵ��ھ����ݳ��ȣ�ֱ����ԭλ��д��
        fseek(file, meta.start, SEEK_SET); // ��λ����ϵ�����ݿ�ʼλ��
        fwrite(&flag, sizeof(char), 1, file); // ���� flag
        fwrite(&id, sizeof(unsigned short), 1, file); // ���� ID
        fwrite(name, sizeof(char), strlen(name) + 1, file); // ��������
        fwrite(phone, sizeof(char), strlen(phone) + 1, file); // ���µ绰
        fwrite(address, sizeof(char), strlen(address) + 1, file); // ���µ�ַ

        // ����Ԫ�ṹ��
        meta.end = meta.start + newDataLength; // ���½���λ��
        // ����ƫ����
        meta.flagOffset = 0;
        meta.idOffset = sizeof(char);
        meta.nameOffset = sizeof(char) + sizeof(unsigned short);
        meta.phoneOffset = meta.nameOffset + strlen(name) + 1;
        meta.addressOffset = meta.phoneOffset + strlen(phone) + 1;

        fseek(file, (meta.id - 1) * sizeof(MetaStructs), SEEK_SET);
        fwrite(&meta, sizeof(MetaStructs), 1, file); // ����Ԫ�ṹ��
    }
    fclose(file);
    printf("��ϵ�� ID %hu �ѱ��޸ġ�\n", idToModify);
}

// ͳ��ÿ����ĸ�ĳ��ִ����ͱ���
void count_letters() 
{
    FILE* file = fopen(DATA_FILE, "rb");
    if (file == NULL) 
    {
        perror("Failed to open file");
        return;
    }

    // ��λ���ļ���5MBλ��
    fseek(file, META_STORAGE, SEEK_SET);

    // ��ʼ����ĸ����
    int letter_count[26] = { 0 }; // ���ڴ洢ÿ����ĸ�ļ���
    long total_letters = 0; // ����ĸ����

    char ch;
    while (fread(&ch, sizeof(char), 1, file) == 1)
    {
        // ת��ΪСд��ĸ
        ch = tolower(ch);
        // ����Ƿ�Ϊ��ĸ
        if (ch >= 'a' && ch <= 'z') 
        {
            letter_count[ch - 'a']++;
            total_letters++;
        }
    }

    fclose(file);

    // ��ӡ���
    printf("��ĸ���ִ����ͱ���:\n");
    for (int i = 0; i < 26; i++) 
    {
        if (letter_count[i] > 0) 
        {
            double proportion = (double)letter_count[i] / total_letters * 100; // �������
            printf("%c: %d (%.2f%%)\n", 'a' + i, letter_count[i], proportion);
        }
    }
}

// ��ʾ�洢��Ϣ
void displayStorageInfo() 
{
    FILE* file = fopen(DATA_FILE, "rb");
    if (file == NULL) 
    {
        perror("�޷����ļ�");
        return;
    }

    // ��λ���ļ�ĩβ����ȡ�ļ���ǰ��С
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, META_STORAGE, SEEK_SET); // ��λ��5MB

    // ��5MB֮��ĳ���
    long storageSize = fileSize - META_STORAGE;

    // ����һ����5MB֮��ĳ�����ͬ�Ĵ洢��������ʼ��ΪȫF
    char* storage = (char*)malloc(storageSize);
    if (storage == NULL)
    {
        perror("�ڴ����ʧ��");
        fclose(file);
        return;
    }
    memset(storage, 'F', storageSize); // ȫ����ʼ��Ϊ 'F'

    MetaStructs meta;
    size_t readSize;

    // ����Ԫ�ṹ������
    for (unsigned short i = 0; i < (META_STORAGE / sizeof(MetaStructs)); i++) 
    {
        fseek(file, i * sizeof(MetaStructs), SEEK_SET); // ��λ����ǰԪ�ṹ��λ��
        readSize = fread(&meta, sizeof(MetaStructs), 1, file); // ��ȡԪ�ṹ��

        // ����Ƿ�ɹ���ȡ
        if (readSize != 1) 
        {
            break; 
        }

        // ��� ID Ϊ 0��˵���Ѿ���������Ԫ������
        if (meta.id == 0)
        {
            break; // ֱ���˳�ѭ��
        }

        // ��ȡ��ϵ�����ݵ� flag
        char flag;
        fseek(file, meta.start, SEEK_SET); // ��λ����ϵ�����ݿ�ʼλ��
        fread(&flag, sizeof(char), 1, file); // ��ȡ flag

        // ���� flag ���´洢����
        if (flag == 0)
        {
            // ����ʹ��������Ϊ 'U'
            for (long j = meta.start - META_STORAGE; j < meta.end - META_STORAGE; j++)
            {
                if (j >= 0 && j < storageSize) { // ȷ�������ڷ�Χ��
                    storage[j] = 'U'; // ��ʹ������
                }
            }
        }
    }

    // ����洢����
    printf("�洢��Դ�ֲ�:\n");
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
        perror("�޷���ԭʼ�ļ�");
        return;
    }

    // ������ʱ�ļ�
    FILE* tempFile = fopen("temp_contacts.bin", "wb+");
    if (tempFile == NULL) 
    {
        perror("�޷�������ʱ�ļ�");
        fclose(file);
        return;
    }

    // �����ʱ�ļ���5MB
    char zero = 0; // �����������ֽ�
    fseek(tempFile, META_STORAGE - 1, SEEK_SET); // �ƶ���5MBλ��
    fputc(zero, tempFile); // ����5MB���ļ�

    MetaStructs meta;
    size_t readSize;
    long writePosition = META_STORAGE; // ����ʱ�ļ��е�д��λ�ã���META_STORAGE��ʼ

    int newId = 0;
    for (unsigned short i = 0; i < (META_STORAGE / sizeof(MetaStructs)); i++)
    {
        fseek(file, i * sizeof(MetaStructs), SEEK_SET); // ��λ����ǰԪ�ṹ��λ��
        readSize = fread(&meta, sizeof(MetaStructs), 1, file); // ��ȡԪ�ṹ��

        // ����Ƿ�ɹ���ȡ
        if (readSize != 1) 
        {
            break;
        }

        // ��� ID Ϊ 0��˵���Ѿ���������Ԫ������
        if (meta.id == 0) 
        {
            break; // ֱ���˳�ѭ��
        }

        char flag;
        fseek(file, meta.start, SEEK_SET); // ��λ����ϵ�����ݿ�ʼλ��
        fread(&flag, sizeof(char), 1, file); // ��ȡ flag

        // ��� flag �Ƿ�Ϊɾ����־
        if (flag == 1) 
        {
            continue; // ��ɾ��������
        }

        // ����ʵ�����ݵĴ�С
        long actualSize = meta.end - meta.start;

        // ��ȡ��ϵ������
        char* dataBuffer = (char*)malloc(actualSize);
        if (dataBuffer == NULL) 
        {
            perror("�ڴ����ʧ��");
            fclose(file);
            fclose(tempFile);
            return;
        }

        fseek(file, meta.start, SEEK_SET); // ȷ��ָ������ȷλ��
        fread(dataBuffer, sizeof(char), actualSize, file); // ��ȡ��ϵ������

        // ���������е� ID
        unsigned short* contactIdPtr = (unsigned short*)(dataBuffer + sizeof(char)); // ָ�� ID ��λ��
        newId++;
        *contactIdPtr = newId; // ���� ID��ȷ����1��ʼ

        // д�뵽��ʱ�ļ�
        fseek(tempFile, writePosition, SEEK_SET);
        fwrite(dataBuffer, sizeof(char), actualSize, tempFile);

        // ����Ԫ�ṹ��
        meta.start = writePosition;
        meta.end = writePosition + actualSize;
        meta.id = newId; // ���� ID��ȷ����1��ʼ

        // д����º��Ԫ�ṹ�嵽��ʱ�ļ�
        fseek(tempFile, (meta.id - 1) * sizeof(MetaStructs), SEEK_SET);
        fwrite(&meta, sizeof(MetaStructs), 1, tempFile);

        // ����д��λ��
        writePosition += actualSize;
        free(dataBuffer);
    }

    fclose(file);
    fclose(tempFile);

    // �滻ԭ�ļ�
    if (remove(DATA_FILE) != 0)
    {
        perror("�޷�ɾ��ԭ�ļ�");
        return;
    }
    if (rename("temp_contacts.bin", DATA_FILE) != 0)
    {
        perror("�޷���������ʱ�ļ�");
        return;
    }

    printf("��Ƭ������ɣ���ϵ�˱�����������С�\n");
}

// ��ʾ�˵�
void showMenu() {
    printf("=====================\n");
    printf("�˵�ѡ��:\n");
    printf("0. ��ʼ���ļ�\n");
    printf("1. �����ϵ��\n");
    printf("2. ɾ����ϵ��\n");
    printf("3. ��ȡ������ϵ��\n");
    printf("4. ������ϵ��\n");
    printf("5. �޸���ϵ��\n");
    printf("6. ͳ����ĸ���ִ����ͱ���\n");
    printf("7. ��ʾ�洢��Ϣ\n");
    printf("8. ��Ƭ����\n");
    printf("9. �˳�\n");
    printf("=====================\n");
}

// ������
int main() {
    int choice;
    do {
        showMenu();
        printf("����������ѡ��: ");
        scanf("%d", &choice);
        getchar(); // ������з�

        switch (choice) {
        case 0:
            initializeFile(); // �����ļ���ʼ������
            break;
        case 1:
            addContact(); // �����ϵ�˺���
            break;
        case 2:
            deleteContact(); // ɾ����ϵ�˺���
            break;
        case 3:
            readContacts(); // ��ȡ������ϵ�˺���
            break;
        case 4:
            searchContacts(); // ������ϵ�˺���
            break;
        case 5:
            modifyContact(); // �޸���ϵ�˺���
            break;
        case 6:
            count_letters(); // ͳ����ĸ����
            break;
        case 7:
            displayStorageInfo(); // ��ʾ�洢��Ϣ����
            break;
        case 8:
            defragmentStorage(); // ��Ƭ������
            break;
        case 9:
            printf("�˳�����\n");
            break;
        default:
            printf("��Ч��ѡ�������ԡ�\n");
        }
    } while (choice != 9); // �˳���������Ϊѡ��9

    return 0;
}