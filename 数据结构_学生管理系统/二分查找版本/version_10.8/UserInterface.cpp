#include "UserInterface.h"
#include <iostream>
#include "StudentManagementException.h"

void UserInterface::displayMenu() const {
    std::cout << "\n学生管理系统\n";
    std::cout << "1. 添加学生\n";
    std::cout << "2. 添加课程\n";
    std::cout << "3. 添加成绩\n";
    std::cout << "4. 查询学生\n";
    std::cout << "5. 查询课程\n";
    std::cout << "6. 查询成绩\n";
    std::cout << "7. 删除学生\n";
    std::cout << "8. 删除课程\n";
    std::cout << "9. 删除成绩\n";
    std::cout << "10. 打印所有信息\n";
    std::cout << "11. 格式化并清空所有数据\n";  // 新选项
    std::cout << "12. 生成大数据集\n";  // 新选项
    std::cout << "13. 修改学生姓名\n";  // 新选项
    std::cout << "14. 修改课程名称\n";  // 新选项
    std::cout << "15. 修改成绩\n";  // 新选项
    std::cout << "0. 退出\n";
    std::cout << "请选择操作: ";
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
        case 11: formatAndClearData(); break;  // 新选项
        case 12: generateLargeDataset(); break;  // 新选项
        case 13: modifyStudentName(); break;  // 新选项
        case 14: modifyCourseName(); break;  // 新选项
        case 15: modifyGrade(); break;  // 新选项
        case 0: exit(0);
        default: std::cout << "无效的选择,请重试\n";
    }
}

void UserInterface::addStudent() {
    int id;
    CMyString name;
    char nameBuffer[1024];
    std::cout << "请输入学生ID: ";
    std::cin >> id;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // 清除输入缓冲区
    std::cout << "请输入学生姓名: ";
    std::cin.getline(nameBuffer, 1024);
    name = CMyString(nameBuffer);
    try {
        m_system.insertStudent(id, name);
        std::cout << "学生添加成功\n";
    } catch (const StudentManagementException& e) {
        std::cout << "错误: " << e.what() << std::endl;
    }
}

void UserInterface::addCourse() {
    int id;
    CMyString name;
    std::cout << "请输入课程ID: ";
    std::cin >> id;
    std::cout << "请输入课程名称: ";
    std::cin >> name;
    try {
        m_system.insertCourse(id, name);
        std::cout << "课程添加成功\n";
    } catch (const StudentManagementException& e) {
        std::cout << "错误: " << e.what() << std::endl;
    }
}

void UserInterface::addGrade() {
    int studentId, courseId;
    float score;
    std::cout << "请输入学生ID: ";
    std::cin >> studentId;
    std::cout << "请输入课程ID: ";
    std::cin >> courseId;
    std::cout << "请输入成绩: ";
    std::cin >> score;
    try {
        m_system.insertGrade(studentId, courseId, score);
      
    } catch (const StudentManagementException& e) {
        std::cout << "错误: " << e.what() << std::endl;
    }
}

void UserInterface::queryStudent() {
    int choice;
    std::cout << "1. 通过ID查询学生\n";
    std::cout << "2. 通过姓名查询学生\n";
    std::cout << "请选择查询方式: ";
    std::cin >> choice;

    if (choice == 1) {
        int id;
        std::cout << "请输入学生ID: ";
        std::cin >> id;
        CMyString name = m_system.queryStudentNameById(id);
        if (!name.empty()) {
            std::cout << "学生姓名: " << name.c_str() << std::endl;
        } else {
            std::cout << "未找到该学生\n";
        }
    } else if (choice == 2) {
        CMyString name;
        std::cout << "请输入学生姓名: ";
        std::cin >> name;
        CVector<int> ids = m_system.queryStudentIdByName(name);
        if (!ids.empty()) {
            std::cout << "找到以下学生ID:\n";
            for (int i = 0; i < ids.size(); ++i) {
                std::cout << ids[i] << std::endl;
            }
        } else {
            std::cout << "未找到该学生\n";
        }
    } else {
        std::cout << "无效的选择\n";
    }
}

