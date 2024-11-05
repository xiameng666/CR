#include "UserInterface.h"
#include <iostream>
#include "StudentManagementException.h"

void UserInterface::displayMenu() const {
    std::cout << "\nѧ������ϵͳ\n";
    std::cout << "1. ���ѧ��\n";
    std::cout << "2. ��ӿγ�\n";
    std::cout << "3. ��ӳɼ�\n";
    std::cout << "4. ��ѯѧ��\n";
    std::cout << "5. ��ѯ�γ�\n";
    std::cout << "6. ��ѯ�ɼ�\n";
    std::cout << "7. ɾ��ѧ��\n";
    std::cout << "8. ɾ���γ�\n";
    std::cout << "9. ɾ���ɼ�\n";
    std::cout << "10. ��ӡ������Ϣ\n";
    std::cout << "11. ��ʽ���������������\n";  // ��ѡ��
    std::cout << "12. ���ɴ����ݼ�\n";  // ��ѡ��
    std::cout << "13. �޸�ѧ������\n";  // ��ѡ��
    std::cout << "14. �޸Ŀγ�����\n";  // ��ѡ��
    std::cout << "15. �޸ĳɼ�\n";  // ��ѡ��
    std::cout << "0. �˳�\n";
    std::cout << "��ѡ�����: ";
}

void UserInterface::run() {
    while (true) {
        displayMenu();
        handleUserInput();
    }
}

void UserInterface::handleUserInput() {
    int choice;
    std::cin >> choice;
    switch (choice) {
        case 1: addStudent(); break;
        case 2: addCourse(); break;
        case 3: addGrade(); break;
        case 4: queryStudent(); break;
        case 5: queryCourse(); break;
        case 6: queryGrade(); break;
        case 7: deleteStudent(); break;
        case 8: deleteCourse(); break;
        case 9: deleteGrade(); break;
        case 10: printAllInformation(); break;
        case 11: formatAndClearData(); break;  // ��ѡ��
        case 12: generateLargeDataset(); break;  // ��ѡ��
        case 13: modifyStudentName(); break;  // ��ѡ��
        case 14: modifyCourseName(); break;  // ��ѡ��
        case 15: modifyGrade(); break;  // ��ѡ��
        case 0: exit(0);
        default: std::cout << "��Ч��ѡ��,������\n";
    }
}

void UserInterface::addStudent() {
    int id;
    CMyString name;
    char nameBuffer[1024];
    std::cout << "������ѧ��ID: ";
    std::cin >> id;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // ������뻺����
    std::cout << "������ѧ������: ";
    std::cin.getline(nameBuffer, 1024);
    name = CMyString(nameBuffer);
    try {
        m_system.insertStudent(id, name);
        std::cout << "ѧ����ӳɹ�\n";
    } catch (const StudentManagementException& e) {
        std::cout << "����: " << e.what() << std::endl;
    }
}

void UserInterface::addCourse() {
    int id;
    CMyString name;
    std::cout << "������γ�ID: ";
    std::cin >> id;
    std::cout << "������γ�����: ";
    std::cin >> name;
    try {
        m_system.insertCourse(id, name);
        std::cout << "�γ���ӳɹ�\n";
    } catch (const StudentManagementException& e) {
        std::cout << "����: " << e.what() << std::endl;
    }
}

void UserInterface::addGrade() {
    int studentId, courseId;
    float score;
    std::cout << "������ѧ��ID: ";
    std::cin >> studentId;
    std::cout << "������γ�ID: ";
    std::cin >> courseId;
    std::cout << "������ɼ�: ";
    std::cin >> score;
    try {
        m_system.insertGrade(studentId, courseId, score);
      
    } catch (const StudentManagementException& e) {
        std::cout << "����: " << e.what() << std::endl;
    }
}

