#pragma once
#include "CMyString.h"
#include "StudentManagementException.h"  // �����һ��

class Student {
private:
    int m_id;
    CMyString m_name;

public:
    Student() : m_id(0), m_name() {}
    Student(int id, const CMyString& name) : m_id(id), m_name(name) {
        if (name.empty()) {
            throw StudentManagementException("ѧ����������Ϊ��");
        }
    }
    
    int getId() const;
    const CMyString& getName() const;
    
    bool operator<(const Student& other) const;
    bool operator==(const Student& other) const;
};