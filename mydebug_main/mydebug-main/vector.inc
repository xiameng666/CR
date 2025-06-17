.const
; 断点常量

; 断点数组和页表数组
.data
g_MemBreakpoints MemBpEntry MAX_MEM_BREAKPOINTS dup(<>)  ; 内存断点数组
;g_PageEntries PageEntry MAX_PAGE_ENTRIES dup(<>)          ; 页表数组
g_NextBpId     dd 1000                                    ; 下一个断点ID
g_StepMemBpPage dd 0                                      ; 当前单步的页基址

g_BpHitReason  db 0                                       ; 断点命中原因