void UserInterface::queryStudent() {
    int choice;
    std::cout << "1. ͨ��ID��ѯѧ��\n";
    std::cout << "2. ͨ��������ѯѧ��\n";
    std::cout << "��ѡ���ѯ��ʽ: ";
    std::cin >> choice;

    if (choice == 1) {
        int id;
        std::cout << "������ѧ��ID: ";
        std::cin >> id;
        CMyString name = m_system.queryStudentNameById(id);
        if (!name.empty()) {
            std::cout << "ѧ������: " << name.c_str() << std::endl;
        } else {
            std::cout << "δ�ҵ���ѧ��\n";
        }
    } else if (choice == 2) {
        CMyString name;
        std::cout << "������ѧ������: ";
        std::cin >> name;
        CVector<int> ids = m_system.queryStudentIdByName(name);
        if (!ids.empty()) {
            std::cout << "�ҵ�����ѧ��ID:\n";
            for (int i = 0; i < ids.size(); ++i) {
                std::cout << ids[i] << std::endl;
            }
        } else {
            std::cout << "δ�ҵ���ѧ��\n";
        }
    } else {
        std::cout << "��Ч��ѡ��\n";
    }
}

void UserInterface::queryCourse() {
    int choice;
    std::cout << "1. ͨ��ID��ѯ�γ�\n";
    std::cout << "2. ͨ�����Ʋ�ѯ�γ�\n";
    std::cout << "��ѡ���ѯ��ʽ: ";
    std::cin >> choice;

    if (choice == 1) {
        int id;
        std::cout << "������γ�ID: ";
        std::cin >> id;
        CMyString name = m_system.queryCourseNameById(id);
        if (!name.empty()) {
            std::cout << "�γ�����: " << name.c_str() << std::endl;
        } else {
            std::cout << "δ�ҵ��ÿγ�\n";
        }
    } else if (choice == 2) {
        CMyString name;
        std::cout << "������γ�����: ";
        std::cin >> name;
        CVector<int> ids = m_system.queryCourseIdByName(name);
        if (!ids.empty()) {
            std::cout << "�ҵ����¿γ�ID:\n";
            for (int i = 0; i < ids.size(); ++i) {
                std::cout << ids[i] << std::endl;
            }
        } else {
            std::cout << "δ�ҵ��ÿγ�\n";
        }
    } else {
        std::cout << "��Ч��ѡ��\n";
    }
}

void UserInterface::queryGrade() {
    int choice;
    std::cout << "1. ��ѯѧ���ɼ�\n";
    std::cout << "2. ��ѯ�γ̳ɼ�\n";
    std::cout << "��ѡ���ѯ��ʽ: ";
    std::cin >> choice;

    if (choice == 1) {
        int studentId;
        std::cout << "������ѧ��ID: ";
        std::cin >> studentId;
        CVector<Grade> grades = m_system.queryGradesByStudentId(studentId);
        if (!grades.empty()) {
            std::cout << "ѧ���ɼ�����:\n";
            for (int i = 0; i < grades.size(); ++i) {
                std::cout << "�γ�ID: " << grades[i].getCourseId() 
                          << ", �ɼ�: " << grades[i].getScore() << std::endl;
            }
        } else {
            std::cout << "δ�ҵ���ѧ���ĳɼ�\n";
        }
    } else if (choice == 2) {
        int courseId;
        std::cout << "������γ�ID: ";
        std::cin >> courseId;
        CVector<Grade> grades = m_system.queryGradesByCourseId(courseId);
        if (!grades.empty()) {
            std::cout << "�γ̳ɼ�����:\n";
            for (int i = 0; i < grades.size(); ++i) {
                std::cout << "ѧ��ID: " << grades[i].getStudentId() 
                          << ", �ɼ�: " << grades[i].getScore() << std::endl;
            }
        } else {
            std::cout << "δ�ҵ��ÿγ̵ĳɼ�\n";
        }
    } else {
        std::cout << "��Ч��ѡ��\n";
    }
}

