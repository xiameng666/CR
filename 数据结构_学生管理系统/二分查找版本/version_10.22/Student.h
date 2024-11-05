#pragma once
#include "CMyString.h"
#include "StudentManagementException.h"  // 添加这一行

class Student {
private:
    int m_id;
    CMyString m_name;

public:
    Student() : m_id(0), m_name() {}
    Student(int id, const CMyString& name) : m_id(id), m_name(name) {
        if (name.empty()) {
            throw StudentManagementException("学生姓名不能为空");
        }
    }
    
    int getId() const;
    const CMyString& getName() const;
    
    bool operator<(const Student& other) const;
    bool operator==(const Student& other) const;
};