.386
.model flat, stdcall
option casemap:none

include windows.inc
include kernel32.inc
include msvcrt.inc
include linkedlistdll.inc


includelib kernel32.lib
includelib msvcrt.lib
includelib linkedlistdll.lib 

.const
; 日志级别
LOG_DEBUG   equ 0
LOG_INFO    equ 1
LOG_WARNING equ 2
LOG_ERROR   equ 3
szDebug   db "[DEBUG] ", 0
szInfo    db "[INFO] ", 0
szWarning db "[WARN] ", 0
szError   db "[ERROR] ", 0

; 导出函数
PUBLIC LogInit
PUBLIC LogSetLevel
PUBLIC LogFilter
PUBLIC LogMsg

.data
g_nLogLevel dd 0
g_ModuleFilterList dd 0 ;模块过滤链表句柄

szLogBuffer db 1024 dup(0)  ; 缓冲区用于格式化字符串
.code

; 日志初始化
LogInit proc
    invoke LinkedList_Create
    mov g_ModuleFilterList, eax
    ret
LogInit endp

; 设置日志级别
LogSetLevel proc nLevel:DWORD
    mov eax, nLevel
    mov g_nLogLevel, eax
    ret
LogSetLevel endp

; 设置日志过滤 
LogFilter proc szModule:DWORD
    invoke LinkedList_AddPtr, g_ModuleFilterList, szModule
    ret
LogFilter endp

; 输出日志
LogMsg proc c uses esi nLevel:DWORD, szFormat:DWORD, args:VARARG ;VARARG parameter requires C calling convention
    LOCAL @szPrefix:DWORD;
    LOCAL @nFlag:DWORD;过滤标志
    LOCAL @index:DWORD;链表索引
    LOCAL @pModuleName:DWORD
    
    ; 检查日志级别
    ;if (nLevel < g_nLogLevel) return;
    mov eax,nLevel
    cmp eax,g_nLogLevel
    jl done
    
;获取当前模块名并检查是否在过滤列表中
    
    ; 格式化日志内容
	lea esi,args
	invoke crt__vsnprintf,offset szLogBuffer,sizeof szLogBuffer,szFormat,esi
	
	mov @nFlag, 1
	mov @index,0

	;遍历链表
	.while TRUE
		invoke LinkedList_GetPtr,g_ModuleFilterList,@index
		test eax,eax;遍历完了
		jz check_output
		
		
		;字符串对比是否包含模块名
		mov @pModuleName,eax
		invoke crt_strstr,offset szLogBuffer,@pModuleName
		test eax,eax
		;找到
		.if !ZERO?
			mov @nFlag,0
			jmp done
		.endif
		
		inc @index
		
	.endw
	
check_output:;是否需要输出日志
	cmp @nFlag, 0
    je done
	
	; 根据日志级别选择前缀
    .if nLevel == LOG_DEBUG
        mov @szPrefix, offset szDebug
    .elseif nLevel == LOG_INFO
        mov @szPrefix, offset szInfo
    .elseif nLevel == LOG_WARNING
        mov @szPrefix, offset szWarning
    .else
        mov @szPrefix, offset szError
    .endif
    
	; 先输出前缀
    invoke crt_printf, @szPrefix
	; 输出格式化后的内容
    invoke crt_printf, offset szLogBuffer
done:
    ret
LogMsg endp

end	