void UserInterface::deleteStudent() {
    int id;
    std::cout << "������Ҫɾ����ѧ��ID: ";
    std::cin >> id;
    try {
        m_system.deleteStudentById(id);
        std::cout << "ѧ��ɾ���ɹ�\n";
    } catch (const StudentManagementException& e) {
        std::cout << "����: " << e.what() << std::endl;
    }
}

void UserInterface::deleteCourse() {
    int id;
    std::cout << "������Ҫɾ���Ŀγ�ID: ";
    std::cin >> id;
    try {
        m_system.deleteCourseById(id);
        std::cout << "�γ�ɾ���ɹ�\n";
    } catch (const StudentManagementException& e) {
        std::cout << "����: " << e.what() << std::endl;
    }
}

void UserInterface::deleteGrade() {
    int studentId, courseId;
    std::cout << "������ѧ��ID: ";
    std::cin >> studentId;
    std::cout << "������γ�ID: ";
    std::cin >> courseId;
    try {
        m_system.deleteGrade(studentId, courseId);
        std::cout << "�ɼ�ɾ���ɹ�\n";
    } catch (const StudentManagementException& e) {
        std::cout << "����: " << e.what() << std::endl;
    }
}

void UserInterface::printAllInformation() {
    m_system.printAllInformation();
}

void UserInterface::formatAndClearData() {
    char confirm;
    std::cout << "���棺�˲���������������ݣ��Ƿ������(y/n): ";
    std::cin >> confirm;
    if (confirm == 'y' || confirm == 'Y') {
        m_system.formatAndClearData();
        std::cout << "�����ѱ���գ�ϵͳ�����á�" << std::endl;
    } else {
        std::cout << "������ȡ����" << std::endl;
    }
}

void UserInterface::generateLargeDataset() {
    char confirm;
    std::cout << "���棺�˲��������ɴ������ݣ�������ҪһЩʱ�䡣�Ƿ������(y/n): ";
    std::cin >> confirm;
    if (confirm == 'y' || confirm == 'Y') {
        m_system.generateLargeDataset();
        std::cout << "�����ݼ������ɡ�" << std::endl;
    } else {
        std::cout << "������ȡ����" << std::endl;
    }
}

void UserInterface::modifyStudentName() {
    int id;
    CMyString newName;
    std::cout << "������Ҫ�޸ĵ�ѧ��ID: ";
    std::cin >> id;
    std::cout << "�������µ�ѧ������: ";
    std::cin >> newName;
    try {
        m_system.modifyStudentName(id, newName);
        std::cout << "ѧ�������޸ĳɹ�\n";
    } catch (const StudentManagementException& e) {
        std::cout << "����: " << e.what() << std::endl;
    }
}

void UserInterface::modifyCourseName() {
    int id;
    CMyString newName;
    std::cout << "������Ҫ�޸ĵĿγ�ID: ";
    std::cin >> id;
    std::cout << "�������µĿγ�����: ";
    std::cin >> newName;
    try {
        m_system.modifyCourseName(id, newName);
        std::cout << "�γ������޸ĳɹ�\n";
    } catch (const StudentManagementException& e) {
        std::cout << "����: " << e.what() << std::endl;
    }
}

void UserInterface::modifyGrade() {
    int studentId, courseId;
    float newScore;
    std::cout << "������ѧ��ID: ";
    std::cin >> studentId;
    std::cout << "������γ�ID: ";
    std::cin >> courseId;
    std::cout << "�������µĳɼ�: ";
    std::cin >> newScore;
    try {
        m_system.modifyGrade(studentId, courseId, newScore);
        std::cout << "�ɼ��޸ĳɹ�\n";
    } catch (const StudentManagementException& e) {
        std::cout << "����: " << e.what() << std::endl;
    }
}

