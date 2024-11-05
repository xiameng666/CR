#pragma once
#include <iostream>
#include <string>

class CMyString {
private:
    char* m_pData;
    int m_nLength;

public:
    CMyString();
    CMyString(const char* pString);
    CMyString(const CMyString& other);
    CMyString(CMyString&& other) noexcept;
    ~CMyString();

    CMyString& operator=(const CMyString& other);
    CMyString& operator=(CMyString&& other) noexcept;

    const char* c_str() const;
    int length() const;
    bool empty() const;

    bool operator==(const CMyString& other) const;
    bool operator<(const CMyString& other) const;
    bool operator>(const CMyString& other) const;

    CMyString(const char* pString, size_t length);
    const char* data() const;

    size_t getLength() const;
    
    void serialize(std::ostream& out) const;
    
    static CMyString deserialize(std::istream& in);
    
    int compare(const CMyString& other) const;
    
    friend std::istream& operator>>(std::istream& is, CMyString& str);
    friend std::ostream& operator<<(std::ostream& os, const CMyString& str);

    CMyString(const std::string& str);
    operator std::string() const;
    CMyString& operator=(const std::string& str);

    CMyString operator+(const CMyString& other) const;
    CMyString& operator+=(const CMyString& other);
    CMyString operator+(const std::string& str) const;
    CMyString operator+(const char* str) const;

    friend CMyString operator+(const std::string& lhs, const CMyString& rhs);
    friend CMyString operator+(const char* lhs, const CMyString& rhs);

    CMyString& append(int value);
    static CMyString to_string(int value);
};
