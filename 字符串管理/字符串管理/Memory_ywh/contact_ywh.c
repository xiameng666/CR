// contact_ywh.c : ���ļ����� "main" ����������ִ�н��ڴ˴���ʼ��������
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#define MAX_NAME_LENGTH 10
#define MAX_PHONE_LENGTH 15
#define MAX_ADDRESS_LENGTH 100
#define MAX_CONTACTS 100

#define MAX_FILE_SIZE 5*1024*1024  // ���洢�ռ��С5MB
#define MAX_STRING_SIZE 100    // �����ַ�����󳤶�
#define MAX_STRING_COUNT 100   // ����ַ����洢����
#define DATA_FILE "contacts.txt"

#define SAFE_CLOSE(fp) if(fp){fclose(fp);fp=NULL;}

typedef struct MemoryBlock
{ 
    char* cAddress;  // �ڴ�����ʼ��ַ
    int nSize;       // �ڴ��Ĵ�С
    int nIsFree;     // ��Ǹ��ڴ���Ƿ���У�1 ��ʾ���У�0 ��ʾ��ռ��
} MemoryBlock;

typedef struct Contact
{
    int id; // ��ϵ�˱��
    int size;
    char name[MAX_NAME_LENGTH];
    char phone[MAX_PHONE_LENGTH];
    char address[MAX_ADDRESS_LENGTH];
    int isDeleted; // ����Ƿ���ɾ��
} Contact;


/**
 * @brief �ṹ�����ڴ洢�ַ�������
 */
typedef struct StrData
{
    char cContent[MAX_STRING_SIZE];  // �ַ�������
    int nIndex;                       // �ַ������� ���
} StrData;

// ȫ�ֱ���
static char g_memorySpace[MAX_FILE_SIZE];  // �ڴ�ռ�
static MemoryBlock g_blocks[MAX_STRING_COUNT];  // �ڴ������
static StrData g_str[MAX_STRING_COUNT];  // �ַ�����������
static int g_numberOfStrings = 0;  // ��ǰ�ַ�������
static size_t g_usedMemory = 0;    // �޸�Ϊ size_t ����

static Contact g_contacts[MAX_CONTACTS];//�洢��ϵ��
static int g_contactCount = 0;//��ϵ�˼���

FILE* g_file = NULL;

/**
 * @brief ��ʼ��������ݽṹ
 */
static void Initialize(void)
{
    // �ֶ���ʼ�� g_memorySpace ����Ϊ 0
    for (int i = 0; i < MAX_FILE_SIZE; i++)
    {
        g_memorySpace[i] = 0;
    }

    // �ֶ���ʼ�� g_blocks ����
    for (int i = 0; i < MAX_STRING_SIZE; i++)
    {
        g_blocks[i].cAddress = NULL;
        g_blocks[i].nSize = 0;
        g_blocks[i].nIsFree = 1;  // ��ʼ��Ϊδʹ��״̬
    }

    // �ֶ���ʼ�� g_str ����
    for (int i = 0; i < MAX_STRING_SIZE; i++)
    {
        g_str[i].cContent[0] = '\0';  // ��ʼ��Ϊ���ַ���
        g_str[i].nIndex = -1;
    }

    // ��ʼ����ϵ������
    for (int i = 0; i < MAX_CONTACTS; i++) {
        g_contacts[i].isDeleted = 1;  // ��ʼ��Ϊ��ɾ��״̬
    }

    //Ԥ����5MB�ռ� 
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

    printf("������ϵ��:\n");
    for (int i = 0; i < g_contactCount; i++) 
    {
        if (g_contacts[i].isDeleted == 0)
        {
            printf("ID: %d\n", g_contacts[i].id);
            printf("����: %s", g_contacts[i].name); // ������һ��
            printf("�绰: %s", g_contacts[i].phone); // �绰�͵�ַ����һ��
            printf("��ַ: %s\n", g_contacts[i].address);
            contactFound = 1;
        }
    }

    if (!contactFound) 
    {
        printf("û���ҵ��κ���ϵ�ˡ�\n");
    }
}

