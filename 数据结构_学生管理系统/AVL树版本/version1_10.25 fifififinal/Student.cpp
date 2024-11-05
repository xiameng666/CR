#include "Student.h"

// 删除或注释掉以下构造函数定义，因为它已经在头文件中定义了
// Student::Student(int id, const CMyString& name) : m_id(id), m_name(name) {
//     if (name.empty()) {
//         throw StudentManagementException("学生姓名不能为空");
//     }
// }

int Student::getId() const {
    return m_id;
}

const CMyString& Student::getName() const {
    return m_name;
}

bool Student::operator<(const Student& other) const {
    return m_id < other.m_id;
}

bool Student::operator==(const Student& other) const {
    return m_id == other.m_id;
}

std::ostream& operator<<(std::ostream& os, const Student& student) {
    os << "学生ID: " << student.m_id << ", 姓名: " << student.m_name;
    return os;
}
