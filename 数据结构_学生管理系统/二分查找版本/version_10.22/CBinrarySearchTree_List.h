#pragma once

#include "CQueue.h"
#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <vector>

template<class KEY, class VALUE>
class Pair
{
public:
    Pair(const KEY& Key) :m_Key(Key) {}
    Pair(const KEY& Key, const VALUE& Value) :m_Key(Key), m_Value(Value) {}

    bool operator>(const Pair& Other) const { return m_Key > Other.m_Key; }
    bool operator==(const Pair& Other) const { return m_Key == Other.m_Key; }
    bool operator<(const Pair& Other) const { return m_Key < Other.m_Key; }

    friend std::ostream& operator<< (std::ostream& os, Pair& s) {
        os << s.m_Value;
        return os;
    }

    KEY m_Key;
    VALUE m_Value;
};

template<class TYPE>
class CAVLTree {
public:
    struct TreeNode {
        TYPE m_Val;
        int m_nHeight;
        TreeNode* m_pParent;
        TreeNode* m_pLeft;
        TreeNode* m_pRight;
        TreeNode* m_pNext;

        TreeNode(const TYPE& Val,
            TreeNode* pParent = nullptr,
            TreeNode* pLeft = nullptr,
            TreeNode* pRight = nullptr,
            int nHeight = 0) :
            m_Val(Val),
            m_pParent(pParent),
            m_pLeft(pLeft),
            m_pRight(pRight),
            m_nHeight(nHeight) {
            m_pNext = nullptr;
        }
        ~TreeNode() {
            if (m_pNext != nullptr)
                delete m_pNext;
        }
    };
public:
    CAVLTree() {
        m_pRoot = nullptr;
        m_nSize = 0;
    }
    ~CAVLTree() {
        Clear(m_pRoot);
    }

    void Clear(TreeNode* p) {
        if (p == nullptr)
            return;

        Clear(p->m_pLeft);
        Clear(p->m_pRight);
        delete p;
    }
    void Remove(const TYPE& Elem) {
        TreeNode* pDelNode = FindNode(Elem);
        if (pDelNode == nullptr)
            return;

        if (pDelNode->m_pLeft == nullptr && pDelNode->m_pRight == nullptr) {
            TreeNode* pParent = pDelNode->m_pParent;
            if (pParent == nullptr) {
                m_pRoot = nullptr;
            }
            else if (Elem < pParent->m_Val) {
                pParent->m_pLeft = nullptr;
            }
            else {
                pParent->m_pRight = nullptr;
            }

            UpdateHeight(pDelNode->m_pParent);
            delete pDelNode;
            m_nSize--;
        }
        else if (pDelNode->m_pLeft == nullptr || pDelNode->m_pRight == nullptr) {
            TreeNode* pParent = pDelNode->m_pParent;
            TreeNode* pChild = nullptr;
            if (pDelNode->m_pLeft != nullptr)
                pChild = pDelNode->m_pLeft;
            else
                pChild = pDelNode->m_pRight;

            if (pParent == nullptr) {
                m_pRoot = pChild;
            }
            else if (Elem < pParent->m_Val) {
                pParent->m_pLeft = pChild;
            }
            else {
                pParent->m_pRight = pChild;
            }
            pChild->m_pParent = pParent;

            UpdateHeight(pDelNode->m_pParent);
            delete pDelNode;
            m_nSize--;
        }
        else {
            TreeNode* pMaxNode = FindMax(pDelNode->m_pLeft);
            TYPE Val = pMaxNode->m_Val;
            std::swap(pDelNode->m_pNext, pMaxNode->m_pNext);
            Remove(Val);
            pDelNode->m_Val = Val;
        }
    }

    void Insert(const TYPE& Val) {
        if (m_pRoot == nullptr) {
            m_pRoot = new TreeNode(Val);
            m_nSize++;
            return;
        }

        TreeNode* pParent = m_pRoot;
        while (pParent != nullptr) {
            if (Val < pParent->m_Val) {
                if (pParent->m_pLeft == nullptr) {
                    break;
                }
                pParent = pParent->m_pLeft;
            }
            else if (Val > pParent->m_Val) {
                if (pParent->m_pRight == nullptr)
                    break;
                pParent = pParent->m_pRight;
            }
            else {
                TreeNode* pTail = pParent;
                while (pTail->m_pNext != nullptr)
                    pTail = pTail->m_pNext;

                pTail->m_pNext = new TreeNode(Val, pParent);
                return;
            }
        }

        TreeNode* pNewNode = new TreeNode(Val, pParent);
        if (Val < pParent->m_Val) {
            pParent->m_pLeft = pNewNode;
        }
        else {
            pParent->m_pRight = pNewNode;
        }
        UpdateHeight(pParent);
        m_nSize++;
    }

    TreeNode* FindNode(const TYPE& Elem) {
        TreeNode* pCurrent = m_pRoot;
        while (pCurrent != nullptr) {
            if (Elem < pCurrent->m_Val) {
                pCurrent = pCurrent->m_pLeft;
            }
            else if (Elem > pCurrent->m_Val) {
                pCurrent = pCurrent->m_pRight;
            }
            else {
                return pCurrent;
            }
        }
        return nullptr;
    }