void AddContact(void)
{
    if (g_contactCount >= MAX_CONTACTS)
    {
        printf("��ϵ�������Ѵ�����\n");
        return;
    }

    Contact newContact;
    newContact.id = g_contactCount + 1;
    printf("����������: ");
    fgets(newContact.name, MAX_NAME_LENGTH, stdin);
    printf("������绰: ");
    fgets(newContact.phone, MAX_PHONE_LENGTH, stdin);
    printf("�������ַ: ");
    fgets(newContact.address, MAX_ADDRESS_LENGTH, stdin);

    newContact.isDeleted = 0; // δɾ��
    g_contacts[g_contactCount] = newContact;

    // �����ļ��е�����
    FILE* g_file = fopen(DATA_FILE, "rb+");
    if (g_file == NULL)
    {
        printf("�޷����ļ�\n");
        return;
    }
    else
    {
        // ��λ���ļ��ж�Ӧ��λ��
        fseek(g_file, g_contactCount * sizeof(Contact), SEEK_SET);
        fwrite(&newContact, sizeof(Contact), 1, g_file); // д�뵥��Contact�ṹ��
        fclose(g_file);
        printf("��ӳɹ�\n");
    }

    g_contactCount++; // ������ϵ������
}


void DelectContact()
{
    int nDeletedId = 0;
    printf("����ɾ������ϵ�˱�ţ�");
    scanf_s("%d", &nDeletedId);
    for (int i = 0; i < g_contactCount; i++)
    {
        if (g_contacts[i].isDeleted == 0 && g_contacts[i].id == nDeletedId)
        {
            //��ɾ����ʶλ��1
            g_contacts[i].isDeleted = 1;

            FILE* g_file = fopen(DATA_FILE, "rb+");
            if (g_file)
            {
                fseek(g_file, i * sizeof(Contact), SEEK_SET);
                fwrite(&g_contacts[i], sizeof(Contact), 1, g_file);
                fclose(g_file);
            }
            printf("��ϵ��ID %d ��ɾ��\n", nDeletedId);

            return;  // �˳�DelectContact����
        }
    }
    printf("δ�ҵ� ID Ϊ %d ����ϵ��\n", nDeletedId);

}

void ModifyContact()
{
    int nModifyId = 0;
    printf("�����޸ĵ���ϵ�˱�ţ�");
    scanf_s("%d", &nModifyId);
    getchar();
    for (int i = 0; i < g_contactCount; i++) {
        if (g_contacts[i].id == nModifyId && g_contacts[i].isDeleted == 0) {
            printf("�޸���ϵ�� %d ��Ϣ\n", nModifyId);
            printf("������������: ");
            fgets(g_contacts[i].name, MAX_NAME_LENGTH, stdin);
            printf("�������µ绰: ");
            fgets(g_contacts[i].phone, MAX_PHONE_LENGTH, stdin);
            printf("�������µ�ַ: ");
            fgets(g_contacts[i].address, MAX_ADDRESS_LENGTH, stdin);

            FILE* g_file = fopen(DATA_FILE, "rb+");
            if (g_file) {
                fseek(g_file, i * sizeof(Contact), SEEK_SET);
                fwrite(&g_contacts[i], sizeof(Contact), 1, g_file);
                fclose(g_file);
            }
            printf("��ϵ�����޸�\n");

            return;
        }
    }
    printf("δ�ҵ�����ϵ��\n");
}
void SearchById(int id)
{
    for (int i = 0; i < g_contactCount; i++)
    {
        if (g_contacts[i].isDeleted == 0 && g_contacts[i].id == id)
        {
            printf("ID: %d\n����: %s\n�绰: %s\n��ַ: %s\n",
                g_contacts[i].id, g_contacts[i].name,
                g_contacts[i].phone, g_contacts[i].address);
            return;
        }
    }
    printf("δ�ҵ� ID Ϊ %d ����ϵ��\n", id);
}

