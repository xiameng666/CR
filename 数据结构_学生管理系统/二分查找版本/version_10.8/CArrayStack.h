template<typename TYPE>
class CArrayStack {
public:
    CArrayStack(int nInitSize = 1) {
        m_pBuf = new TYPE[nInitSize];
        m_nSize = 0;
        m_nCapacity = nInitSize;
    }
    ~CArrayStack() {
        delete[] m_pBuf;
    }
    void push(const TYPE& Elem) {
        if (m_nSize == m_nCapacity) {
            // 如果栈满，扩展容量
            m_nCapacity *= 2;
            TYPE* newBuf = new TYPE[m_nCapacity];
            for (int i = 0; i < m_nSize; i++) {
                newBuf[i] = m_pBuf[i];
            }
            delete[] m_pBuf;
            m_pBuf = newBuf;
        }
        m_pBuf[m_nSize++] = Elem;
    }
    void pop() {
        if (m_nSize > 0) {
            m_nSize--;
        }
    }
    TYPE Peek() const {
        if (m_nSize > 0) {
            return m_pBuf[m_nSize - 1];
        }
        throw std::runtime_error("栈为空");
    }
    bool Empty() const {
        return m_nSize == 0;
    }
    void Clear() {
        m_nSize = 0;
    }
    int Size() const {
        return m_nSize;
    }
private:
    TYPE* m_pBuf;
    int   m_nSize;
    int   m_nCapacity;
};