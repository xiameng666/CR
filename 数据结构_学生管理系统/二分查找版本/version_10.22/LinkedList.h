#pragma once

#include <stdio.h>

template<typename TYPE>
class CLinkedList {
private:
    struct Node {
        friend class CLinkedList;
        Node(Node* pPrev = nullptr, Node* pNext = nullptr)
            :m_pPrev(pPrev), m_pNext(pNext)
        {
        }
        Node(const TYPE& Data, Node* pPrev = nullptr, Node* pNext = nullptr)
            :m_Data(Data), m_pPrev(pPrev), m_pNext(pNext)
        {
        }
        Node(TYPE&& Data, Node* pPrev = nullptr, Node* pNext = nullptr)
            :m_Data(Data), m_pPrev(pPrev), m_pNext(pNext)
        {
        }

        TYPE     m_Data;    //元素
        Node* m_pPrev; //前驱
        Node* m_pNext; //后继
    };
public:
    class iterator {
    public:
        iterator(Node* pElem) {
            m_pElement = pElem;
        }
        TYPE& operator*() {
            return m_pElement->m_Data;
        }
        iterator operator++() {
            m_pElement = m_pElement->m_pNext;
            return iterator{ m_pElement };
        }
        const iterator operator++(int) {
            iterator it{ m_pElement };
            m_pElement = m_pElement->m_pNext;
            return it;
        }
        bool operator!=(const iterator& Other) {
            return m_pElement != Other.m_pElement;
        }
        bool operator==(const iterator& Other) {
            return m_pElement == Other.m_pElement;
        }
        TYPE* operator->() {
            return &(m_pElement->m_Data);
        }
        Node* GetNode() const {  // 将 GetNode() 改为公有
            return m_pElement;
        }
    private:
        friend class CLinkedList;
        Node* m_pElement;
    };
    iterator begin() const { return iterator{ m_pHead }; }
    iterator end() const { return iterator{ nullptr }; }
private:
    void Init() {
        m_pHead = new Node{};
        m_pTail = new Node{};
        m_pHead->m_pNext = m_pTail;
        m_pTail->m_pPrev = m_pHead;
        m_nSize = 0;
    }
public:
    CLinkedList() {
        Init();
    }
    CLinkedList(std::initializer_list<TYPE> l) {
        Init();
        for (auto e : l)
            push_back(e);
    }
    ~CLinkedList() {
        clear();
        delete m_pHead;
        delete m_pTail;
    }
    CLinkedList(const CLinkedList& Other) {
        Init();
        for (auto e : (CLinkedList&)Other)
            push_back(e);
    }
    CLinkedList(CLinkedList&& Other) {
        Init();
        std::swap(m_pHead, Other.m_pHead);
        std::swap(m_pTail, Other.m_pTail);
        std::swap(m_nSize, Other.m_nSize);
    }

    CLinkedList& operator= (const CLinkedList& Other) {
        CLinkedList Copy{ Other };
        std::swap(m_pHead, Copy.m_pHead);
        std::swap(m_pTail, Copy.m_pTail);
        std::swap(m_nSize, Copy.m_nSize);
        return *this;
    }

    CLinkedList& operator=(CLinkedList&& Other) {
        std::swap(m_pHead, Other.m_pHead);
        std::swap(m_pTail, Other.m_pTail);
        std::swap(m_nSize, Other.m_nSize);
        return *this;
    }

    iterator push_front(const TYPE& Element) {
        return insert(m_pHead, Element);
    }

    iterator push_front(TYPE&& Element) {
        return insert(m_pHead, Element);
    }

    iterator push_back(const TYPE& Element) {
        m_nSize++;
        Node* newNode = new Node{ Element, m_pTail->m_pPrev, m_pTail };
        m_pTail->m_pPrev->m_pNext = newNode;
        m_pTail->m_pPrev = newNode;
        return iterator(newNode);
    }

    iterator push_back(TYPE&& Element) {
        m_nSize++;
        Node* newNode = new Node{ std::move(Element), m_pTail->m_pPrev, m_pTail };
        m_pTail->m_pPrev->m_pNext = newNode;
        m_pTail->m_pPrev = newNode;
        return iterator(newNode);
    }