void SearchByName(char* name)
{
    for (int i = 0; i < g_contactCount; i++)
    {
        if (g_contacts[i].isDeleted == 0 && strstr(g_contacts[i].name, name) != NULL)
        {
            printf("ID: %d\n����: %s\n�绰: %s\n��ַ: %s\n",
                g_contacts[i].id, g_contacts[i].name,
                g_contacts[i].phone, g_contacts[i].address);
            return;
        }
    }
    printf("δ�ҵ��������� '%s' ����ϵ��\n", name);
}

void SearchByPhone(char* phone)
{
    for (int i = 0; i < g_contactCount; i++) {
        if (g_contacts[i].isDeleted == 0 && strcmp(g_contacts[i].phone, phone) == 0) {
            printf("ID: %d\n����: %s\n�绰: %s\n��ַ: %s\n",
                g_contacts[i].id, g_contacts[i].name,
                g_contacts[i].phone, g_contacts[i].address);
            return;
        }
    }
    printf("δ�ҵ��绰Ϊ '%s' ����ϵ��\n", phone);
}

void SearchContact() 
{
    int choice;
    printf("��ѡ���ѯ��ʽ:\n");
    printf("1. ����Ų�ѯ\n");
    printf("2. �����ֲ�ѯ\n");
    printf("3. ���绰�����ѯ\n");
    printf("������ѡ��: ");
    scanf_s("%d", &choice);
    getchar(); // ������뻺����

    switch (choice) 
    {
    case 1: 
    {
        int id;
        printf("��������ϵ�˱��: ");
        scanf_s("%d",&id);
        getchar();  // ������뻺����
        SearchById(id);
        break;
    }
    case 2:
    {
        char name[MAX_NAME_LENGTH];
        printf("��������ϵ�����ֻ򲿷�����: ");
        fgets(name, MAX_NAME_LENGTH, stdin);
        name[strcspn(name, "\n")] = '\0'; // ȥ�����з�
        SearchByName(name);
        break;
    }
    case 3: 
    {
        char phone[MAX_PHONE_LENGTH];
        printf("��������ϵ�˵绰����: ");
        fgets(phone, MAX_PHONE_LENGTH, stdin);
        phone[strcspn(phone, "\n")] = '\0'; // ȥ�����з�
        SearchByPhone(phone);
        break;
    }
    default:
        printf("��Ч��ѡ��\n");
        break;
    }
}

void RefactorContacts(void) {
    FILE* file = fopen(DATA_FILE, "rb");
    if (!file) {
        printf("�޷����ļ�\n");
        return;
    }

    Contact tempContacts[MAX_CONTACTS];
    int usedCount = 0;

    // ��ȡ��ϵ�˲��ռ���Ч����
    while (fread(&tempContacts[usedCount], sizeof(Contact), 1, file) == 1) {
        if (tempContacts[usedCount].isDeleted == 0) {
            usedCount++;
        }
    }
    fclose(file);

    // ����д����Ч��ϵ�˲����±��
    file = fopen(DATA_FILE, "wb");
    if (!file) {
        printf("�޷����ļ�\n");
        return;
    }

    for (int i = 0; i < usedCount; i++) {
        tempContacts[i].id = i + 1; // ���·�����
        if (fwrite(&tempContacts[i], sizeof(Contact), 1, file) != 1) {
            printf("д����ϵ��ʱ����\n");
        }
    }

    fclose(file);
    printf("��Ƭ������ɣ���ϵ�˱���Ѹ��¡�\n");
}

