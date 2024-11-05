#pragma once
#include "CVector.h"
#include "Student.h"
#include "Course.h"
#include "Grade.h"
#include "StudentManagementException.h"

class StudentManagementSystem {
private:
   //存储所有对象的数组
    CVector<Student> m_students;//按学生ID排序
    CVector<Course> m_courses;//按课程ID排序
    CVector<Grade> m_grades;
    

    /*
    下面这两个数组在逻辑上是一个索引空间
    索引值 < m_students.size()，指向 m_students
    索引值 >= m_students.size()，指向 m_courses   
    */ 

    //用于按名字查询的辅助数据结构
    CVector<CVector<int>> m_studentNameIndex;//外层按学生姓名字典序排序
                                            // 内层存储具有相同姓名的学生在m_students中的索引

    CVector<CVector<int>> m_courseNameIndex;//外层按课程名称字典序排序
                                            // 内层存储具有相同名称的课程在m_courses中的索引
                                            // 
    //用于按课程ID查询选课学生的辅助数据结构
    CVector<CVector<Grade>> m_courseGradeIndex;//外层按课程ID排序
                                               // 内层存储选修该课程的所有学生的成绩记录
    
    // 用于按学生ID查询选课信息的辅助数据结构
    CVector<CVector<Grade>> m_studentGradeIndex;//外层按学生ID排序
                                                // 内层存储该学生选修的所有课程的成绩记录

    // 二分查找辅助函数
    int binarySearch(const CVector<Student>& vec, int id) const;
    int binarySearch(const CVector<Course>& vec, int id) const;
    int binarySearchByName(const CVector<CVector<int>>& index, const CMyString& name) const;

    // 更新索引的辅助函数
    void updateStudentNameIndex(const Student& student, int index);
    void updateCourseNameIndex(const Course& course, int index);
    void updateCourseGradeIndex(const Grade& grade);
    void updateStudentGradeIndex(const Grade& grade);

    // 查找插入位置的辅助函数
    int findInsertPosition(const CVector<Student>& vec, int id) const;
    int findInsertPosition(const CVector<Course>& vec, int id) const;

    // 文件操作
    // void saveToFile() const;改为公有了
    void loadFromFile();



public:
    StudentManagementSystem();
    ~StudentManagementSystem() = default; 

    // 查询功能
    CMyString queryStudentNameById(int studentId) const;
    CVector<int> queryStudentIdByName(const CMyString& studentName) const;
    CMyString queryCourseNameById(int courseId) const;
    CVector<int> queryCourseIdByName(const CMyString& courseName) const;
    CVector<Grade> queryGradesByCourseId(int courseId) const;
    CVector<Grade> queryGradesByStudentId(int studentId) const;

    // 插入功能
    void insertStudent(int id, const CMyString& name);
    void insertCourse(int id, const CMyString& name);
    void insertGrade(int studentId, int courseId, float score);

    // 删除功能
    void deleteStudentById(int studentId);
    void deleteCourseById(int courseId);
    void deleteGrade(int studentId, int courseId);


    void saveToFile() const;

    // 打印
    void printAllInformation() const;

    // 格式化文件
    void formatAndClearData();


    //修改
// 新增修改方法
    void modifyStudentName(int studentId, const CMyString& newName);
    void modifyCourseName(int courseId, const CMyString& newName);
    void modifyGrade(int studentId, int courseId, float newScore);
    void generateLargeDataset();

    // 新增批量插入方法
    void batchInsertStudents(const CVector<Student>& students);
    void batchInsertCourses(const CVector<Course>& courses);
    void batchInsertGrades(const CVector<Grade>& grades);

    

private:
    // 新增方法
    void updateAllIndexes();
    bool m_indexingEnabled;

    // 预分配内存的辅助方法
    void reserveCapacity(size_t studentCount, size_t courseCount, size_t gradeCount);
};