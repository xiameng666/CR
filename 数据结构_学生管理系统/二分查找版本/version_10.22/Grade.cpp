#include "Grade.h"

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