void DisplayMemory(void) {
    FILE* file = fopen(DATA_FILE, "rb");
    if (!file) {
        printf("�޷����ļ�\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file); // ��ȡ�ļ���С
    fseek(file, 0, SEEK_SET); // �ص��ļ���ͷ

    char* storageMap = malloc(fileSize + 1); // Ϊ�洢ͼ�����ڴ�
    if (!storageMap) {
        printf("�ڴ����ʧ��\n");
        fclose(file);
        return;
    }

    // ��ʼ���洢ͼ
    for (long i = 0; i < fileSize; i++) {
        Contact contact;
        if (fread(&contact, sizeof(Contact), 1, file) == 1) {
            storageMap[i] = (contact.isDeleted == 0) ? 'U' : 'F'; // U: ��ʹ��, F: ����
        }
        else {
            storageMap[i] = 'F'; // �����ļ���Χ���Ϊ����
        }
    }
    storageMap[fileSize] = '\0'; // �����ַ���

    printf("�洢��Դ�ֲ�: %s\n", storageMap);

    free(storageMap);
    fclose(file);
}

void CountLetters(void)
{
    int nLetterCount[26] = { 0 };  // ���ڴ洢ÿ����ĸ�ļ���
    int nTotalLetters = 0;  // ����ĸ��

    FILE* g_file = fopen(DATA_FILE, "rb");
    if (g_file == NULL) 
    {
        printf("�޷����ļ�\n");
        return;
    }

    Contact contact;

    //���ļ� file �в��϶�ȡ���ݣ�������洢�� contact �ṹ���У�ֱ��û�и�������ݿɶ�
    while (fread(&contact, sizeof(Contact), 1, g_file));//fread ���سɹ���ȡ��Ԫ�ظ���,
    {
        if (contact.isDeleted == 0)
        {
            // ͳ����������
            for (int j = 0; contact.name[j] != '\0'; j++)
            {
                char ch = tolower(contact.name[j]);  // ת��ΪСд
                if (isalpha(ch)) 
                {  // ֻͳ����ĸ
                    nLetterCount[ch - 'a']++;
                    nTotalLetters++;
                }
            }

            // ͳ�Ƶ绰����
            for (int j = 0; contact.phone[j] != '\0'; j++)
            {
                char ch = tolower(contact.phone[j]);  // ת��ΪСд
                if (isalpha(ch)) 
                { 
                    nLetterCount[ch - 'a']++;
                    nTotalLetters++;
                }
            }

            // ͳ�Ƶ�ַ����
            for (int j = 0; contact.address[j] != '\0'; j++)
            {
                char ch = tolower(contact.address[j]);  // ת��ΪСд
                if (isalpha(ch)) 
                { 
                    nLetterCount[ch - 'a']++;
                    nTotalLetters++;
                }
            }
        }
    } 
    fclose(g_file);
    
    // ���ͳ�ƽ��
    printf("��ĸͳ��:\n");
    for (int i = 0; i < 26; i++)
    {
        if (nLetterCount[i] > 0)
        {
            double percentage = (double)nLetterCount[i] / nTotalLetters * 100.0;
            printf("%c: %d �Σ�ռ %.2f%%\n", 'a' + i, nLetterCount[i], percentage);
        }
    }

    if (nTotalLetters == 0)
    {
        printf("û����ĸ��ͳ�Ƶ���\n");
    }
}



static void Menu(void)
{
    int nChoice;
    printf("�ļ�����ϵͳ\n");

    while (1)
    {
        printf("---------------------\n");
        printf("0. ��ʾ������ϵ��\n");
        printf("1. ������ϵ��\n");
        printf("2. ɾ����ϵ��\n");
        printf("3. �޸���ϵ��\n");
        printf("4. ��ѯ��ϵ��\n");
        printf("5. ͳ����ĸ���ִ���\n");
        printf("6. ��ʾ�洢��Ϣ\n");
        printf("7. ������ϵ����Ϣ\n");
        printf("8. �˳�\n");
        printf("---------------------\n");
        printf("ָ��: ");

        if (scanf_s("%d", &nChoice) != 1) // ������벻������
        {
            // ������뻺�����Ա�������ѭ��
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);

            printf("�������ݴ���������һ����Ч������ѡ�\n");
            continue; // ���ز˵��������ȴ��û�����
        }
        getchar(); // ����������еĻ��з�

        switch (nChoice) {
        case 0:
            ShowAllContacts(); // ��ʾ������ϵ��
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
            printf("�˳�\n");
            return;
        default:
            printf("��Чѡ�������ѡ��\n");
        }
    }
}

int main(void)
{
    Initialize();  // ��ʼ������
    Menu();  // ��ʾ�˵���ִ�й���
    return 0;
}
