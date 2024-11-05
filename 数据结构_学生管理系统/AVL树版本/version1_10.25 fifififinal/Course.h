#pragma once
#include "CMyString.h"
#include <iostream>

class Course {
private:
    int m_id;
    CMyString m_name;

public:
    Course() :  m_id(0), m_name(0){}
    Course(int id, const CMyString& name);
    
    int getId() const;
    const CMyString& getName() const;
    
    bool operator<(const Course& other) const;
    bool operator==(const Course& other) const;

    // 添加友元函数声明
    friend std::ostream& operator<<(std::ostream& os, const Course& course);
};