void UserInterface::queryCourse() {
    int choice;
    std::cout << "1. 通过ID查询课程\n";
    std::cout << "2. 通过名称查询课程\n";
    std::cout << "请选择查询方式: ";
    std::cin >> choice;

    if (choice == 1) {
        int id;
        std::cout << "请输入课程ID: ";
        std::cin >> id;
        CMyString name = m_system.queryCourseNameById(id);
        if (!name.empty()) {
            std::cout << "课程名称: " << name.c_str() << std::endl;
        } else {
            std::cout << "未找到该课程\n";
        }
    } else if (choice == 2) {
        CMyString name;
        std::cout << "请输入课程名称: ";
        std::cin >> name;
        CVector<int> ids = m_system.queryCourseIdByName(name);
        if (!ids.empty()) {
            std::cout << "找到以下课程ID:\n";
            for (int i = 0; i < ids.size(); ++i) {
                std::cout << ids[i] << std::endl;
            }
        } else {
            std::cout << "未找到该课程\n";
        }
    } else {
        std::cout << "无效的选择\n";
    }
}

void UserInterface::queryGrade() {
    int choice;
    std::cout << "1. 查询学生成绩\n";
    std::cout << "2. 查询课程成绩\n";
    std::cout << "请选择查询方式: ";
    std::cin >> choice;

    if (choice == 1) {
        int studentId;
        std::cout << "请输入学生ID: ";
        std::cin >> studentId;
        CVector<Grade> grades = m_system.queryGradesByStudentId(studentId);
        if (!grades.empty()) {
            std::cout << "学生成绩如下:\n";
            for (int i = 0; i < grades.size(); ++i) {
                std::cout << "课程ID: " << grades[i].getCourseId() 
                          << ", 成绩: " << grades[i].getScore() << std::endl;
            }
        } else {
            std::cout << "未找到该学生的成绩\n";
        }
    } else if (choice == 2) {
        int courseId;
        std::cout << "请输入课程ID: ";
        std::cin >> courseId;
        CVector<Grade> grades = m_system.queryGradesByCourseId(courseId);
        if (!grades.empty()) {
            std::cout << "课程成绩如下:\n";
            for (int i = 0; i < grades.size(); ++i) {
                std::cout << "学生ID: " << grades[i].getStudentId() 
                          << ", 成绩: " << grades[i].getScore() << std::endl;
            }
        } else {
            std::cout << "未找到该课程的成绩\n";
        }
    } else {
        std::cout << "无效的选择\n";
    }
}

void UserInterface::deleteStudent() {
    int id;
    std::cout << "请输入要删除的学生ID: ";
    std::cin >> id;
    try {
        m_system.deleteStudentById(id);
        std::cout << "学生删除成功\n";
    } catch (const StudentManagementException& e) {
        std::cout << "错误: " << e.what() << std::endl;
    }
}

void UserInterface::deleteCourse() {
    int id;
    std::cout << "请输入要删除的课程ID: ";
    std::cin >> id;
    try {
        m_system.deleteCourseById(id);
        std::cout << "课程删除成功\n";
    } catch (const StudentManagementException& e) {
        std::cout << "错误: " << e.what() << std::endl;
    }
}

void UserInterface::deleteGrade() {
    int studentId, courseId;
    std::cout << "请输入学生ID: ";
    std::cin >> studentId;
    std::cout << "请输入课程ID: ";
    std::cin >> courseId;
    try {
        m_system.deleteGrade(studentId, courseId);
        std::cout << "成绩删除成功\n";
    } catch (const StudentManagementException& e) {
        std::cout << "错误: " << e.what() << std::endl;
    }
}

void UserInterface::printAllInformation() {
    m_system.printAllInformation();
}

void UserInterface::formatAndClearData() {
    char confirm;
    std::cout << "警告：此操作将清空所有数据！是否继续？(y/n): ";
    std::cin >> confirm;
    if (confirm == 'y' || confirm == 'Y') {
        m_system.formatAndClearData();
        std::cout << "数据已被清空，系统已重置。" << std::endl;
    } else {
        std::cout << "操作已取消。" << std::endl;
    }
}

void UserInterface::generateLargeDataset() {
    char confirm;
    std::cout << "警告：此操作将生成大量数据，可能需要一些时间。是否继续？(y/n): ";
    std::cin >> confirm;
    if (confirm == 'y' || confirm == 'Y') {
        m_system.generateLargeDataset();
        std::cout << "大数据集已生成。" << std::endl;
    } else {
        std::cout << "操作已取消。" << std::endl;
    }
}

