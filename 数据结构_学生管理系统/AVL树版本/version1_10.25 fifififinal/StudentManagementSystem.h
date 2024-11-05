#pragma once
#include "CBinarySearchTree_List.h"
#include "Student.h"
#include "Course.h"
#include "Grade.h"
#include "CMyString.h"
#include <fstream>
#include "CVector.h"

class StudentManagementSystem {
private:
    CAVLTree<Pair<int, Student>> studentById;
    CAVLTree<Pair<CMyString, Student>> studentByName;
    CAVLTree<Pair<int, Course>> courseById;
    CAVLTree<Pair<CMyString, Course>> courseByName;
    CAVLTree<Pair<int, Grade>> gradeByStudentId;
    CAVLTree<Pair<int, Grade>> gradeByCourseId;

    // 添加以下私有辅助函数声明
    void saveStudentNode(std::ofstream& file, CAVLTree<Pair<int, Student>>::TreeNode* node);
    void saveCourseNode(std::ofstream& file, CAVLTree<Pair<int, Course>>::TreeNode* node);
    void saveGradeNode(std::ofstream& file, CAVLTree<Pair<int, Grade>>::TreeNode* node);
    //void saveGradeData(std::ofstream& file, CAVLTree<Pair<int, Grade>>::TreeNode* node);
    void loadStudentData(const char* filename);
    void loadCourseData(const char* filename);
    void loadGradeData(const char* filename);

    // 新增：自动保存数据
    void autoSave();

    // 新增：显示所有数据
    void displayStudentNode(CAVLTree<Pair<int, Student>>::TreeNode* node);
    void displayCourseNode(CAVLTree<Pair<int, Course>>::TreeNode* node);
    void displayGradeNode(CAVLTree<Pair<int, Grade>>::TreeNode* node);

public:
    // 构造函数，在创建对象时自动加载数据
    StudentManagementSystem();

    // 查询功能
    Student* queryStudentById(int id);
    Student* queryStudentByName(const CMyString& name);
    Course* queryCourseById(int id);
    Course* queryCourseByName(const CMyString& name);
    void queryStudentsByCourseId(int courseId);
    void queryCoursesByStudentId(int studentId);

    // 添加以下两个新的查询函数
    CVector<Student*> queryStudentsByName(const CMyString& name);
    CVector<Course*> queryCoursesByName(const CMyString& name);

    // 插入功能
    void insertStudent(int id, const CMyString& name);
    void insertCourse(int id, const CMyString& name);
    void insertGrade(int studentId, int courseId, float score);

    // 删除功能
    void removeStudentById(int id);
    void removeCourseById(int id);
    void removeGrade(int studentId, int courseId);

    // 修改功能
    void modifyStudent(int id, const CMyString& newName);
    void modifyCourse(int id, const CMyString& newName);
    void modifyGrade(int studentId, int courseId, float newScore);

    // 文件操作
    void saveToFile();
    void loadFromFile();

    // 新增：清空所有数据
    void clearAllData();

    // 新增：显示所有数据
    void displayAllData();

    // 修改查询功能
    void queryStudent(int id, const CMyString& name);
    void queryCourse(int id, const CMyString& name);
    void queryGrade(int studentId, int courseId);



    // 添加新方法用于生成大量测试数据
    void generateTestData();

    void insertSampleData();
public:
    void PrintStudentTreeByName();
    void PrintCourseTreeByName();

private:
    void PrintTreeByName(CAVLTree<Pair<CMyString, Student>>::TreeNode* node, int depth);
    void PrintTreeByName(CAVLTree<Pair<CMyString, Course>>::TreeNode* node, int depth);
};