    void insert(const iterator& it, const CLinkedList& List) {
        iterator it2 = it;
        for (auto e : (CLinkedList&)List) {
            it2 = insert(it2, e);
        }

    }
    iterator insert(const iterator& it, const TYPE& Element) {
        /*
        *   p [3] tail
        *
        *   new->prev = p
        *   new->next = p->next
        *
        *   p->next->prev = new
        *
        *   p->next = new
        *
        *
        */
        m_nSize++;
        Node* p = it.GetNode();
        Node* newNode = new Node{ Element, p, p->m_pNext };
        if (p->m_pNext) {
            p->m_pNext->m_pPrev = newNode;
        }
        else {
            m_pTail = newNode;
        }
        p->m_pNext = newNode;
        return iterator(newNode);
    }
    iterator insert(const iterator& it, TYPE&& Element) {
        m_nSize++;
        Node* p = it.GetNode();
        return p->m_pNext = p->m_pNext->m_pPrev =
            new Node{ std::move(Element),  p, p->m_pNext };
    }


    void  pop_back() {
        earase(back());
    }
    void  pop_front() {
        earase(front());
    }
    void  earase(const iterator& it) {
        /*
        * head [1] tail
        * head 1 [2] 3 tail
        *
        *  del->prev->next = del->next
        *  del->next->prev = del->prev;
        */
        Node* pDel = it.GetNode();
        pDel->m_pPrev->m_pNext = pDel->m_pNext;
        pDel->m_pNext->m_pPrev = pDel->m_pPrev;
        delete pDel;
        m_nSize--;
    }

    iterator back() {
        return iterator(m_pTail->m_pPrev);
    }
    iterator front() {
        return m_pHead->m_pNext;
    }
    int size() const {
        return m_nSize;
    }
    bool empty() const {
        return m_nSize == 0;
    }
    void clear() {
        //遍历链表
        Node* pNode = m_pHead->m_pNext;
        Node* pNext = nullptr;
        while (pNode != m_pTail) {
            pNext = pNode->m_pNext;
            delete pNode;
            pNode = pNext;
        }
        m_pHead->m_pNext = m_pTail;
        m_pTail->m_pPrev = m_pHead;
        m_nSize = 0;
    }

    void Show() {
        Node* p = m_pHead;
        printf("size:%d\n", m_nSize);
        while (p != nullptr) {
            printf("%p:%08x[%p][%p]->\n", p, p->m_Data, p->m_pPrev, p->m_pNext);
            p = p->m_pNext;
        }

        printf("\n");
        p = m_pTail;
        while (p != nullptr) {
            printf("%p:%08x[%p][%p]->\n", p, p->m_Data, p->m_pPrev, p->m_pNext);
            p = p->m_pPrev;
        }
    }

    void erase(iterator it) {
        if (it == end()) return;

        Node* pNode = it.GetNode();
        if (pNode->m_pPrev) {
            pNode->m_pPrev->m_pNext = pNode->m_pNext;
        }
        else {
            m_pHead = pNode->m_pNext;
        }

        if (pNode->m_pNext) {
            pNode->m_pNext->m_pPrev = pNode->m_pPrev;
        }
        else {
            m_pTail = pNode->m_pPrev;
        }

        delete pNode;
        m_nSize--;
    }

    Node* getHead() const { return m_pHead; }
    Node* getTail() const { return m_pTail; }

private:
    Node* m_pHead;
    Node* m_pTail;
    int m_nSize;
};


template<typename TYPE>
class CList {
private:
    struct Node {
        friend class CList;
        Node(Node* pNext = nullptr)
            :m_pNext(pNext)
        {
        }
        Node(const TYPE& Data, Node* pNext = nullptr)
            :m_Data(Data), m_pNext(pNext)
        {
        }
        Node(TYPE&& Data, Node* pNext = nullptr)
            :m_Data(Data), m_pNext(pNext)
        {
        }

