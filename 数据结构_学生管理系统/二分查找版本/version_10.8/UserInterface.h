#pragma once
#include "StudentManagementSystem.h"

class UserInterface {
private:
    StudentManagementSystem& m_system;

    void displayMenu() const;
    void handleUserInput();

    void addStudent();
    void addCourse();
    void addGrade();
    void queryStudent();
    void queryCourse();
    void queryGrade();
    void deleteStudent();
    void deleteCourse();
    void deleteGrade();

    void printAllInformation();

    // 添加新方法
    void formatAndClearData();

    void generateLargeDataset();

    // 新增修改方法
    void modifyStudentName();
    void modifyCourseName();
    void modifyGrade();
    
public:
    UserInterface(StudentManagementSystem& system) : m_system(system) {}
    void run();
    void runAutomatedTest();
};