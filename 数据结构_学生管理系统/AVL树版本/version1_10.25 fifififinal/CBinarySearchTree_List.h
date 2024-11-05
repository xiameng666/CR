#pragma once

#include "CQueue.h"
#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include "CVector.h"
#include<functional>

template<class KEY, class VALUE>
class Pair
{
public:
    Pair(const KEY& Key) : m_Key(Key) {}
    Pair(const KEY& Key, const VALUE& Value) : m_Key(Key), m_Value(Value) {}

    bool operator>(const Pair& Other) const { return m_Key > Other.m_Key; }
    bool operator==(const Pair& Other) const { return m_Key == Other.m_Key; }
    bool operator<(const Pair& Other) const { return m_Key < Other.m_Key; }
    bool operator!=(const Pair& Other) const { return !(m_Key == Other.m_Key); }

    friend std::ostream& operator<< (std::ostream& os, const Pair& s) {
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
            m_nHeight(nHeight),
            m_pNext(nullptr) {}

        ~TreeNode() {
           /* TreeNode* current = m_pNext;
            while (current != nullptr) {
                TreeNode* next = current->m_pNext;
                delete current;
                current = next;
            }*/
            // 只删除直接链接的下一个节点
     /*       delete m_pNext;*/
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

        // 删除链表中的所有节点
        TreeNode* current = p->m_pNext;
        while (current != nullptr) {
            TreeNode* next = current->m_pNext;
            current->m_pNext = nullptr;  // 断开链接，防止重复删除
            delete current;
            current = next;
        }

        delete p;
        p = nullptr;
    }

    void Remove(const TYPE& PrimaryKey, const TYPE& SecondaryKey) {
        TreeNode* pDelNode = FindNode(PrimaryKey);
        if (pDelNode == nullptr)
            return;

        TreeNode* pCurrent = pDelNode;
        TreeNode* pPrev = nullptr;
        while (pCurrent != nullptr) {
            if (pCurrent->m_Val.m_Value == SecondaryKey.m_Value) {
                if (pPrev == nullptr) {
                    // 删除的是链表头（树节点）
                    if (pCurrent->m_pNext != nullptr) {
                        // 有下一个节点，将下一个节点的值复制到当前节点
                        pCurrent->m_Val = pCurrent->m_pNext->m_Val;
                        TreeNode* pTemp = pCurrent->m_pNext;
                        pCurrent->m_pNext = pCurrent->m_pNext->m_pNext;
                        delete pTemp;
                        pTemp = nullptr;
                    }
                    else {
                        // 没有下一个节点，执行正常的AVL树删除操作
                        RemoveNode(pDelNode);
                    }
                }
                else {
                    // 删除的是链表中的非头节点
                    if (pCurrent->m_pNext != nullptr) {
                        pPrev->m_pNext = pCurrent->m_pNext;
                        delete pCurrent;
                        pCurrent = nullptr;
                    }
                    else {
                        pPrev->m_pNext = nullptr;
                        delete pCurrent;
                        pCurrent = nullptr;
                    }
                }
                m_nSize--;
                return;
            }
            pPrev = pCurrent;
            pCurrent = pCurrent->m_pNext;
        }
    }

    void Remove(const TYPE& Elem) {
        TreeNode* pDelNode = FindNode(Elem);
        if (pDelNode == nullptr)
            return;

        TreeNode* pCurrent = pDelNode;
        TreeNode* pPrev = nullptr;
        while (pCurrent != nullptr) {
            if (pCurrent->m_Val.m_Value == Elem.m_Value) {
                if (pPrev == nullptr) {
                    // 删除的是链表头（树节点）
                    if (pCurrent->m_pNext != nullptr) {
                        // 有下一个节点，将下一个节点的值复制到当前节点
                        pCurrent->m_Val = pCurrent->m_pNext->m_Val;
                        TreeNode* pTemp = pCurrent->m_pNext;
                        pCurrent->m_pNext = pCurrent->m_pNext->m_pNext;
                        delete pTemp;
                        pTemp = nullptr;
                    }
                    else {
                        // 没有下一个节点，执行正常的AVL树删除操作
                        RemoveNode(pDelNode);
                    }
                }
                else {
                    // 删除的是链表中的非头节点
                    if (pCurrent->m_pNext != nullptr) {
                        pPrev->m_pNext = pCurrent->m_pNext;
                        delete pCurrent;
                        pCurrent = nullptr;
                    }
                    else {
                        pPrev->m_pNext = nullptr;
                        delete pCurrent;
                        pCurrent = nullptr;
                    }
                }
                m_nSize--;
                return;
            }
            pPrev = pCurrent;
            pCurrent = pCurrent->m_pNext;
        }
    }

    bool RemoveNode(TreeNode* pDelNode) {
        if (pDelNode == nullptr)
            return false;

        TreeNode* pParent = pDelNode->m_pParent;

        if (pDelNode->m_pLeft == nullptr && pDelNode->m_pRight == nullptr) {
            // 叶子节点
            if (pParent == nullptr) {
                m_pRoot = nullptr;
            }
            else if (pDelNode == pParent->m_pLeft) {
                pParent->m_pLeft = nullptr;
            }
            else {
                pParent->m_pRight = nullptr;
            }

            delete pDelNode;
            pDelNode = nullptr;
            m_nSize--;
        }
        else if (pDelNode->m_pLeft == nullptr || pDelNode->m_pRight == nullptr) {
            // 只有一个子节点
            TreeNode* pChild = (pDelNode->m_pLeft != nullptr) ? pDelNode->m_pLeft : pDelNode->m_pRight;

            if (pParent == nullptr) {
                m_pRoot = pChild;
            }
            else if (pDelNode == pParent->m_pLeft) {
                pParent->m_pLeft = pChild;
            }
            else {
                pParent->m_pRight = pChild;
            }
            pChild->m_pParent = pParent;

            delete pDelNode;
            pDelNode = nullptr;
            m_nSize--;
        }
        else {
            // 有两个子节点
            TreeNode* pMaxNode = FindMax(pDelNode->m_pLeft);
            TYPE Val = pMaxNode->m_Val;
            std::swap(pDelNode->m_pNext, pMaxNode->m_pNext);
            RemoveNode(pMaxNode);
            pDelNode->m_Val = Val;
        }

        // 更新高度并重新平衡
        UpdateHeightAndBalance(pParent);
        return true;
    }

    void UpdateHeightAndBalance(TreeNode* node) {
        while (node != nullptr) {
            UpdateHeight(node);
            int balance = BalanceFactor(node);
            if (std::abs(balance) > 1) {
                Rebalance(node);
            }
            node = node->m_pParent;
        }
    }

    TreeNode* FindMin(TreeNode* pRoot) {
        while (pRoot->m_pLeft != nullptr) {
            pRoot = pRoot->m_pLeft;
        }
        return pRoot;
    }


    void Insert(const TYPE& Val) {
        if (m_pRoot == nullptr) {
            m_pRoot = new TreeNode(Val);
            m_nSize++;
            return;
        }

        TreeNode* pParent = m_pRoot;
        while (pParent != nullptr) {
            if (Val.m_Key < pParent->m_Val.m_Key) {
                if (pParent->m_pLeft == nullptr) {
                    break;
                }
                pParent = pParent->m_pLeft;
            }
            else if (Val.m_Key > pParent->m_Val.m_Key) {
                if (pParent->m_pRight == nullptr)
                    break;
                pParent = pParent->m_pRight;
            }
            else {
                // 如果键相同，插入到链表中
                TreeNode* pNewNode = new TreeNode(Val, pParent);
                pNewNode->m_pNext = pParent->m_pNext;
                pParent->m_pNext = pNewNode;
                m_nSize++;
                return;
            }
        }

        TreeNode* pNewNode = new TreeNode(Val, pParent);
        if (Val.m_Key < pParent->m_Val.m_Key) {
            pParent->m_pLeft = pNewNode;
        }
        else {
            pParent->m_pRight = pNewNode;
        }
        UpdateHeight(pParent);
        m_nSize++;
    }

    //找树节点
    TreeNode* FindNode(const TYPE& Elem) {
        TreeNode* pCurrent = m_pRoot;
        while (pCurrent != nullptr) {
            if (Elem.m_Key < pCurrent->m_Val.m_Key) {
                pCurrent = pCurrent->m_pLeft;
            }
            else if (Elem.m_Key > pCurrent->m_Val.m_Key) {
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
        CVector<TreeNode*> Vector;
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
    void DeleteList(TreeNode* head) {
        head = head->m_pNext;//忽略树节点
        while (head != nullptr) {
            TreeNode* next = head->m_pNext;
            delete head;
            head = next;
        }
    }

    bool IsLeaf(TreeNode* node) {
        return node->m_pLeft == nullptr && node->m_pRight == nullptr;
    }

    bool HasOneChild(TreeNode* node) {
        return (node->m_pLeft == nullptr) != (node->m_pRight == nullptr);
    }

    void DeleteLeaf(TreeNode* node) {
        if (node->m_pParent == nullptr) {
            m_pRoot = nullptr;
        }
        else if (node == node->m_pParent->m_pLeft) {
            node->m_pParent->m_pLeft = nullptr;
        }
        else {
            node->m_pParent->m_pRight = nullptr;
        }
        delete node;
        node = nullptr;
    }

    void DeleteNodeWithOneChild(TreeNode* node) {
        TreeNode* child = (node->m_pLeft != nullptr) ? node->m_pLeft : node->m_pRight;
        ReplaceNode(node, child);
        delete node;
        node = nullptr;
    }

    void DeleteNodeWithTwoChildren(TreeNode* node) {
        TreeNode* successor = FindMin(node->m_pRight);
        node->m_Val = successor->m_Val;
        RemoveNode(successor);
    }

    //用一个新节点替换树中的一个旧节点
    void ReplaceNode(TreeNode* oldNode, TreeNode* newNode) {
        if (oldNode->m_pParent == nullptr) {
            // 如果旧节点是根节点
            m_pRoot = newNode;
        }
        else if (oldNode == oldNode->m_pParent->m_pLeft) {
            // 如果旧节点是其父节点的左子节点
            oldNode->m_pParent->m_pLeft = newNode;
        }
        else {
            // 如果旧节点是其父节点的右子节点
            oldNode->m_pParent->m_pRight = newNode;
        }

        // 更新新节点的父节点
        if (newNode != nullptr) {
            newNode->m_pParent = oldNode->m_pParent;
        }
    }



    void Rebalance(TreeNode* node) {
        if (BalanceFactor(node) > 1) {
            if (BalanceFactor(node->m_pLeft) >= 0) {
                RotateRight(node);
            }
            else {
                RotateLeft(node->m_pLeft);
                RotateRight(node);
            }
        }
        else if (BalanceFactor(node) < -1) {
            if (BalanceFactor(node->m_pRight) <= 0) {
                RotateLeft(node);
            }
            else {
                RotateRight(node->m_pRight);
                RotateLeft(node);
            }
        }
    }



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


public:
    void PrintTree() {
        PrintTreeRecursive(m_pRoot, 0);
    }

    TreeNode* FindNodeWithTraversal(const TYPE& Elem, std::function<bool(const TYPE&)> predicate) {
        return FindNodeWithTraversalHelper(m_pRoot, Elem, predicate);
    }

private:
    void PrintTreeRecursive(TreeNode* node, int depth) {
        if (node == nullptr) {
            return;
        }

        // 先打印右子树
        PrintTreeRecursive(node->m_pRight, depth + 1);

        // 打印当前节点
        std::string indent(depth * 4, ' ');
        std::cout << indent << node->m_Val.m_Key << ": " << node->m_Val.m_Value;

        // 打印链表
        TreeNode* listNode = node->m_pNext;
        while (listNode != nullptr) {
            std::cout << " -> " << listNode->m_Val.m_Value;
            listNode = listNode->m_pNext;
        }
        std::cout << std::endl;

        // 最后打印左子树
        PrintTreeRecursive(node->m_pLeft, depth + 1);
    }

    TreeNode* FindNodeWithTraversalHelper(TreeNode* node, const TYPE& Elem, std::function<bool(const TYPE&)> predicate) {
        if (node == nullptr) {
            return nullptr;
        }

        if (Elem < node->m_Val) {
            return FindNodeWithTraversalHelper(node->m_pLeft, Elem, predicate);
        } else if (Elem > node->m_Val) {
            return FindNodeWithTraversalHelper(node->m_pRight, Elem, predicate);
        } else {
            // 找到匹配的键，现在遍历链表
            TreeNode* current = node;
            while (current != nullptr) {
                if (predicate(current->m_Val)) {
                    return current;
                }
                current = current->m_pNext;
            }
            return nullptr;
        }
    }
};
