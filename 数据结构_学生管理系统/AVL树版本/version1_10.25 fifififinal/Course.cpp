#include "Course.h"
#include <ostream>

Course::Course(int id, const CMyString& name) : m_id(id), m_name(name) {}

int Course::getId() const {
    return m_id;
}

const CMyString& Course::getName() const {
    return m_name;
}

bool Course::operator<(const Course& other) const {
    return m_id < other.m_id;
}

bool Course::operator==(const Course& other) const {
    return m_id == other.m_id;
}

std::ostream& operator<<(std::ostream& os, const Course& course) {
    os << "¿Î³ÌID: " << course.m_id << ", Ãû³Æ: " << course.m_name;
    return os;
}
