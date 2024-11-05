#include "StudentManagementSystem.h"
#include <iostream>
#include <limits>

void printMenu() {
    std::cout << "\n学生管理系统\n";
    std::cout << "1. 添加学生\n";
    std::cout << "2. 添加课程\n";
    std::cout << "3. 添加成绩\n";
    std::cout << "4. 查询学生信息\n";
    std::cout << "5. 查询课程信息\n";
    std::cout << "6. 查询成绩信息\n";
    std::cout << "7. 修改学生信息\n";
    std::cout << "8. 修改课程信息\n";
    std::cout << "9. 修改成绩\n";
    std::cout << "10. 删除学生\n";
    std::cout << "11. 删除课程\n";
    std::cout << "12. 删除成绩\n";
    std::cout << "13. 显示所有数据\n";
    std::cout << "14. 清空所有数据\n";
    std::cout << "15. 生成测试数据\n";
    std::cout << "16. 插入示例数据\n";
    std::cout << "0. 退出\n";
    std::cout << "请选择操作: ";
}

CMyString getInputWithLimit(const char* prompt, int maxLength) {
    char buffer[1024];  // 假设最大输入长度不超过1024
    while (true) {
        std::cout << prompt;
        std::cin.getline(buffer, sizeof(buffer));
        CMyString input(buffer);
        if (input.length() <= maxLength) {
            return input;
        }
        std::cout << "输入超过" << maxLength << "个字符，请重新输入。" << std::endl;
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
                std::cout << "输入学生ID: ";
                std::cin >> id;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                CMyString name = getInputWithLimit("输入学生姓名 (最多10个字符): ", 10);
                system.insertStudent(id, name);
                break;
            }
            case 2: {
                int id;
                std::cout << "输入课程ID: ";
                std::cin >> id;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                CMyString name = getInputWithLimit("输入课程名称 (最多10个字符): ", 10);
                system.insertCourse(id, name);
                break;
            }
            case 3: {
                int studentId, courseId;
                float score;
                std::cout << "输入学生ID: ";
                std::cin >> studentId;
                std::cout << "输入课程ID: ";
                std::cin >> courseId;
                std::cout << "输入成绩: ";
                std::cin >> score;
                system.insertGrade(studentId, courseId, score);
                break;
            }
            case 4: {
                int subChoice;
                std::cout << "1. 按ID查询学生\n";
                std::cout << "2. 按姓名查询学生\n";
                std::cout << "请选择查询方式: ";
                std::cin >> subChoice;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                if (subChoice == 1) {
                    int id;
                    std::cout << "输入学生ID: ";
                    std::cin >> id;
                    system.queryStudent(id, CMyString());
                } else if (subChoice == 2) {
                    CMyString name = getInputWithLimit("输入学生姓名: ", 10);
                    system.queryStudent(-1, name);
                } else {
                    std::cout << "无效的选择\n";
                }
                break;
            }
            case 5: {
                int subChoice;
                std::cout << "1. 按ID查询课程\n";
                std::cout << "2. 按名称查询课程\n";
              /*  std::cout << "3. 查询课程选课情况\n";*/
                std::cout << "请选择查询方式: ";
                std::cin >> subChoice;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                if (subChoice == 1) {
                    int id;
                    std::cout << "输入课程ID: ";
                    std::cin >> id;
                    system.queryCourse(id, CMyString());
                } else if (subChoice == 2) {
                    CMyString name = getInputWithLimit("输入课程名称: ", 10);
                    system.queryCourse(-1, name);
                }
                else if (subChoice == 3) {
                    int courseId;
                    std::cout << "输入课程ID: ";
                    std::cin >> courseId;
                  /*  system.queryStudentsByCourseId(courseId);*/
                } 
                else {
                    std::cout << "无效的选择\n";
                }
                break;
            }
            case 6: {
                int subChoice;
                std::cout << "1. 查询指定学生的所有成绩\n";
                std::cout << "2. 查询指定课程的所有成绩\n";
              /*  std::cout << "3. 查询指定学生的指定课程成绩\n";*/
                std::cout << "请选择查询方式: ";
                std::cin >> subChoice;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                if (subChoice == 1) {
                    int studentId;
                    std::cout << "输入学生ID: ";
                    std::cin >> studentId;
                    system.queryGrade(studentId, -1);
                } else if (subChoice == 2) {
                    int courseId;
                    std::cout << "输入课程ID: ";
                    std::cin >> courseId;
                    system.queryGrade(-1, courseId);
                } else if (subChoice == 3) {
                    int studentId, courseId;
                    std::cout << "输入学生ID: ";
                    std::cin >> studentId;
                    std::cout << "输入课程ID: ";
                    std::cin >> courseId;
                    system.queryGrade(studentId, courseId);
                } else {
                    std::cout << "无效的选择\n";
                }
                break;
            }
            case 7: {
                int id;
                std::cout << "输入学生ID: ";
                std::cin >> id;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // 清除输入缓冲
                CMyString newName = getInputWithLimit("输入新的学生姓名 (最多10个字符): ", 10);
                system.modifyStudent(id, newName);
                break;
            }
            case 8: {
                int id;
                std::cout << "输入课程ID: ";
                std::cin >> id;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  // 清除输入缓冲
                CMyString newName = getInputWithLimit("输入新的课程名称 (最多10个字符): ", 10);
                system.modifyCourse(id, newName);
                break;
            }
            case 9: {
                int studentId, courseId;
                float newScore;
                std::cout << "输入学生ID: ";
                std::cin >> studentId;
                std::cout << "输入课程ID: ";
                std::cin >> courseId;
                std::cout << "输入新的成绩: ";
                std::cin >> newScore;
                system.modifyGrade(studentId, courseId, newScore);
                break;
            }
            case 10: {
                int id;
                std::cout << "输入要删除的学生ID: ";
                std::cin >> id;
                system.removeStudentById(id);
                break;
            }
            case 11: {
                int id;
                std::cout << "输入要删除的课程ID: ";
                std::cin >> id;
                system.removeCourseById(id);
                break;
            }
            case 12: {
                int studentId, courseId;
                std::cout << "输入学生ID: ";
                std::cin >> studentId;
                std::cout << "输入课程ID: ";
                std::cin >> courseId;
                system.removeGrade(studentId, courseId);
                break;
            }
            case 13:
                system.displayAllData();
                break;
            case 14:
                system.clearAllData();
                std::cout << "数据清空，程序即将退出。" << std::endl;
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
                std::cout << "感谢使用,再见!\n";
                break;
            default:
                std::cout << "无效的选择,请重新输入\n";
        }
    } while (choice != 0);

    return 0;
}
