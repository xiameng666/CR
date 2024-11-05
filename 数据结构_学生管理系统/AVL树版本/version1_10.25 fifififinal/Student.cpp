#include "Student.h"

// ɾ����ע�͵����¹��캯�����壬��Ϊ���Ѿ���ͷ�ļ��ж�����
// Student::Student(int id, const CMyString& name) : m_id(id), m_name(name) {
//     if (name.empty()) {
//         throw StudentManagementException("ѧ����������Ϊ��");
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
    os << "ѧ��ID: " << student.m_id << ", ����: " << student.m_name;
    return os;
}