        TYPE     m_Data;    //元素
        Node* m_pNext; //后继
    };
public:
    class iterator {
    public:
        iterator(Node* pElem) {
            m_pElement = pElem;
        }
        TYPE& operator*() {
            return m_pElement->m_Data;
        }
        iterator operator++() {
            m_pElement = m_pElement->m_pNext;
            return iterator{ m_pElement };
        }
        const iterator operator++(int) {
            iterator it{ m_pElement };
            m_pElement = m_pElement->m_pNext;
            return it;
        }
        bool operator!=(const iterator& Other) {
            return m_pElement != Other.m_pElement;
        }
        bool operator==(const iterator& Other) {
            return m_pElement == Other.m_pElement;
        }
    private:
        friend class CList;
        Node* GetNode() const {
            return m_pElement;
        }
        Node* m_pElement;
    };
    iterator begin() {
        return iterator{ front() };
    }
    iterator end() {
        return iterator{ back() };
    }
private:
    void Init() {
        m_pTail = m_pHead = new Node{};
        m_nSize = 0;
    }
public:
    CList() {
        Init();
    }
    CList(std::initializer_list<TYPE> l) {
        Init();
        for (auto e : l)
            push_back(e);
    }
    ~CList() {
        clear();
        delete m_pHead;
    }
    CList(const CList& Other) {
        Init();
        for (auto e : (CList&)Other)
            push_back(e);
    }
    CList(CList&& Other) {
        Init();
        std::swap(m_pHead, Other.m_pHead);
        std::swap(m_nSize, Other.m_nSize);
    }

    CList& operator= (const CList& Other) {
        CList Copy{ Other };
        std::swap(m_pHead, Copy.m_pHead);
        std::swap(m_nSize, Copy.m_nSize);
        return *this;
    }

    CList& operator=(CList&& Other) {
        std::swap(m_pHead, Other.m_pHead);
        std::swap(m_nSize, Other.m_nSize);
        return *this;
    }

    iterator push_front(const TYPE& Element) {
        return insert(m_pHead, Element);
    }

    iterator push_front(TYPE&& Element) {
        return insert(m_pHead, Element);
    }

    //1 2 3  [4]
    iterator push_back(const TYPE& Element) {
        return m_pTail = m_pTail->m_pNext = new Node{ Element , nullptr };
    }
    iterator push_back(TYPE&& Element) {
        //内存分配器 Alloctor new chache(8M)  
      /*  Alloctor a;
        a.alloc(sizeof(Node));
        a.free(p);*/
        return m_pTail = m_pTail->m_pNext = new Node{ std::move(Element), nullptr };
    }
    void insert(const iterator& it, const CList& List) {
        iterator it2 = it;
        for (auto e : (CList&)List) {
            it2 = insert(it2, e);
        }

    }
    iterator insert(const iterator& it, const TYPE& Element) {
        /*
        *   p [3] tail
        *
        *   new->next = p->next
        *   p->next = new
        *
        *
        */
        m_nSize++;
        Node* p = it.GetNode();
        return p->m_pNext = new Node{ Element, p->m_pNext };
    }
    iterator insert(const iterator& it, TYPE&& Element) {
        m_nSize++;
        Node* p = it.GetNode();
        return p->m_pNext =
            new Node{ std::move(Element), p->m_pNext };
    }


    void  pop_back() {
        earase(back());
    }
    void  pop_front() {
        Node* pDel = m_pHead->m_pNext;
        //1 [2]
        //1 [2] 3
        m_pHead->m_pNext = pDel->m_pNext;
        if (m_pHead->m_pNext == nullptr)
            m_pTail = m_pHead;
        delete pDel;
        m_nSize--;
    }
    void  earase(const iterator& it) {
        /*
        * head [1] tail
        * head 1 [2] 3 tail
        *
        *  del->prev->next = del->next
        */
        Node* pDel = it.GetNode();
        Node* pPrev = m_pHead;
        while (pPrev->m_pNext != pDel) {
            pPrev = pPrev->m_pNext;
        }

        pPrev->m_pNext = pDel->m_pNext;
        delete pDel;
        m_nSize--;
    }

    iterator back() {
        return m_pTail;
    }
    iterator front() {
        return m_pHead->m_pNext;
    }
    int size() const {
        return m_nSize;
    }
    bool empty() {
        return m_nSize == 0;
    }
    void clear() {
        //遍历链表
        Node* pNode = m_pHead->m_pNext;
        Node* pNext = nullptr;
        while (pNode != nullptr) {
            pNext = pNode->m_pNext;
            delete pNode;
            pNode = pNext;
        }
        m_pHead->m_pNext = nullptr;
        m_nSize = 0;
    }

    void Show() {
        Node* p = m_pHead;
        printf("size:%d\n", m_nSize);
        while (p != nullptr) {
            printf("%p:%08x [%p]->\n", p, p->m_Data, p->m_pNext);
            p = p->m_pNext;
        }
        printf("\n");
    }
private:
    Node* m_pHead;
    Node* m_pTail;
    int m_nSize;
};