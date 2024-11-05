#include "Grade.h"
#include <ostream>

Grade::Grade(int studentId, int courseId, float score)
    : m_studentId(studentId), m_courseId(courseId), m_score(score) {}

int Grade::getStudentId() const {
    return m_studentId;
}

int Grade::getCourseId() const {
    return m_courseId;
}

float Grade::getScore() const {
    return m_score;
}

bool Grade::operator<(const Grade& other) const {
    if (m_studentId != other.m_studentId) {
        return m_studentId < other.m_studentId;
    }
    return m_courseId < other.m_courseId;
}

void Grade::setScore(float score) {
    m_score = score;
}

// 实现 << 运算符重载
std::ostream& operator<<(std::ostream& os, const Grade& grade) {
    os << "学生ID: " << grade.m_studentId 
       << ", 课程ID: " << grade.m_courseId 
       << ", 成绩: " << grade.m_score;
    return os;
}
