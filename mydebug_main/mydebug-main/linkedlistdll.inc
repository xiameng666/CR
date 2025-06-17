; 链表操作函数声明


LinkedList_Create PROTO c                          ; void* LinkedList_Create()
LinkedList_Destroy PROTO c :DWORD                  ; void LinkedList_Destroy(void* hList)  std::list的析构函数会自动释放所有节点?
LinkedList_AddPtr PROTO c :DWORD, :DWORD          ; BOOL LinkedList_AddPtr(void* hList, const char* ptr)
LinkedList_GetPtr PROTO c :DWORD, :DWORD          ; const char* LinkedList_GetPtr(void* hList, DWORD index)
LinkedList_GetSize PROTO c :DWORD                  ; DWORD LinkedList_GetSize(void* hList)
LinkedList_RemoveAt PROTO c:DWORD, :DWORD           ; BOOL LinkedList_RemoveAt(void* hList, DWORD index)
LinkedList_CompareData PROTO c:DWORD, :DWORD,:DWORD, :DWORD   ;DWORD LinkedList_CompareData(void* hList, const void* pBuffer, DWORD offset, DWORD compareSize)
;遍历链表节点memcmp 找到返回找到的索引 不然-1
