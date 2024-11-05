#pragma once



template<typename TYPE>
class CLinkedQueue {
    struct ListNode {
        ListNode(const TYPE& Elem,
            ListNode* pPrev = nullptr,
            ListNode* pNext = nullptr)
            :m_Elem(Elem), m_pPrev(pPrev), m_pNext(pNext)
        {

        }
        ListNode(ListNode* pPrev = nullptr, ListNode* pNext = nullptr)
            :m_pPrev(pPrev), m_pNext(pNext)
        {

        }
        TYPE m_Elem;
        ListNode* m_pPrev;
        ListNode* m_pNext;
    };
public:
    CLinkedQueue() {
        m_pHead = new ListNode();
        m_pTail = new ListNode();
        m_pHead->m_pNext = m_pTail;
        m_pTail->m_pPrev = m_pHead;
        m_nSize = 0;
    }
    ~CLinkedQueue() {
        Clear();
        delete m_pHead;
        delete m_pTail;
    }
    void push(const TYPE& Elem) {
        //[] 1 [2] []
        m_pTail->m_pPrev = m_pTail->m_pPrev->m_pNext = new ListNode(Elem, m_pTail->m_pPrev, m_pTail);
        m_nSize++;
    }
    void pop() {
        //[] [1] 2 []
        ListNode* pDelNode = m_pHead->m_pNext;
        m_pHead->m_pNext = pDelNode->m_pNext;
        pDelNode->m_pNext->m_pPrev = m_pHead;
        delete pDelNode;
        m_nSize--;
    }
    TYPE Peek() const {
        return m_pHead->m_pNext->m_Elem;
    }
    bool Empty() const {
        return m_nSize == 0;
    }
    int Size() const {
        return m_nSize;
    }
    void Clear() {
        ListNode* pDelNode = m_pHead->m_pNext;
        ListNode* pNextNode = nullptr;
        while (pDelNode != m_pTail) {
            pNextNode = pDelNode->m_pNext;
            delete pDelNode;
            pDelNode = pNextNode;
        }
    }
private:
    ListNode* m_pHead;
    ListNode* m_pTail;
    int m_nSize;
};


template<typename TYPE>
class CArrayQueue {
public:
    CArrayQueue(int nInitSize = 1) {
        m_pBuf = m_pFront = new TYPE[nInitSize];
        m_nSize = 0;
    }
    ~CArrayQueue() {
        delete[] m_pBuf;
    }
    void push(const TYPE& Elem) {
        m_pFront[m_nSize++] = Elem;
    }
    void pop() {
        m_pFront++;
        m_nSize--;
    }
    TYPE Peek() const {
        return *m_pFront;
    }
    bool Empty() const {
        return m_nSize == 0;
    }
    void Clear() {
        m_pFront = m_pBuf;
        m_nSize = 0;
    }
    void Size() const {
        return m_nSize;
    }
private:
    TYPE* m_pBuf;
    TYPE* m_pFront;
    int   m_nSize;
};