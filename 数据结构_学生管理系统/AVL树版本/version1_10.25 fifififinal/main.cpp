#include "StudentManagementSystem.h"
#include <iostream>
#include <limits>

void printMenu() {
    std::cout << "\nѧ������ϵͳ\n";
    std::cout << "1. ���ѧ��\n";
    std::cout << "2. ��ӿγ�\n";
    std::cout << "3. ��ӳɼ�\n";
    std::cout << "4. ��ѯѧ����Ϣ\n";
    std::cout << "5. ��ѯ�γ���Ϣ\n";
    std::cout << "6. ��ѯ�ɼ���Ϣ\n";
    std::cout << "7. �޸�ѧ����Ϣ\n";
    std::cout << "8. �޸Ŀγ���Ϣ\n";
    std::cout << "9. �޸ĳɼ�\n";
    std::cout << "10. ɾ��ѧ��\n";
    std::cout << "11. ɾ���γ�\n";
    std::cout << "12. ɾ���ɼ�\n";
    std::cout << "13. ��ʾ��������\n";
    std::cout << "14. �����������\n";
    std::cout << "15. ���ɲ�������\n";
    std::cout << "16. ����ʾ������\n";
    std::cout << "0. �˳�\n";
    std::cout << "��ѡ�����: ";
}

CMyString getInputWithLimit(const char* prompt, int maxLength) {
    char buffer[1024];  // ����������볤�Ȳ�����1024
    while (true) {
        std::cout << prompt;
        std::cin.getline(buffer, sizeof(buffer));
        CMyString input(buffer);
        if (input.length() <= maxLength) {
            return input;
        }
        std::cout << "���볬��" << maxLength << "���ַ������������롣" << std::endl;
    }
}

int main() {
    StudentManagementSystem system;
    int choice;
    
    do {
        printMenu();
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1: {
                int id;
                std::cout << "����ѧ��ID: ";
                std::cin >> id;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                CMyString name = getInputWithLimit("����ѧ������ (���10���ַ�): ", 10);
                system.insertStudent(id, name);
                break;
            }
            case 2: {
                int id;
                std::cout << "����γ�ID: ";
                std::cin >> id;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                CMyString name = getInputWithLimit("����γ����� (���10���ַ�): ", 10);
                system.insertCourse(id, name);
                break;
            }
            case 3: {
                int studentId, courseId;
                float score;
                std::cout << "����ѧ��ID: ";
                std::cin >> studentId;
                std::cout << "����γ�ID: ";
                std::cin >> courseId;
                std::cout << "����ɼ�: ";
                std::cin >> score;
                system.insertGrade(studentId, courseId, score);
                break;
            }
            case 4: {
                int subChoice;
                std::cout << "1. ��ID��ѯѧ��\n";
                std::cout << "2. ��������ѯѧ��\n";
                std::cout << "��ѡ���ѯ��ʽ: ";
                std::cin >> subChoice;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                if (subChoice == 1) {
                    int id;
                    std::cout << "����ѧ��ID: ";
                    std::cin >> id;
                    system.queryStudent(id, CMyString());
                } else if (subChoice == 2) {
                    CMyString name = getInputWithLimit("����ѧ������: ", 10);
                    system.queryStudent(-1, name);
                } else {
                    std::cout << "��Ч��ѡ��\n";
                }
                break;
            }
            case 5: {
                int subChoice;
                std::cout << "1. ��ID��ѯ�γ�\n";
                std::cout << "2. �����Ʋ�ѯ�γ�\n";
              /*  std::cout << "3. ��ѯ�γ�ѡ�����\n";*/
                std::cout << "��ѡ���ѯ��ʽ: ";
                std::cin >> subChoice;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                if (subChoice == 1) {
                    int id;
                    std::cout << "����γ�ID: ";
                    std::cin >> id;
                    system.queryCourse(id, CMyString());
                } else if (subChoice == 2) {
                    CMyString name = getInputWithLimit("����γ�����: ", 10);
                    system.queryCourse(-1, name);
                }
                else if (subChoice == 3) {
                    int courseId;
                    std::cout << "����γ�ID: ";
                    std::cin >> courseId;
                  /*  system.queryStudentsByCourseId(courseId);*/
                } 
                else {
                    std::cout << "��Ч��ѡ��\n";
                }
                break;
            }
            case 6: {
                int subChoice;
                std::cout << "1. ��ѯָ��ѧ�������гɼ�\n";
                std::cout << "2. ��ѯָ���γ̵����гɼ�\n";
              /*  std::cout << "3. ��ѯָ��ѧ����ָ���γ̳ɼ�\n";*/
                std::cout << "��ѡ���ѯ��ʽ: ";
                std::cin >> subChoice;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                if (subChoice == 1) {
                    int studentId;
                    std::cout << "����ѧ��ID: ";
                    std::cin >> studentId;
                    system.queryGrade(studentId, -1);
                } else if (subChoice == 2) {
                    int courseId;
                    std::cout << "����γ�ID: ";
                    std::cin >> courseId;
                    system.queryGrade(-1, courseId);
                } else if (subChoice == 3) {
                    int studentId, courseId;
                    std::cout << "����ѧ��ID: ";
                    std::cin >> studentId;
                    std::cout << "����γ�ID: ";
                    std::cin >> courseId;
                    system.queryGrade(studentId, courseId);
                } else {
                    std::cout << "��Ч��ѡ��\n";
                }
                break;
            }
            case 7: {
                int id;
                std::cout << "����ѧ��ID: ";
                std::cin >> id;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // ������뻺��
                CMyString newName = getInputWithLimit("�����µ�ѧ������ (���10���ַ�): ", 10);
                system.modifyStudent(id, newName);
                break;
            }
            case 8: {
                int id;
                std::cout << "����γ�ID: ";
                std::cin >> id;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // ������뻺��
                CMyString newName = getInputWithLimit("�����µĿγ����� (���10���ַ�): ", 10);
                system.modifyCourse(id, newName);
                break;
            }
            case 9: {
                int studentId, courseId;
                float newScore;
                std::cout << "����ѧ��ID: ";
                std::cin >> studentId;
                std::cout << "����γ�ID: ";
                std::cin >> courseId;
                std::cout << "�����µĳɼ�: ";
                std::cin >> newScore;
                system.modifyGrade(studentId, courseId, newScore);
                break;
            }
            case 10: {
                int id;
                std::cout << "����Ҫɾ����ѧ��ID: ";
                std::cin >> id;
                system.removeStudentById(id);
                break;
            }
            case 11: {
                int id;
                std::cout << "����Ҫɾ���Ŀγ�ID: ";
                std::cin >> id;
                system.removeCourseById(id);
                break;
            }
            case 12: {
                int studentId, courseId;
                std::cout << "����ѧ��ID: ";
                std::cin >> studentId;
                std::cout << "����γ�ID: ";
                std::cin >> courseId;
                system.removeGrade(studentId, courseId);
                break;
            }
            case 13:
                system.displayAllData();
                break;
            case 14:
                system.clearAllData();
                std::cout << "������գ����򼴽��˳���" << std::endl;
                return 0;
            case 15:
                system.generateTestData();
                break;
            case 16:
                system.insertSampleData();
                break;
            case 17:
                system.PrintStudentTreeByName();
                break;
            case 18:
                system.PrintCourseTreeByName();
                break;
          /*  case 19:
                system.PrintGradeTree();
                break;*/
            case 0:
                std::cout << "��лʹ��,�ټ�!\n";
                break;
            default:
                std::cout << "��Ч��ѡ��,����������\n";
        }
    } while (choice != 0);

    return 0;
}