void UserInterface::modifyStudentName() {
    int id;
    CMyString newName;
    std::cout << "请输入要修改的学生ID: ";
    std::cin >> id;
    std::cout << "请输入新的学生姓名: ";
    std::cin >> newName;
    try {
        m_system.modifyStudentName(id, newName);
        std::cout << "学生姓名修改成功\n";
    } catch (const StudentManagementException& e) {
        std::cout << "错误: " << e.what() << std::endl;
    }
}

void UserInterface::modifyCourseName() {
    int id;
    CMyString newName;
    std::cout << "请输入要修改的课程ID: ";
    std::cin >> id;
    std::cout << "请输入新的课程名称: ";
    std::cin >> newName;
    try {
        m_system.modifyCourseName(id, newName);
        std::cout << "课程名称修改成功\n";
    } catch (const StudentManagementException& e) {
        std::cout << "错误: " << e.what() << std::endl;
    }
}

void UserInterface::modifyGrade() {
    int studentId, courseId;
    float newScore;
    std::cout << "请输入学生ID: ";
    std::cin >> studentId;
    std::cout << "请输入课程ID: ";
    std::cin >> courseId;
    std::cout << "请输入新的成绩: ";
    std::cin >> newScore;
    try {
        m_system.modifyGrade(studentId, courseId, newScore);
        std::cout << "成绩修改成功\n";
    } catch (const StudentManagementException& e) {
        std::cout << "错误: " << e.what() << std::endl;
    }
}

