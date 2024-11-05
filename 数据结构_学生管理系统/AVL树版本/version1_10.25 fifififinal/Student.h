#pragma once
#include "CMyString.h"
#include <iostream>

class Student {
private:
    int m_id;
    CMyString m_name;

public:
    Student() : m_id(0), m_name() {}
    Student(int id, const CMyString& name) : m_id(id), m_name(name) {};
    
    int getId() const;
    const CMyString& getName() const;
    
    void setName(const CMyString & name) { m_name = name; }
    
    bool operator<(const Student& other) const;
    bool operator==(const Student& other) const;

    // 添加友元函数声明
    friend std::ostream& operator<<(std::ostream& os, const Student& student);
};
