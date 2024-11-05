#define _CRT_SECURE_NO_WARNINGS
#include "CMyString.h"
#include <cstring>
#include <string>
#include <algorithm>
#include <cstdio>

// 删除 customCompare 函数实现

CMyString::CMyString() : m_pData(new char[1]), m_nLength(0) {
    m_pData[0] = '\0';
}

CMyString::CMyString(const char* pString) {
    if (pString) {
        m_nLength = strlen(pString);
        m_pData = new char[m_nLength + 1];
        strcpy(m_pData, pString);
    } else {
        m_pData = new char[1];
        m_pData[0] = '\0';
        m_nLength = 0;
    }
}

CMyString::CMyString(const CMyString& other) {
    if (other.m_pData) {
        m_nLength = other.m_nLength;
        m_pData = new char[m_nLength + 1];
        strcpy(m_pData, other.m_pData);
    } else {
        m_pData = new char[1];
        m_pData[0] = '\0';
        m_nLength = 0;
    }
}

CMyString::CMyString(CMyString&& other) noexcept
    : m_pData(other.m_pData), m_nLength(other.m_nLength) {
    other.m_pData = nullptr;
    other.m_nLength = 0;
}

CMyString::~CMyString() {
    delete[] m_pData;
}

CMyString& CMyString::operator=(const CMyString& other) {
    if (this != &other) {
        delete[] m_pData;
        if (other.m_pData) {
            m_nLength = other.m_nLength;
            m_pData = new char[m_nLength + 1];
            strcpy_s(m_pData, m_nLength + 1, other.m_pData);
        } else {
            m_pData = nullptr;
            m_nLength = 0;
        }
    }
    return *this;
}

CMyString& CMyString::operator=(CMyString&& other) noexcept {
    if (this != &other) {
        delete[] m_pData;
        m_pData = other.m_pData;
        m_nLength = other.m_nLength;
        other.m_pData = nullptr;
        other.m_nLength = 0;
    }
    return *this;
}

const char* CMyString::c_str() const {
    return m_pData ? m_pData : "";
}

int CMyString::length() const {
    return m_nLength;
}

bool CMyString::empty() const {
    return m_nLength == 0;
}

bool CMyString::operator==(const CMyString& other) const {
    return std::strcmp(m_pData, other.m_pData) == 0;
}

bool CMyString::operator<(const CMyString& other) const {
    return std::strcmp(m_pData, other.m_pData) < 0;
}

CMyString::CMyString(const char* pString, size_t length) {
    m_nLength = length;
    m_pData = new char[m_nLength + 1];
    strncpy_s(m_pData, m_nLength + 1, pString, m_nLength);
    m_pData[m_nLength] = '\0';
}

const char* CMyString::data() const {
    return m_pData;
}

size_t CMyString::getLength() const {
    return m_nLength;
}

void CMyString::serialize(std::ostream& out) const {
    out.write(reinterpret_cast<const char*>(&m_nLength), sizeof(int));
    out.write(m_pData, m_nLength);
}

CMyString CMyString::deserialize(std::istream& in) {
    int length;
    in.read(reinterpret_cast<char*>(&length), sizeof(int));
    char* buffer = new char[length + 1];
    in.read(buffer, length);
    buffer[length] = '\0';
    CMyString result(buffer);
    delete[] buffer;
    return result;
}

int CMyString::compare(const CMyString& other) const {
    // 处理空指针情况
    if (m_pData == nullptr && other.m_pData == nullptr) {
        return 0;
    }
    if (m_pData == nullptr) {
        return -1;
    }
    if (other.m_pData == nullptr) {
        return 1;
    }
    
    // 正常比较
    return std::strcmp(m_pData, other.m_pData);
}

std::istream& operator>>(std::istream& is, CMyString& str) {
    char buffer[1024];
    is >> buffer;
    str = CMyString(buffer);
    return is;
}

std::ostream& operator<<(std::ostream& os, const CMyString& str) {
    os << str.c_str();
    return os;
}

CMyString::CMyString(const std::string& str) {
    m_nLength = str.length();
    m_pData = new char[m_nLength + 1];
    strcpy_s(m_pData, m_nLength + 1, str.c_str());
}

CMyString::operator std::string() const {
    return std::string(m_pData ? m_pData : "");
}

CMyString& CMyString::operator=(const std::string& str) {
    if (m_pData != str.c_str()) {
        delete[] m_pData;
        m_nLength = str.length();
        m_pData = new char[m_nLength + 1];
        strcpy_s(m_pData, m_nLength + 1, str.c_str());
    }
    return *this;
}

CMyString CMyString::operator+(const CMyString& other) const {
    CMyString result;
    result.m_nLength = m_nLength + other.m_nLength;
    result.m_pData = new char[result.m_nLength + 1];
    strcpy_s(result.m_pData, result.m_nLength + 1, m_pData);
    strcat_s(result.m_pData, result.m_nLength + 1, other.m_pData);
    return result;
}

CMyString& CMyString::operator+=(const CMyString& other) {
    char* newData = new char[m_nLength + other.m_nLength + 1];
    strcpy_s(newData, m_nLength + other.m_nLength + 1, m_pData);
    strcat_s(newData, m_nLength + other.m_nLength + 1, other.m_pData);
    delete[] m_pData;
    m_pData = newData;
    m_nLength += other.m_nLength;
    return *this;
}

CMyString CMyString::operator+(const std::string& str) const {
    return *this + CMyString(str);
}

CMyString CMyString::operator+(const char* str) const {
    return *this + CMyString(str);
}

CMyString operator+(const std::string& lhs, const CMyString& rhs) {
    return CMyString(lhs) + rhs;
}

CMyString operator+(const char* lhs, const CMyString& rhs) {
    return CMyString(lhs) + rhs;
}

CMyString& CMyString::append(int value) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%d", value);
    *this += buffer;
    return *this;
}

CMyString CMyString::to_string(int value) {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%d", value);
    return CMyString(buffer);
}