void UserInterface::runAutomatedTest() {
    std::cout << "��ʼ�Զ�������...\n\n";

    // 2.1 ����
    std::cout << "2.1 ���Ӳ���\n";

    // (1) ����ѧ��
    std::cout << "(1) ����ѧ������\n";
    try { m_system.insertStudent(1, "��С��"); }
    catch (const StudentManagementException& e) { std::cout << "Ԥ�ڵĴ���: " << e.what() << std::endl; }
    m_system.insertStudent(100001, "��С��");
    try { m_system.insertStudent(100002, "������������������������������������������������������������������������������������������������������"); }
    catch (const StudentManagementException& e) { std::cout << "Ԥ�ڵĴ���: " << e.what() << std::endl; }
    m_system.insertStudent(100002, "����������");
    m_system.insertStudent(100003, "��С��");

    std::cout << "��ѯID 100001, 100002, 100003:\n";
    std::cout << m_system.queryStudentNameById(100001) << ", "
        << m_system.queryStudentNameById(100002) << ", "
        << m_system.queryStudentNameById(100003) << std::endl;

    std::cout << "��ѯ���� ��С��, ����������:\n";
    auto ids1 = m_system.queryStudentIdByName("��С��");
    auto ids2 = m_system.queryStudentIdByName("����������");
    for (const auto& id : ids1) std::cout << id << " ";
    std::cout << std::endl;
    for (const auto& id : ids2) std::cout << id << " ";
    std::cout << std::endl;

    // (2) �����γ�
    std::cout << "\n(2) �����γ̲���\n";
    try { m_system.insertCourse(1, "C++"); }
    catch (const StudentManagementException& e) { std::cout << "Ԥ�ڵĴ���: " << e.what() << std::endl; }
    m_system.insertCourse(1001, "C++");
    m_system.insertCourse(1002, "16λ΢��");
    m_system.insertCourse(1003, "C++");

    std::cout << "��ѯID 1001, 1002, 1003:\n";
    std::cout << m_system.queryCourseNameById(1001) << ", "
        << m_system.queryCourseNameById(1002) << ", "
        << m_system.queryCourseNameById(1003) << std::endl;

    std::cout << "��ѯ���� C++, 16λ΢��:\n";
    auto cids1 = m_system.queryCourseIdByName("C++");
    auto cids2 = m_system.queryCourseIdByName("16λ΢��");
    for (const auto& id : cids1) std::cout << id << " ";
    std::cout << std::endl;
    for (const auto& id : cids2) std::cout << id << " ";
    std::cout << std::endl;

    // (3) ����ѡ�μ�¼
    std::cout << "\n(3) ����ѡ�μ�¼����\n";
    m_system.insertGrade(100001, 1001, 61);
    m_system.insertGrade(100001, 1002, 62);
    m_system.insertGrade(100001, 1003, 63);
    m_system.insertGrade(100002, 1001, 71);
    m_system.insertGrade(100002, 1002, 72);
    m_system.insertGrade(100002, 1003, 73);
    m_system.insertGrade(100003, 1001, 81);
    m_system.insertGrade(100003, 1002, 82);
    m_system.insertGrade(100003, 1003, 83);
    try { m_system.insertGrade(100003, 1003, 99); }
    catch (const StudentManagementException& e) { std::cout << "Ԥ�ڵĴ���: " << e.what() << std::endl; }

    std::cout << "��ѯѧ��ID 100001, 100002, 100003 ѡ�����:\n";
    (m_system.queryGradesByStudentId(100001));
    (m_system.queryGradesByStudentId(100002));
    (m_system.queryGradesByStudentId(100003));

    std::cout << "��ѯ�γ�ID 1001, 1002, 1003 ѡ�����:\n";
    (m_system.queryGradesByCourseId(1001));
    (m_system.queryGradesByCourseId(1002));
    (m_system.queryGradesByCourseId(1003));

    std::cout << "��ѯ�γ���C++ �������:\n";
    auto cppIds = m_system.queryCourseIdByName("C++");
    for (const auto& id : cppIds) {
       (m_system.queryGradesByCourseId(id));
    }

    // 2.2 �޸�
    std::cout << "\n2.2 �޸Ĳ���\n";

    // (1) �޸�ѧ������
    std::cout << "(1) �޸�ѧ����������\n";
    m_system.modifyStudentName(100002, "����");
    std::cout << "��ѯID 100002: " << m_system.queryStudentNameById(100002) << std::endl;
    std::cout << "��ѯ���� ����:\n";
    auto ids3 = m_system.queryStudentIdByName("����");
    for (const auto& id : ids3) std::cout << id << " ";
    std::cout << std::endl;

    // (2) �޸Ŀγ�����
    std::cout << "\n(2) �޸Ŀγ����Ʋ���\n";
    m_system.modifyCourseName(1002, "64λ΢��");
    std::cout << "��ѯID 1002: " << m_system.queryCourseNameById(1002) << std::endl;
    std::cout << "��ѯ���� 64λ΢��:\n";
    auto cids3 = m_system.queryCourseIdByName("64λ΢��");
    for (const auto& id : cids3) std::cout << id << " ";
    std::cout << std::endl;

    // (3) �޸�ѡ�η���
    std::cout << "\n(3) �޸�ѡ�η�������\n";
    m_system.modifyGrade(100001, 1001, 150);
    std::cout << "��ѯѧ��ID 100001 ѡ�μ�¼:\n";
    (m_system.queryGradesByStudentId(100001));
    std::cout << "��ѯ�γ�ID 1001 ѡ�μ�¼:\n";
    (m_system.queryGradesByCourseId(1001));

    // 2.3 ɾ��
    std::cout << "\n2.3 ɾ������\n";

    // (1) ɾ��ѧ��
    std::cout << "(1) ɾ��ѧ������\n";
    m_system.deleteStudentById(100001);
    try { m_system.deleteStudentById(100001); }
    catch (const StudentManagementException& e) { std::cout << "Ԥ�ڵĴ���: " << e.what() << std::endl; }

    std::cout << "��ѯѧ��ID 100001: " << m_system.queryStudentNameById(100001) << std::endl;
    std::cout << "��ѯѧ������ ��С��:\n";
    auto ids4 = m_system.queryStudentIdByName("��С��");
    for (const auto& id : ids4) std::cout << id << " ";
    std::cout << std::endl;

    std::cout << "��ѯ�γ�ID 1001, 1002, 1003 ѡ�����:\n";
    (m_system.queryGradesByCourseId(1001));
    (m_system.queryGradesByCourseId(1002));
    (m_system.queryGradesByCourseId(1003));

    // (2) ɾ���γ�
    std::cout << "\n(2) ɾ���γ̲���\n";
    m_system.deleteCourseById(1001);
    try { m_system.deleteCourseById(1001); }
    catch (const StudentManagementException& e) { std::cout << "Ԥ�ڵĴ���: " << e.what() << std::endl; }

    std::cout << "��ѯ�γ�ID 1001: " << m_system.queryCourseNameById(1001) << std::endl;
    std::cout << "��ѯ�γ���C++:\n";
    auto cids4 = m_system.queryCourseIdByName("C++");
    for (const auto& id : cids4) std::cout << id << " ";
    std::cout << std::endl;

    std::cout << "��ѯѧ��ID 100002, 100003 ѡ�����:\n";
    (m_system.queryGradesByStudentId(100002));
    (m_system.queryGradesByStudentId(100003));

    // (3) ɾ��ѡ�μ�¼
    std::cout << "\n(3) ɾ��ѡ�μ�¼����\n";
    m_system.deleteGrade(100002, 1002);
    try { m_system.deleteGrade(100002, 1002); }
    catch (const StudentManagementException& e) { std::cout << "Ԥ�ڵĴ���: " << e.what() << std::endl; }

    std::cout << "��ѯѧ��ID 100002 ѡ�μ�¼:\n";
    (m_system.queryGradesByStudentId(100002));
    std::cout << "��ѯ�γ�ID 1002 ѡ�μ�¼:\n";
    (m_system.queryGradesByCourseId(1002));

    std::cout << "\n�Զ���������ɡ�\n";
}