void UserInterface::runAutomatedTest() {
    std::cout << "开始自动化测试...\n\n";

    // 2.1 增加
    std::cout << "2.1 增加测试\n";

    // (1) 新增学生
    std::cout << "(1) 新增学生测试\n";
    try { m_system.insertStudent(1, "王小二"); }
    catch (const StudentManagementException& e) { std::cout << "预期的错误: " << e.what() << std::endl; }
    m_system.insertStudent(100001, "王小二");
    try { m_system.insertStudent(100002, "李四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四四"); }
    catch (const StudentManagementException& e) { std::cout << "预期的错误: " << e.what() << std::endl; }
    m_system.insertStudent(100002, "李四四四四");
    m_system.insertStudent(100003, "王小二");

    std::cout << "查询ID 100001, 100002, 100003:\n";
    std::cout << m_system.queryStudentNameById(100001) << ", "
        << m_system.queryStudentNameById(100002) << ", "
        << m_system.queryStudentNameById(100003) << std::endl;

    std::cout << "查询名字 王小二, 李四四四四:\n";
    auto ids1 = m_system.queryStudentIdByName("王小二");
    auto ids2 = m_system.queryStudentIdByName("李四四四四");
    for (const auto& id : ids1) std::cout << id << " ";
    std::cout << std::endl;
    for (const auto& id : ids2) std::cout << id << " ";
    std::cout << std::endl;

    // (2) 新增课程
    std::cout << "\n(2) 新增课程测试\n";
    try { m_system.insertCourse(1, "C++"); }
    catch (const StudentManagementException& e) { std::cout << "预期的错误: " << e.what() << std::endl; }
    m_system.insertCourse(1001, "C++");
    m_system.insertCourse(1002, "16位微机");
    m_system.insertCourse(1003, "C++");

    std::cout << "查询ID 1001, 1002, 1003:\n";
    std::cout << m_system.queryCourseNameById(1001) << ", "
        << m_system.queryCourseNameById(1002) << ", "
        << m_system.queryCourseNameById(1003) << std::endl;

    std::cout << "查询名字 C++, 16位微机:\n";
    auto cids1 = m_system.queryCourseIdByName("C++");
    auto cids2 = m_system.queryCourseIdByName("16位微机");
    for (const auto& id : cids1) std::cout << id << " ";
    std::cout << std::endl;
    for (const auto& id : cids2) std::cout << id << " ";
    std::cout << std::endl;

    // (3) 新增选课记录
    std::cout << "\n(3) 新增选课记录测试\n";
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
    catch (const StudentManagementException& e) { std::cout << "预期的错误: " << e.what() << std::endl; }

    std::cout << "查询学生ID 100001, 100002, 100003 选课情况:\n";
    (m_system.queryGradesByStudentId(100001));
    (m_system.queryGradesByStudentId(100002));
    (m_system.queryGradesByStudentId(100003));

    std::cout << "查询课程ID 1001, 1002, 1003 选课情况:\n";
    (m_system.queryGradesByCourseId(1001));
    (m_system.queryGradesByCourseId(1002));
    (m_system.queryGradesByCourseId(1003));

    std::cout << "查询课程名C++ 考试情况:\n";
    auto cppIds = m_system.queryCourseIdByName("C++");
    for (const auto& id : cppIds) {
       (m_system.queryGradesByCourseId(id));
    }

    // 2.2 修改
    std::cout << "\n2.2 修改测试\n";

    // (1) 修改学生姓名
    std::cout << "(1) 修改学生姓名测试\n";
    m_system.modifyStudentName(100002, "李四");
    std::cout << "查询ID 100002: " << m_system.queryStudentNameById(100002) << std::endl;
    std::cout << "查询名字 李四:\n";
    auto ids3 = m_system.queryStudentIdByName("李四");
    for (const auto& id : ids3) std::cout << id << " ";
    std::cout << std::endl;

    // (2) 修改课程名称
    std::cout << "\n(2) 修改课程名称测试\n";
    m_system.modifyCourseName(1002, "64位微机");
    std::cout << "查询ID 1002: " << m_system.queryCourseNameById(1002) << std::endl;
    std::cout << "查询名字 64位微机:\n";
    auto cids3 = m_system.queryCourseIdByName("64位微机");
    for (const auto& id : cids3) std::cout << id << " ";
    std::cout << std::endl;

    // (3) 修改选课分数
    std::cout << "\n(3) 修改选课分数测试\n";
    m_system.modifyGrade(100001, 1001, 150);
    std::cout << "查询学生ID 100001 选课记录:\n";
    (m_system.queryGradesByStudentId(100001));
    std::cout << "查询课程ID 1001 选课记录:\n";
    (m_system.queryGradesByCourseId(1001));

    // 2.3 删除
    std::cout << "\n2.3 删除测试\n";

    // (1) 删除学生
    std::cout << "(1) 删除学生测试\n";
    m_system.deleteStudentById(100001);
    try { m_system.deleteStudentById(100001); }
    catch (const StudentManagementException& e) { std::cout << "预期的错误: " << e.what() << std::endl; }

    std::cout << "查询学生ID 100001: " << m_system.queryStudentNameById(100001) << std::endl;
    std::cout << "查询学生姓名 王小二:\n";
    auto ids4 = m_system.queryStudentIdByName("王小二");
    for (const auto& id : ids4) std::cout << id << " ";
    std::cout << std::endl;

    std::cout << "查询课程ID 1001, 1002, 1003 选课情况:\n";
    (m_system.queryGradesByCourseId(1001));
    (m_system.queryGradesByCourseId(1002));
    (m_system.queryGradesByCourseId(1003));

    // (2) 删除课程
    std::cout << "\n(2) 删除课程测试\n";
    m_system.deleteCourseById(1001);
    try { m_system.deleteCourseById(1001); }
    catch (const StudentManagementException& e) { std::cout << "预期的错误: " << e.what() << std::endl; }

    std::cout << "查询课程ID 1001: " << m_system.queryCourseNameById(1001) << std::endl;
    std::cout << "查询课程名C++:\n";
    auto cids4 = m_system.queryCourseIdByName("C++");
    for (const auto& id : cids4) std::cout << id << " ";
    std::cout << std::endl;

    std::cout << "查询学生ID 100002, 100003 选课情况:\n";
    (m_system.queryGradesByStudentId(100002));
    (m_system.queryGradesByStudentId(100003));

    // (3) 删除选课记录
    std::cout << "\n(3) 删除选课记录测试\n";
    m_system.deleteGrade(100002, 1002);
    try { m_system.deleteGrade(100002, 1002); }
    catch (const StudentManagementException& e) { std::cout << "预期的错误: " << e.what() << std::endl; }

    std::cout << "查询学生ID 100002 选课记录:\n";
    (m_system.queryGradesByStudentId(100002));
    std::cout << "查询课程ID 1002 选课记录:\n";
    (m_system.queryGradesByCourseId(1002));

    std::cout << "\n自动化测试完成。\n";
}

