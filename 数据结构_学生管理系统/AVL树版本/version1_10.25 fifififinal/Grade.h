#pragma once
#include <iostream>

class Grade {
private:
    int m_studentId;
    int m_courseId;
    float m_score;

public:
    // 添加默认构造函数
    Grade() : m_studentId(0), m_courseId(0), m_score(0.0f) {}
    
    Grade(int studentId, int courseId, float score);
    
    int getStudentId() const;
    int getCourseId() const;
    float getScore() const;
    
    bool operator<(const Grade& other) const;
    void setScore(float score);
    // 添加 == 运算符重载
    bool operator==(const Grade& other) const {
        return m_studentId == other.m_studentId && m_courseId == other.m_courseId;
    }

    // 添加友元函数声明
    friend std::ostream& operator<<(std::ostream& os, const Grade& grade);
};
