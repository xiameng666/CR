#pragma once
#include "CVector.h"
#include "Student.h"
#include "Course.h"
#include "Grade.h"
#include "StudentManagementException.h"

class StudentManagementSystem {
private:
   //�洢���ж��������
    CVector<Student> m_students;//��ѧ��ID����
    CVector<Course> m_courses;//���γ�ID����
    CVector<Grade> m_grades;
    

    /*
    �����������������߼�����һ�������ռ�
    ����ֵ < m_students.size()��ָ�� m_students
    ����ֵ >= m_students.size()��ָ�� m_courses   
    */ 

    //���ڰ����ֲ�ѯ�ĸ������ݽṹ
    CVector<CVector<int>> m_studentNameIndex;//��㰴ѧ�������ֵ�������
                                            // �ڲ�洢������ͬ������ѧ����m_students�е�����

    CVector<CVector<int>> m_courseNameIndex;//��㰴�γ������ֵ�������
                                            // �ڲ�洢������ͬ���ƵĿγ���m_courses�е�����
                                            // 
    //���ڰ��γ�ID��ѯѡ��ѧ���ĸ������ݽṹ
    CVector<CVector<Grade>> m_courseGradeIndex;//��㰴�γ�ID����
                                               // �ڲ�洢ѡ�޸ÿγ̵�����ѧ���ĳɼ���¼
    
    // ���ڰ�ѧ��ID��ѯѡ����Ϣ�ĸ������ݽṹ
    CVector<CVector<Grade>> m_studentGradeIndex;//��㰴ѧ��ID����
                                                // �ڲ�洢��ѧ��ѡ�޵����пγ̵ĳɼ���¼

    // ���ֲ��Ҹ�������
    int binarySearch(const CVector<Student>& vec, int id) const;
    int binarySearch(const CVector<Course>& vec, int id) const;
    int binarySearchByName(const CVector<CVector<int>>& index, const CMyString& name) const;

    // ���������ĸ�������
    void updateStudentNameIndex(const Student& student, int index);
    void updateCourseNameIndex(const Course& course, int index);
    void updateCourseGradeIndex(const Grade& grade);
    void updateStudentGradeIndex(const Grade& grade);

    // ���Ҳ���λ�õĸ�������
    int findInsertPosition(const CVector<Student>& vec, int id) const;
    int findInsertPosition(const CVector<Course>& vec, int id) const;

    // �ļ�����
    // void saveToFile() const;��Ϊ������
    void loadFromFile();



public:
    StudentManagementSystem();
    ~StudentManagementSystem() = default; 

    // ��ѯ����
    CMyString queryStudentNameById(int studentId) const;
    CVector<int> queryStudentIdByName(const CMyString& studentName) const;
    CMyString queryCourseNameById(int courseId) const;
    CVector<int> queryCourseIdByName(const CMyString& courseName) const;
    CVector<Grade> queryGradesByCourseId(int courseId) const;
    CVector<Grade> queryGradesByStudentId(int studentId) const;

    // ���빦��
    void insertStudent(int id, const CMyString& name);
    void insertCourse(int id, const CMyString& name);
    void insertGrade(int studentId, int courseId, float score);

    // ɾ������
    void deleteStudentById(int studentId);
    void deleteCourseById(int courseId);
    void deleteGrade(int studentId, int courseId);


    void saveToFile() const;

    // ��ӡ
    void printAllInformation() const;

    // ��ʽ���ļ�
    void formatAndClearData();


    //�޸�
// �����޸ķ���
    void modifyStudentName(int studentId, const CMyString& newName);
    void modifyCourseName(int courseId, const CMyString& newName);
    void modifyGrade(int studentId, int courseId, float newScore);
    void generateLargeDataset();

    // �����������뷽��
    void batchInsertStudents(const CVector<Student>& students);
    void batchInsertCourses(const CVector<Course>& courses);
    void batchInsertGrades(const CVector<Grade>& grades);

    

private:
    // ��������
    void updateAllIndexes();
    bool m_indexingEnabled;

    // Ԥ�����ڴ�ĸ�������
    void reserveCapacity(size_t studentCount, size_t courseCount, size_t gradeCount);
};