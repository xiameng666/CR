#pragma once
#include <algorithm>


template<typename TYPE>
class CVector
{
private:
    void Init(int nCapacity) {
        m_pBuf = nullptr;
        m_nCapacity = m_nSize = 0;
        reserve(nCapacity);
    }
public:
    CVector(int nInitSize = 1) {
        Init(nInitSize);
    }
    CVector(const CVector& Other) {
        Init(Other.m_nCapacity);
        for (int i = 0; i < Other.size(); i++) {
            push_back(Other[i]);
        }
    }
    CVector(CVector&& Other) {
        Init(0);
        std::swap(m_nCapacity, Other.m_nCapacity);
        std::swap(m_nSize, Other.m_nSize);
        std::swap(m_pBuf, Other.m_pBuf);
    }
    CVector& operator=(const CVector& Other) {
        CVector v{ Other };
        std::swap(m_nCapacity, v.m_nCapacity);
        std::swap(m_nSize, v.m_nSize);
        std::swap(m_pBuf, v.m_pBuf);
        return *this;
    }
    CVector& operator=(CVector&& Other) {
        //释放自己
        std::swap(m_nCapacity, Other.m_nCapacity);
        std::swap(m_nSize, Other.m_nSize);
        std::swap(m_pBuf, Other.m_pBuf);
        return *this;
    }

    ~CVector() {
        delete[] m_pBuf;
    }

    //调整大小
    void resize(int nSize) {
        if (m_nSize <= m_nCapacity) {
            reserve(nSize + 1);
        }
        m_nSize = nSize;
    }

    //扩容O(n) 线性阶
    void reserve(int nNewCapacity) {
        if (nNewCapacity <= m_nCapacity)
            return;

        //申请新的空间
        TYPE* pNewBuf = new TYPE[nNewCapacity];

        //拷贝旧的数据   
        //memcpy(pNewBuf, m_pBuf, m_nSize * sizeof(int));
        for (int i = 0; i < m_nSize; i++) {
            pNewBuf[i] = std::move(m_pBuf[i]);
        }

        //释放旧的空间
        delete[] m_pBuf;

        //更新缓冲区和容量
        m_pBuf = pNewBuf;
        m_nCapacity = nNewCapacity;
    }

    //最好O(1)   最坏:O(n)
    void push_back(const TYPE& Element) {
        //判断容量
        if (m_nSize == m_nCapacity) {
           reserve(m_nSize * 2 + 1);
        }
        m_pBuf[m_nSize++] = Element;
    }

    void push_back(TYPE&& Element) {
        //判断容量
        if (m_nSize == m_nCapacity) {
            reserve(m_nSize * 2 + 1);
        }
        m_pBuf[m_nSize++] = std::move(Element);
    }

    //O(n) 线性阶
    void  insert(int nIndex, const TYPE& Element) {//插入
        //判断容量
        if (m_nSize == m_nCapacity) {
            reserve(m_nSize * 2 + 1);
        }

        //移动元素
        //1 2 5 4 0    4
        for (int i = m_nSize; i > nIndex; i--) {
            m_pBuf[i] = std::move(m_pBuf[i - 1]);
        }

        //1 [3] 2 5 4  
        m_pBuf[nIndex] = Element;
        m_nSize++;
    }
    void  insert(int nIndex, TYPE& Element) {//插入
    //判断容量
        if (m_nSize == m_nCapacity) {
            reserve(m_nSize * 2 + 1);
        }

        //移动元素
        //1 2 5 4 0    4
        for (int i = m_nSize; i > nIndex; i--) {
            m_pBuf[i] = std::move(m_pBuf[i - 1]);
        }

        //1 [3] 2 5 4  
        m_pBuf[nIndex] = std::move(Element);
        m_nSize++;
    }

    //O(1)
    void  pop_back() {
        //1 3 2 5 [4]
        m_nSize--;
    }

    void  erase(int nIndex) { //删除
         //移动元素
        //1 [3] 2 5 4 
        for (int i = nIndex; i < m_nSize - 1; i++) {
            m_pBuf[i] = std::move(m_pBuf[i + 1]);
        }
        m_nSize--;
    }

    //O(n)
    int   find(const TYPE& nElem) {
        for (int i = 0; i < m_nSize; i++) {
            if (m_pBuf[i] == nElem) {
                return i;
            }
        }
        return -1;
    }

    //随机访问O(1) 常量阶
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
    const TYPE& front() const  {
        return m_pBuf[0];
    }

    int size() const {
        return m_nSize;
    }
    int capacity() const {
        return m_nCapacity;
    }
    bool empty() const {
        return m_nSize == 0;
    }
    void clear()  {
       m_nSize = 0;
    }
public:
    class iterator {
    public:
        iterator(TYPE* pElem) {
            m_pElement = pElem;
        }
        TYPE& operator*() {
            return *m_pElement;
        }
        iterator operator++() {
            return iterator{ ++m_pElement };
        }
        const iterator operator++(int) {
             return iterator{ m_pElement++ };
        }
        bool operator!=(const iterator& Other) {
            return m_pElement != Other.m_pElement;
        }
        bool operator==(const iterator& Other) {
            return m_pElement == Other.m_pElement;
        }
    private:
        TYPE* m_pElement;
    };


    class const_iterator {
    public:
        const_iterator(TYPE* pElem) {
            m_pElement = pElem;
        }
        const TYPE& operator*() {
            return *m_pElement;
        }
        const_iterator operator++() {
            return const_iterator{ ++m_pElement };
        }
        const const_iterator operator++(int) {
            return const_iterator{ m_pElement++ };
        }
        bool operator!=(const const_iterator& Other) {
            return m_pElement != Other.m_pElement;
        }
        bool operator==(const const_iterator& Other) {
            return m_pElement == Other.m_pElement;
        }
    private:
        TYPE* m_pElement;
    };

    iterator begin() {
        return iterator{m_pBuf};
    }
    iterator end() {
        return iterator{ m_pBuf + m_nSize };
    }

    const_iterator begin() const {
        return const_iterator{ m_pBuf };
    }
    const_iterator end() const {
        return const_iterator{ m_pBuf + m_nSize };
    }
private:
    TYPE* m_pBuf;      //数组缓冲区
    int       m_nSize;     //大小
    int       m_nCapacity; //容量
};

