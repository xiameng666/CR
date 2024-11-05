#pragma once
#include <iostream>

class Grade {
private:
    int m_studentId;
    int m_courseId;
    float m_score;

public:
    // ���Ĭ�Ϲ��캯��
    Grade() : m_studentId(0), m_courseId(0), m_score(0.0f) {}
    
    Grade(int studentId, int courseId, float score);
    
    int getStudentId() const;
    int getCourseId() const;
    float getScore() const;
    
    bool operator<(const Grade& other) const;
    void setScore(float score);
    // ��� == ���������
    bool operator==(const Grade& other) const {
        return m_studentId == other.m_studentId && m_courseId == other.m_courseId;
    }

    // �����Ԫ��������
    friend std::ostream& operator<<(std::ostream& os, const Grade& grade);
};