    void LevelOrder() {
        const int nWidth = 7;
        const char nSplit = ' ';
        CLinkedQueue<TreeNode*> Queue;
        std::vector<TreeNode*> Vector;
        Queue.push(m_pRoot);
        int nTreeHeight = m_pRoot->m_nHeight;
        int nHeight = m_pRoot->m_nHeight;
        std::cout << "tree:";

        for (int i = 0; i <= nTreeHeight; i++) {
            while (!Queue.Empty()) {
                TreeNode* p = Queue.Peek();
                Queue.pop();

                Vector.push_back(p);
                if (p != nullptr) {
                    Queue.push(p->m_pLeft);
                    Queue.push(p->m_pRight);
                }
                else {
                    Queue.push(nullptr);
                    Queue.push(nullptr);
                }

                if (Vector.size() == pow(2, i)) {
                    break;
                }
            }

            std::cout << std::endl;
            for (int j = 0; j < pow(2, nTreeHeight - i) - 1; j++)
                std::cout << std::setw(nWidth) << nSplit;

            for (auto e : Vector) {
                if (e == nullptr) {
                    std::cout << std::setw(nWidth) << "nil";
                }
                else {
                    while (e != nullptr) {
                        std::cout << e->m_Val << "->";
                        e = e->m_pNext;
                    }
                }
                for (int j = 0; j < pow(2, nTreeHeight - i + 1) - 1; j++)
                    std::cout << std::setw(nWidth) << nSplit;
            }
            Vector.clear();
        }
        std::cout << std::endl;
    }
    void InOrder(TreeNode* pRoot, int nSpace = 0)
    {
        if (pRoot == nullptr)
            return;

        InOrder(pRoot->m_pLeft, nSpace + 4);
        std::cout << pRoot->m_Val.m_Value << ' ';
        InOrder(pRoot->m_pRight, nSpace + 4);
    }
    TreeNode* GetRoot() const {
        return m_pRoot;
    }

    int Size() const {
        return m_nSize;
    }
private:
    inline int Height(TreeNode* p) {
        return p == nullptr ? -1 : p->m_nHeight;
    }

    inline int BalanceFactor(TreeNode* p) {
        if (p == nullptr)
            return 0;
        return Height(p->m_pLeft) - Height(p->m_pRight);
    }

    void UpdateHeight(TreeNode* p) {
        while (p != nullptr) {
            p->m_nHeight = std::max(Height(p->m_pLeft), Height(p->m_pRight)) + 1;

            if (BalanceFactor(p) > 1) {
                if (BalanceFactor(p->m_pLeft) >= 0) {
                    RotateRight(p);
                }
                else {
                    RotateLeft(p->m_pLeft);
                    RotateRight(p);
                }
            }
            else if (BalanceFactor(p) < -1) {
                if (BalanceFactor(p->m_pRight) <= 0) {
                    RotateLeft(p);
                }
                else {
                    RotateRight(p->m_pRight);
                    RotateLeft(p);
                }
            }

            p = p->m_pParent;
        }
    }

    void RotateLeft(TreeNode* K1) {
        TreeNode* P = K1->m_pParent;
        TreeNode* K2 = K1->m_pRight;
        TreeNode* B = K2->m_pLeft;

        K1->m_pParent = K2;
        K1->m_pRight = B;

        K2->m_pParent = P;
        K2->m_pLeft = K1;

        if (B != nullptr)
            B->m_pParent = K1;

        if (P != nullptr) {
            if (P->m_pLeft == K1)
                P->m_pLeft = K2;
            else
                P->m_pRight = K2;
        }
        else
            m_pRoot = K2;

        K1->m_nHeight = std::max(Height(K1->m_pLeft), Height(K1->m_pRight)) + 1;
        K2->m_nHeight = std::max(Height(K2->m_pLeft), Height(K2->m_pRight)) + 1;
    }

    void RotateRight(TreeNode* K1) {
        TreeNode* P = K1->m_pParent;
        TreeNode* K2 = K1->m_pLeft;
        TreeNode* B = K2->m_pRight;

        K1->m_pParent = K2;
        K1->m_pLeft = B;

        K2->m_pParent = P;
        K2->m_pRight = K1;

        if (B != nullptr)
            B->m_pParent = K1;

        if (P != nullptr) {
            if (P->m_pLeft == K1)
                P->m_pLeft = K2;
            else
                P->m_pRight = K2;
        }
        else
            m_pRoot = K2;

        K1->m_nHeight = std::max(Height(K1->m_pLeft), Height(K1->m_pRight)) + 1;
        K2->m_nHeight = std::max(Height(K2->m_pLeft), Height(K2->m_pRight)) + 1;
    }

    TreeNode* FindMax(TreeNode* pRoot) {
        TreeNode* pCurrent = pRoot;

        while (pCurrent != nullptr) {
            if (pCurrent->m_pRight == nullptr)
                break;

            pCurrent = pCurrent->m_pRight;
        }
        return pCurrent;
    }
private:
    TreeNode* m_pRoot;
    int m_nSize;
};