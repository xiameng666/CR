#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME_LENGTH 10
#define MAX_PHONE_LENGTH 15
#define MAX_ADDRESS_LENGTH 100
#define MAX_CONTACTS 100

#define MAX_FILE_SIZE 5*1024*1024  // ���洢�ռ��С5MB
#define DATA_FILE "contacts.txt"

#define SAFE_CLOSE(fp) if(fp){fclose(fp);fp=NULL;}

typedef struct {
    int id;             // ��ϵ�˱��
    int size;           // �����ṹ�壨����Ԫ���ݺ��������飩�Ĵ�С
    int nameLength;     // ��������
    int phoneLength;    // �绰����
    int addressLength;  // ��ַ����
    int isDeleted;      // ����Ƿ���ɾ��
    char data[];        // �������飬���ڴ洢�������绰�͵�ַ
} Contact;

void AddContact(FILE* file) {
    char name[MAX_NAME_LENGTH];
    char phone[MAX_PHONE_LENGTH];
    char address[MAX_ADDRESS_LENGTH];

    printf("���������� (���10�ֽ�): ");
    fgets(name, MAX_NAME_LENGTH, stdin);
    printf("������绰 (���15�ֽ�): ");
    fgets(phone, MAX_PHONE_LENGTH, stdin);
    printf("�������ַ (���100�ֽ�): ");
    fgets(address, MAX_ADDRESS_LENGTH, stdin);

    // ȥ�����з�
    name[strcspn(name, "\n")] = '\0';
    phone[strcspn(phone, "\n")] = '\0';
    address[strcspn(address, "\n")] = '\0';

    // �������������Թ�����
    printf("����: %s, �绰: %s, ��ַ: %s\n", name, phone, address);

    int nameLength = strlen(name);
    int phoneLength = strlen(phone);
    int addressLength = strlen(address);
    int dataLength = nameLength + phoneLength + addressLength;
    int totalSize = sizeof(Contact) + dataLength;

    // �ҵ���ǰ���ID
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

    // �ҵ���һ�����п鲢д������
    fseek(file, 0, SEEK_END); // д���ļ�ĩβ
    fwrite(newContact, totalSize, 1, file);
    fflush(file);

    free(newContact);
    printf("��ӳɹ�\n");
}

void DeleteContact(FILE* file, int id) {
    fseek(file, 0, SEEK_SET); // ���ļ���ͷ��ʼ����
    Contact contact;
    while (fread(&contact, sizeof(Contact), 1, file) == 1) {
        if (contact.id == id && contact.isDeleted == 0) {
            contact.isDeleted = 1;
            fseek(file, -contact.size, SEEK_CUR); // �ص��ṹ�忪ͷ
            fwrite(&contact, sizeof(Contact), 1, file);
            fflush(file);
            printf("��ϵ��ID %d ��ɾ��\n", id);
            return;
        }
        fseek(file, contact.size - sizeof(Contact), SEEK_CUR); // ������һ����ϵ��λ��
    }
    printf("δ�ҵ� ID Ϊ %d ����ϵ��\n", id);
}

void ModifyContact(FILE* file, int id) {
    fseek(file, 0, SEEK_SET);
    Contact contact;
    while (fread(&contact, sizeof(Contact), 1, file) == 1) {
        if (contact.id == id && contact.isDeleted == 0) {
            char name[MAX_NAME_LENGTH];
            char phone[MAX_PHONE_LENGTH];
            char address[MAX_ADDRESS_LENGTH];
            printf("������������ (���10�ֽ�): ");
            fgets(name, MAX_NAME_LENGTH, stdin);
            printf("�������µ绰 (���15�ֽ�): ");
            fgets(phone, MAX_PHONE_LENGTH, stdin);
            printf("�������µ�ַ (���100�ֽ�): ");
            fgets(address, MAX_ADDRESS_LENGTH, stdin);

            // ȥ�����з�
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
                printf("��ϵ�����޸�\n");
            }
            return;
        }
        fseek(file, contact.size - sizeof(Contact), SEEK_CUR);
    }
    printf("δ�ҵ� ID Ϊ %d ����ϵ��\n", id);
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
                printf("ID: %d\n����: %s\n�绰: %s\n��ַ: %s\n", contact.id, name, phone, address);
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

    printf("��ĸͳ��:\n");
    for (int i = 0; i < 26; i++) {
        if (letterCount[i] > 0) {
            double percentage = (double)letterCount[i] / totalLetters * 100.0;
            printf("%c: %d �Σ�ռ %.2f%%\n", 'a' + i, letterCount[i], percentage);
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

    printf("�洢�ռ�ֲ�:\n");
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
        perror("�޷�������ʱ�ļ�");
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
        perror("�޷����´������ļ�");
    }
}

int main() {
    FILE* file = fopen(DATA_FILE, "r+b");
    if (!file) {
        file = fopen(DATA_FILE, "w+b");
        if (!file) {
            perror("�޷����������ļ�");
            return 1;
        }
    }

    int choice;
    while (1) {
        printf("1. �����ϵ��\n");
        printf("2. ɾ����ϵ��\n");
        printf("3. �޸���ϵ��\n");
        printf("4. ��ѯ��ϵ��\n");
        printf("5. ͳ����ĸ���ִ���\n");
        printf("6. ��ʾ�洢��Ϣ\n");
        printf("7. ��Ƭ����\n");
        printf("0. �˳�\n");
        printf("ѡ�����: ");
        scanf("%d", &choice);
        getchar(); // ����������Ļ��з�

        switch (choice) {
        case 1:
            AddContact(file);
            break;
        case 2: {
            int id;
            printf("��������ϵ��ID: ");
            scanf("%d", &id);
            getchar(); // ����������Ļ��з�
            DeleteContact(file, id);
            break;
        }
        case 3: {
            int id;
            printf("��������ϵ��ID: ");
            scanf("%d", &id);
            getchar(); // ����������Ļ��з�
            ModifyContact(file, id);
            break;
        }
        case 4: {
            char keyword[MAX_NAME_LENGTH + MAX_PHONE_LENGTH + MAX_ADDRESS_LENGTH];
            printf("�������ѯ�ؼ���: ");
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
            printf("��Ч��ѡ�������ԡ�\n");
            break;
        }
    }

    SAFE_CLOSE(file);
    return 0;
}