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

    // �������˽�и�����������
    void saveStudentNode(std::ofstream& file, CAVLTree<Pair<int, Student>>::TreeNode* node);
    void saveCourseNode(std::ofstream& file, CAVLTree<Pair<int, Course>>::TreeNode* node);
    void saveGradeNode(std::ofstream& file, CAVLTree<Pair<int, Grade>>::TreeNode* node);
    //void saveGradeData(std::ofstream& file, CAVLTree<Pair<int, Grade>>::TreeNode* node);
    void loadStudentData(const char* filename);
    void loadCourseData(const char* filename);
    void loadGradeData(const char* filename);

    // �������Զ���������
    void autoSave();

    // ��������ʾ��������
    void displayStudentNode(CAVLTree<Pair<int, Student>>::TreeNode* node);
    void displayCourseNode(CAVLTree<Pair<int, Course>>::TreeNode* node);
    void displayGradeNode(CAVLTree<Pair<int, Grade>>::TreeNode* node);

public:
    // ���캯�����ڴ�������ʱ�Զ���������
    StudentManagementSystem();

    // ��ѯ����
    Student* queryStudentById(int id);
    Student* queryStudentByName(const CMyString& name);
    Course* queryCourseById(int id);
    Course* queryCourseByName(const CMyString& name);
    void queryStudentsByCourseId(int courseId);
    void queryCoursesByStudentId(int studentId);

    // ������������µĲ�ѯ����
    CVector<Student*> queryStudentsByName(const CMyString& name);
    CVector<Course*> queryCoursesByName(const CMyString& name);

    // ���빦��
    void insertStudent(int id, const CMyString& name);
    void insertCourse(int id, const CMyString& name);
    void insertGrade(int studentId, int courseId, float score);

    // ɾ������
    void removeStudentById(int id);
    void removeCourseById(int id);
    void removeGrade(int studentId, int courseId);

    // �޸Ĺ���
    void modifyStudent(int id, const CMyString& newName);
    void modifyCourse(int id, const CMyString& newName);
    void modifyGrade(int studentId, int courseId, float newScore);

    // �ļ�����
    void saveToFile();
    void loadFromFile();

    // �����������������
    void clearAllData();

    // ��������ʾ��������
    void displayAllData();

    // �޸Ĳ�ѯ����
    void queryStudent(int id, const CMyString& name);
    void queryCourse(int id, const CMyString& name);
    void queryGrade(int studentId, int courseId);



    // ����·����������ɴ�����������
    void generateTestData();

    void insertSampleData();
public:
    void PrintStudentTreeByName();
    void PrintCourseTreeByName();

private:
    void PrintTreeByName(CAVLTree<Pair<CMyString, Student>>::TreeNode* node, int depth);
    void PrintTreeByName(CAVLTree<Pair<CMyString, Course>>::TreeNode* node, int depth);
};
