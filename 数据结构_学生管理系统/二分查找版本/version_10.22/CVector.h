#pragma once
#include <algorithm>
#include <cstddef>
#include <iostream> 

template<typename TYPE>
class CVector
{
private:
    TYPE* m_pBuf;
    int   m_nSize;
    int   m_nCapacity;

    void Init(int nInitSize) {
        m_pBuf = nullptr;
        m_nSize = 0;
        m_nCapacity = 0;
        reserve(nInitSize);
    }

public:
    CVector(int nInitSize = 1) : m_pBuf(nullptr), m_nSize(0), m_nCapacity(0) {
        Init(nInitSize);
    }
    CVector(const CVector& Other) : m_pBuf(nullptr), m_nSize(0), m_nCapacity(0) {
        Init(Other.m_nCapacity);
        for (int i = 0; i < Other.size(); i++) {
            push_back(Other[i]);
        }
    }
    CVector(CVector&& Other) noexcept : m_pBuf(Other.m_pBuf), m_nSize(Other.m_nSize), m_nCapacity(Other.m_nCapacity) {
        Other.m_pBuf = nullptr;
        Other.m_nSize = 0;
        Other.m_nCapacity = 0;
    }
    CVector& operator=(const CVector& Other) {
        if (this != &Other) {
            CVector temp(Other);
            std::swap(m_pBuf, temp.m_pBuf);
            std::swap(m_nSize, temp.m_nSize);
            std::swap(m_nCapacity, temp.m_nCapacity);
        }
        return *this;
    }
    CVector& operator=(CVector&& Other) noexcept {
        if (this != &Other) {
            delete[] m_pBuf;
            m_pBuf = Other.m_pBuf;
            m_nSize = Other.m_nSize;
            m_nCapacity = Other.m_nCapacity;
            Other.m_pBuf = nullptr;
            Other.m_nSize = 0;
            Other.m_nCapacity = 0;
        }
        return *this;
    }

    ~CVector() {
        delete[] m_pBuf;
    }

    void resize(int nSize) {
        if (m_nSize <= m_nCapacity) {
            reserve(nSize + 1);
        }
        m_nSize = nSize;
    }

    void reserve(int nNewCapacity) {
        if (nNewCapacity <= m_nCapacity)
            return;

        TYPE* pNewBuf = new TYPE[nNewCapacity];

        for (int i = 0; i < m_nSize; i++) {
            new (&pNewBuf[i]) TYPE(std::move(m_pBuf[i]));
            m_pBuf[i].~TYPE();
        }

        delete[] m_pBuf;

        m_pBuf = pNewBuf;
        m_nCapacity = nNewCapacity;
    }

    void push_back(const TYPE& Element) {
        if (m_nSize == m_nCapacity) {
            reserve(m_nSize * 2 + 1);
        }
        if (m_pBuf) {
            new (&m_pBuf[m_nSize]) TYPE(Element);
            m_nSize++;
        }
        else {
            throw std::runtime_error("CVector: Buffer is null");
        }
    }

    void push_back(TYPE&& Element) {
        if (m_nSize == m_nCapacity) {
            reserve(m_nSize * 2 + 1);
        }
        if (m_pBuf) {
            new (&m_pBuf[m_nSize]) TYPE(std::move(Element));
            m_nSize++;
        }
        else {
            throw std::runtime_error("CVector: Buffer is null");
        }
    }

    void  insert(int nIndex, const TYPE& Element) {
        if (m_nSize > m_nCapacity) {
            reserve(m_nSize * 2 + 1);
        }

        for (int i = m_nSize; i > nIndex; i--) {
            m_pBuf[i] = m_pBuf[i - 1];  // 使用复制而不是移动
        }

        m_pBuf[nIndex] = Element;  // 使用复制而不是移动
        m_nSize++;
    }
    void  insert(int nIndex, TYPE& Element) {
        if (m_nSize == m_nCapacity) {
            reserve(m_nSize * 2 + 1);
        }

        for (int i = m_nSize; i > nIndex; i--) {
            m_pBuf[i] = std::move(m_pBuf[i - 1]);
        }

        m_pBuf[nIndex] = std::move(Element);
        m_nSize++;
    }

    void  pop_back() {
        m_nSize--;
    }

    void  erase(int nIndex) {
        for (int i = nIndex; i < m_nSize - 1; i++) {
            m_pBuf[i] = std::move(m_pBuf[i + 1]);
        }
        m_nSize--;
    }

    int   find(const TYPE& nElem) {
        for (int i = 0; i < m_nSize; i++) {
            if (m_pBuf[i] == nElem) {
                return i;
            }
        }
        return -1;
    }

    TYPE& at(int nIndex) {
        return m_pBuf[nIndex];
    }
    const TYPE& at(int nIndex) const {
        return m_pBuf[nIndex];
    }

    TYPE& operator[](int nIndex) {
        return at(nIndex);
    }
    const TYPE& operator[](int nIndex) const {
        return at(nIndex);
    }

    const TYPE& back()  const {
        return m_pBuf[m_nSize - 1];
    }
    const TYPE& front() const {
        return m_pBuf[0];
    }

    int size() const {
        if (this == nullptr) {
            std::cerr << "错误：CVector 对象为空" << std::endl;
            return 0;
        }
        return m_nSize;
    }
    int capacity() const {
        return m_nCapacity;
    }
    bool empty() const {
        return m_nSize == 0;
    }
    void clear() {
        m_nSize = 0;
    }

    // Iterator support
    typedef TYPE* iterator;
    typedef const TYPE* const_iterator;

    iterator begin() { return m_pBuf; }
    iterator end() { return m_pBuf + m_nSize; }
    const_iterator begin() const { return m_pBuf; }
    const_iterator end() const { return m_pBuf + m_nSize; }

};