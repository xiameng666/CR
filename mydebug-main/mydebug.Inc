include windows.inc
include kernel32.inc
include user32.inc

includelib kernel32.lib
includelib user32.lib
includelib dasm.lib

include msvcrt.inc
includelib msvcrt.lib

; 导出函数
PUBLIC StartDebug 
PUBLIC AttachDebug
PUBLIC DetachDebug
PUBLIC StopDebug

; 反汇编引擎函数声明
Decode2Asm PROTO  :DWORD,:DWORD,:DWORD,:DWORD
;	Decode2Asm(IN PBYTE pCodeEntry,   // 需要解析指令地址
;           OUT char* strAsmCode,  // 得到反汇编指令信息
;           OUT UINT* pnCodeSize,  // 解析指令长度
;           UINT nAddress);  


;mydebug.Asm函数声明
;工具函数
DbgGetCmd PROTO fgetsBuf:DWORD
DbgReadMemory PROTO lpBaseAddress:DWORD, lpBuffer:DWORD, nSize:DWORD
DbgWriteMemory PROTO lpBaseAddress:DWORD, lpBuffer:DWORD, nSize:DWORD
DbgGetRegs PROTO :DWORD  ; 参数是 CONTEXT 结构指针
DbgSetRegs PROTO :DWORD
DbgShowRegs PROTO :DWORD
DbgDiasm PROTO :DWORD,:UINT  ; 地址和行数
DbgSetBpSoft PROTO :DWORD;
DbgSetBpMemory PROTO  lpAddress:DWORD, dwSize:DWORD, dwType:DWORD
DbgSetStep proto 
DbgMemoryProtected proto  lpBaseAddress:DWORD, dwSize:DWORD, flNewProtect:DWORD
DbgSetBpHard proto dwAddr:DWORD, dwType:DWORD, dwSize:DWORD
DbgGoRet proto
DbgRecard proto
DbgGetPEinfo proto 

;事件函数
EventException PROTO
EventCreateProcess PROTO
EventCreateThread PROTO
EventExitThread PROTO
EventExitProcess PROTO
EventLoadDll PROTO
EventUnLoadDll PROTO
EventOutputDebugString PROTO
EventRip PROTO

;异常处理
ExecBreakPoint PROTO
ExecAccess PROTO  
ExecStep PROTO

;GetCmd PROTO

;命令
CmdRun PROTO:DWORD
CmdSetSingleStep PROTO:DWORD
CmdDiasm PROTO:DWORD
CmdSetBreakPoint PROTO :DWORD
CmdListBreakPoint proto:DWORD
CmdNext PROTO:DWORD
CmdGoRet PROTO szArgs:DWORD
CmdLogCmd PROTO szArgs:DWORD

PrintHardBpVec proto
PrintSoftBpVec proto
PrintMemBpVec proto
CmdDeleteBp proto szArgs:DWORD 
DbgDeleteSoftBp proto dwBpId:DWORD
DbgDeleteMemBp proto dwBpId:DWORD
DbgDeleteHardBp proto dwBpId:DWORD
DbgGetApiName proto lpAddress:DWORD, lpBuffer:DWORD
        
;断点链表

Command STRUCT
    szCmd DWORD ?    ;
    pfnCmd DWORD ?   ;
Command ENDS


SoftBpInfo struct
;数组下标作为id打印
    Address  dd ?
    OldCode db ?
    p1		db ?
    p2		db ?
    p3		db ?
    Enable  dd 0    
SoftBpInfo ends

MemBpInfo struct
;数组下标作为id打印
    StartAddr  dd ?
    EndAddr dd ?
     dwSize    dd ?  
  ;  OldProto 直接从g_oldprotAry恢复 不需要保存 ;NewProtect 默认PAGE_NOACCESS 就不写结构体里了
    dwType	dd ?
    TempDisabled dd 0	;临时禁用标志
	Enable  dd 0       
MemBpInfo ends


; 硬件断点结构体
HardBpInfo STRUCT
    Address     DWORD ?    ; 断点地址
    Index       DWORD ?    ; 调试寄存器索引(0-3)
    dwType      DWORD ?    ; 访问类型(0=执行,1=写入,3=读写)
    dwSize      DWORD ?    ; 访问大小(1,2,4字节)
    Enable      DWORD 0    ; 是否启用
HardBpInfo ENDS

DR7_REG STRUCT
    L0      BYTE 0:1   ; 局部断点0启用
    G0      BYTE 0:1   ; 全局断点0启用
    L1      BYTE 0:1   ; 局部断点1启用
    G1      BYTE 0:1   ; 全局断点1启用
    L2      BYTE 0:1   ; 局部断点2启用
    G2      BYTE 0:1   ; 全局断点2启用
    L3      BYTE 0:1   ; 局部断点3启用
    G3      BYTE 0:1   ; 全局断点3启用
    reg7LE      BYTE 0:1   ; 局部精确断点    避免和宏冲突
    reg7GE      BYTE 0:1   ; 全局精确断点    避免和宏冲突
    Reserved1 BYTE 0:3 ; 保留位
    GD      BYTE 0:1   ; 全局调试寄存器保护
    Reserved2 BYTE 0:2 ; 保留位
    RW0     BYTE 0:2   ; 断点0类型(0=执行,1=写入,3=读写)
    LEN0    BYTE 0:2   ; 断点0大小(0=1字节,1=2字节,3=4字节)
    RW1     BYTE 0:2   ; 断点1类型
    LEN1    BYTE 0:2   ; 断点1大小
    RW2     BYTE 0:2   ; 断点2类型
    LEN2    BYTE 0:2   ; 断点2大小
    RW3     BYTE 0:2   ; 断点3类型
    LEN3    BYTE 0:2   ; 断点3大小
DR7_REG ENDS


.const
szDebugPath db"TestDebug.exe",0
 ;szDebugPath db"winmine.exe",0
   ;szDebugPath db"Hello_pack.exe",0
;------------------------------------------

E_OK  equ 0
E_ERR equ -1

LOG_DEBUG   equ 0    
LOG_INFO    equ 1    
LOG_WARNING equ 2    
LOG_ERROR   equ 3  
 
CONTEXT_ALL            equ 0001001Fh  ; CONTROL | INTEGER | SEGMENTS | FLOATING_POINT | DEBUG_REGISTERS

szDbgSetMemBp db "DbgSetBpMemory: addr=%08X, end=%08X, type=%c", 0dh, 0ah, 0
szErrAllocMemBp db "错误: 无法分配内存断点", 0dh, 0ah, 0
szFmtBpListCmd db "%c", 0             ; 解析断点列表命令类型
szFmtBpListPage db "p %x", 0          ; 解析页面断点列表命令    
    szDebugInfo     db "[DEBUG] %s", 0dh, 0ah, 0
    
; DbgSetBpMemory调试信息
szDbgBpParams    db "断点参数: 地址=0x%08X 大小=%d 类型=%c", 0dh, 0ah, 0
szDbgBpList      db "断点链表地址: 0x%08X", 0dh, 0ah, 0
szDbgBpAlloc     db "断点结构分配结果: 0x%08X", 0dh, 0ah, 0
szDbgBpBeforeAdd db "添加前检查: 链表=0x%08X 断点=0x%08X", 0dh, 0ah, 0
szDbgBpAfterAdd  db "添加到链表结果: %d", 0dh, 0ah, 0
szDbgBpComplete  db "内存断点设置完成: 地址=0x%08X 大小=%d 类型=%c", 0dh, 0ah, 0
szMemBpHit          db "MemBpHit: ID=%d range=[0x%08X-0x%08X] type=%c addr=0x%08X", 0dh, 0ah, 0
szBpTempDisabled  db "temp disable bp: ID=%d start=%p end=%p", 0dh, 0ah, 0
;szBpHit           db "BpHit: ID=%d 范围=[%p-%p] 类型=%c 访问地址=%p", 0dh, 0ah, 0
szTempDisablePage      db "临时禁用页面(0x%08X)，页面上共有%d个断点", 0dh, 0ah, 0
szBpRestored    db "恢复断点: ID=%d 范围=[%p-%p]", 0dh, 0ah, 0
szRestoredCount db "共恢复%d个断点的保护属性", 0dh, 0ah, 0
szBpAlreadyExists  db "szBpAlreadyExists: addr=%p", 0dh, 0ah, 0
szBpReadFailed     db "读取断点地址失败: 地址=%p", 0dh, 0ah, 0
szDiasmWithBp db "%08X  [BP] %s", 0Ah, 0
szDiasmNormal db "%08X  %s", 0Ah, 0
    szMemBpFormat db "Memory Breakpoint ID: %d Start Address: 0x%08X End Address: 0x%08X Size: %d bytes Type: %d Temporary Disabled: %d Enabled: %d", 0Ah, 0Ah, 0
                  
	szSoftBpFormat db "Software Breakpoint ID: %d Address: 0x%08X Old Code: 0x%02X Enabled: %d", 0Ah, 0Ah, 0
.data
;断点
	;内存保护属性区域   
    MAX_MEMORY_REGIONS    equ 8000
   	g_oldprotStart 		 equ 00400000h
    g_oldprotAry         dd MAX_MEMORY_REGIONS dup(0) ;保存从xxx开始的oldprotect
    
    MAX_MEMBP_COUNT EQU 10
    g_memBpAry MemBpInfo MAX_MEMBP_COUNT dup(<>) 
    
	MAX_SOFTBP_COUNT EQU 10
    g_softBpAry SoftBpInfo MAX_SOFTBP_COUNT dup(<>) 
    

; 命令字符串
szCmdG  db "g", 0 
szCmdU  db "u", 0
szCmdT  db "t", 0
szCmdP  db "p", 0
szCmdBP db "bp", 0       
szCmdBH db "bh", 0       
szCmdBM db "bm", 0       
szCmdLM db "lm", 0     
szCmdD db "d", 0 
szCmdE db "e", 0 
szCmdR db "r", 0       ; 执行到返回
szCmdREG db "a", 0    
szCmdTrace db "trace", 0  ; 执行脚本
szCmdRecord db "record", 0  ; 开始/停止记录
szCmdHistory db "history", 0 ; 显示历史记录
szCmdSave db "t", 0     ; 保存历史记录
szCmdBat db "bat", 0 
szCmdDelete db "bc", 0 

szCmdBL db "bl", 0
szCmdBLS db "bls", 0
szCmdBLM db "blm", 0
szCmdBLH db "blh", 0
szCmdBLP db "blp", 0

szCmdDump db "dump", 0
szLogCmd db "log", 0
; 命令表
g_Commands  Command <offset szCmdG, offset CmdRun>
			Command <offset szCmdU, offset CmdDiasm>
			Command <offset szCmdT, offset CmdSetSingleStep>
			Command <offset szCmdP, offset CmdNext>
			Command <offset szCmdBP, offset CmdSetBpSoftware>
			Command <offset szCmdBH, offset CmdSetBpHardware>
			Command <offset szCmdBM, offset CmdSetBpMemory>
			Command <offset szCmdBL, offset CmdListBreakPoint>
			Command <offset szCmdR, offset CmdGoRet>
			Command <offset szCmdD, offset CmdDumpMem>
			Command <offset szCmdLM, offset CmdListModule>
			Command <offset szCmdE, offset CmdEditMem> 
			Command <offset szCmdTrace, offset CmdTrace>
			Command <offset szCmdREG, offset CmdShowRegisters>
			Command <offset szCmdBat, offset CmdBat>
			Command <offset szCmdDelete, offset CmdDeleteBp>
			Command <offset szCmdDump, offset CmdDumpPE>
			Command <offset szLogCmd, offset CmdLogCmd>
           
; 命令表项数
g_nCommands dd ($ - g_Commands) / sizeof Command

; 命令格式化

g_szCmdPrompt db "cmd: ", 0
;g_szFmtCmdArgs db "%s %s", 0
g_szFmtCmdArgs db "%s %[^\n]", 0  ; 第一个是命令，之后所有内容是参数
g_szCmdUnknown db "Unknown command", 0dh, 0ah, 0
;-----------------------------------------------------------------
g_lpAddress dd 0         ; 反汇编地址
g_lpBpAddress dd 0       ; 断点地址
g_DebugEv DEBUG_EVENT <>  ; 全局调试事件结构
g_IsSytemBp db 1 ;是不是系统断点
  g_hProcess      HANDLE 0    ; 被调试进程的句柄
g_nBpCode db 0            ; 断点处的原始代码
g_IsBpStep db 0 ;断点后单步标志
g_IsStep db 0 ;主动-t 单步标志
g_IsMemStep dd 0        ; 内存断点单步标志

g_isFindRet dd 0  ;执行到返回标志

g_MemPageToRestore   DWORD 0       ; 需要恢复的页面地址
g_MemProtToRestore   DWORD PAGE_NOACCESS ; 需要设置的保护属性

g_hRecordFile  dd 0         ; 记录文件句柄
g_nRecordCount dd 0         ; 指令记录计数
g_setbp dd 0
szCall db "CALL", 0 
szRet db "RET", 0 

g_isCmdNext dd 0
;hList dd 0    ; 链表

szInvalidCmd db "Invalid Commandline", 0dh, 0ah, 0
;debug日志format
szLogStart db "StartDebug lpApplicationName:%s lpCommandLine:%s", 0dh, 0ah, 0
szLogCreateProcess db "StartDebug CreateProcess dwProcessId:%d dwThreadId:%d", 0dh, 0ah, 0
szLogCreateError db "StartDebug CreateProcess ErrCode:%08x", 0dh, 0ah, 0

;event
szDbgReadMemory db "DbgReadMemory", 0
szEventLoadDll db "EventLoadDll", 0
szEventExitThread db "EventExitThread", 0
szEventUnLoadDll db "EventUnLoadDll", 0
szEventCreateThread db "EventCreateThread", 0
szCreateProcessInfo db "[%s] lpBaseOfImage:%p lpStartAddress:%p TEB:%p", 0dh, 0ah, 0
    g_szEXCEPTION_DEBUG_EVENT           db "EXCEPTION_DEBUG_EVENT",0dh,0ah, 0

    g_szCREATE_THREAD_DEBUG_EVENT       db "CREATE_THREAD_DEBUG_EVENT",0dh,0ah,0

    g_szCREATE_PROCESS_DEBUG_EVENT      db "CREATE_PROCESS_DEBUG_EVENT", 0

    g_szEXIT_THREAD_DEBUG_EVENT         db "EXIT_THREAD_DEBUG_EVENT",0dh,0ah, 0

    g_szEXIT_PROCESS_DEBUG_EVENT        db "EXIT_PROCESS_DEBUG_EVENT",0dh,0ah, 0

    g_szLOAD_DLL_DEBUG_EVENT            db "LOAD_DLL_DEBUG_EVENT" , 0dh,0ah, 0

    g_szUNLOAD_DLL_DEBUG_EVENT          db "UNLOAD_DLL_DEBUG_EVENT" , 0dh,0ah, 0

    g_szOUTPUT_DEBUG_STRING_EVENT       db "OUTPUT_DEBUG_STRING_EVENT" , 0dh,0ah, 0
    
; 工具函数相关
szDbgRead db "DbgReadMemory: Reading %08X bytes from %08X", 0dh, 0ah, 0
szDbgWrite db "DbgWriteMemory: Writing %08X bytes to %08X", 0dh, 0ah, 0

szDbgReadSuccess db "DbgReadMemory: Successfully read %d bytes", 0dh, 0ah, 0
szDbgWriteSuccess db "DbgWriteMemory: Successfully write %d bytes", 0dh, 0ah, 0

szDbgOpenError db "DbgReadMemory OpenProcess ErrCode=%08X", 0dh, 0ah, 0
szDbgReadError db "DbgReadMemory ReadProcessMemory ErrCode=%08X", 0dh, 0ah, 0
szDbgWriteError db "DbgWriteMemory WriteProcessMemory ErrCode=%08X", 0dh, 0ah, 0
szDbgReadSizeError db "DbgReadMemory: Size mismatch - read %d of %d bytes", 0dh, 0ah, 0
szDbgWriteSizeError db "DbgWriteMemory: Size mismatch - write %d of %d bytes", 0dh, 0ah, 0



; Regs
szOpenThreadError db "Dbg Regs: Failed to open thread, ErrCode=%08X", 0dh, 0ah, 0
szGetContextError db "DbgGetRegs: Failed to get thread context, ErrCode=%08X", 0dh, 0ah, 0
szSetContextError db "DbgSetRegs: Failed to set thread context, ErrCode=%08X", 0dh, 0ah, 0
; 寄存器显示格式
szFmtRegs1 db "EAX=%08X EBX=%08X ECX=%08X EDX=%08X", 0dh, 0ah, 0
szFmtRegs2 db "ESI=%08X EDI=%08X EBP=%08X ESP=%08X", 0dh, 0ah, 0
szFmtRegs3 db "EIP=%08X EFL=%08X", 0dh, 0ah, 0

; DLL加载事件相关
szLoadDllInfo db "LoadDll: %s BaseAddr=%08X Handle=%08X", 0dh, 0ah, 0
szLoadDllError db "LoadDll: Failed to read DLL information", 0dh, 0ah, 0

;异常相关
;szExceptionInfo db "Exception: Code=%08X Address=%08X Flags=%08X", 0dh, 0ah, 0
szExeceptionInfo db "%s ExceptionCode:%08X ExceptionAddress:%p ExceptionFlags:%08X NumberParameters:%d", 0dh, 0ah, 0
szExceptionAccessInfo db "%s ExceptionCode:%08X ExceptionAddress:%p ExceptionFlags:%08X NumberParameters:%d AccessType:%d AccessAddr:%08X", 0dh, 0ah, 0
szExecBreakPoint db "ExecBreakPoint", 0
szExecStep db "ExecStep", 0
szExecAccess db "ExecAccess", 0
; 反汇编相关字符串
szFmtDisasmAddr db "%08X: ", 0
szFmtDisasmByte db "%02X ", 0
szFmtDisasmSpace db "   ", 0
szFmtDisasmInst db "%s", 0dh, 0ah, 0
szDisasmError db "DbgDiasm: Failed to read memory, ErrCode=%08X", 0dh, 0ah, 0

;cmd相关
szFmtHex db "%x", 0
szFmtBpMem db "%c %d %x", 0    ; 内存断点格式: 类型 大小 地址
szFmtBpHard db "%d %d %x", 0 
szDebugParams db "%d   %d   %x", 0 
szBpMemArgs db "内存断点参数: '%s', 类型=%c, 大小=%d, 地址=0x%08X", 0dh, 0ah, 0
szBpMemAdded db "CmdSetBpMemory: DbgSetBpMemory err  地址=0x%08X  类型=%c", 0dh, 0ah, 0
szBpAddrInvalid db "Invalid breakpoint address", 0dh, 0ah, 0
szBpList db "Breakpoint at %08X", 0dh, 0ah, 0
szBpNone db "No breakpoints set", 0dh, 0ah, 0


szDbgMemoryProtected       db "DbgMemoryProtected: 修改地址 0x%08X 大小 %d 的保护属性为 0x%08X", 0dh, 0ah, 0
szDbgMemoryProtectedError  db "DbgMemoryProtected: 修改内存保护属性失败，错误码: %08X", 0dh, 0ah, 0
szDbgMemoryProtectedSuccess db "DbgMemoryProtected: 成功修改保护属性，原保护: 0x%08X", 0dh, 0ah, 0

; 调试输出格式
szInvalidProcessHandle db "Invalid process handle", 0dh, 0ah, 0
szSoftBpHit      db "命中软件断点: ID=%d 地址=%p", 0dh, 0ah, 0
szSoftBpRestored db "恢复软件断点: ID=%d 地址=%p", 0dh, 0ah, 0

 
    
;硬件断点   
    g_HardBpArray HardBpInfo 4 dup(<>)  ; 4个硬件断点结构体数组
    g_HardBpCount DWORD 0                ; 当前使用的硬件断点数量
    g_IsHardStep DWORD 0                 ; 硬件断点单步模式标志
    g_LastHitIndex DWORD -1              ; 最近命中的断点索引
    g_IsHardIndex dd 0   ; 断点索引
	szHardBpFull db "硬件断点已满，无法添加更多断点", 0dh, 0ah, 0
    szHardBpAdded db "已添加硬件断点: 地址=0x%08X, 索引=%d, 类型=%d, 大小=%d", 0dh, 0ah, 0
    szHardBpRemoved db "已移除硬件断点: 索引=%d", 0dh, 0ah, 0
    szBpRemoved db "步过断点已移除: 地址=%08X", 0Ah, 0
	szBpCmdArgs    db "命令参数: %s", 0dh, 0ah, 0 ; 记录命令参数
	szHardBpListEntry  db "%d    %c     %d    %08X", 0dh, 0ah, 0
szHardBpListFooter db "------------------------", 0dh, 0ah
                   db "共 %d 个硬件断点", 0dh, 0ah, 0
szDebugDR  db "调试寄存器: DR0=%08X DR1=%08X DR2=%08X DR3=%08X DR7=%08X", 0dh, 0ah, 0
szHardBpSet db "已设置断点 #%d 在地址 %08X", 0dh, 0ah, 0
szHardBpSetFailed db "硬件断点设置失败: 索引=%d 地址=%08X", 0dh, 0ah, 0
szHardBpDR7Failed db "DR7设置失败: 期望=%08X 实际=%08X", 0dh, 0ah, 0
szHardBpTriggered db "非执行硬件断点触发: DR6=%08X", 0dh, 0ah, 0

szDR7Modified  db "修改后DR7=%08X", 0dh, 0ah, 0
szDR7Bits      db "DR7寄存器位分析:", 0dh, 0ah
               db "L0=%d G0=%d L1=%d G1=%d L2=%d G2=%d L3=%d G3=%d", 0dh, 0ah
               db "LE=%d GE=%d GD=%d", 0dh, 0ah
               db "RW0=%d LEN0=%d RW1=%d LEN1=%d", 0dh, 0ah
               db "RW2=%d LEN2=%d RW3=%d LEN3=%d", 0dh, 0ah, 0
               
               szHardBpParams db "DbgSetBpHard参数: 地址=0x%08X, 类型=%d, 大小=%d", 0Ah, 0
szHardBpArrayAddr db "g_HardBpArray地址: 0x%08X", 0Ah, 0
szHardBpContent db "断点[%d]: 启用=%d, 地址=0x%08X, 类型=%d, 大小=%d", 0Ah, 0

szFoundReturn db "找到返回指令(RET)，地址: 0x%08X", 0Ah, 0

szRetDetected db "检测到函数返回点", 0Ah, 0

; 添加缺少的全局变量
g_dwEntryEBP dd 0   ; 保存进入函数时的EBP值
g_dwLastEIP dd 0    ; 保存上一次执行的EIP值

; 字符串定义
szSnapError db "错误：无法创建系统快照", 0Ah, 0
szNoModules db "错误：没有找到模块", 0Ah, 0
szModuleHeader db "模块名称               基址      大小      路径", 0Ah, \
                 "-----------------------------------------------------", 0Ah, 0
szModuleFmt db "%-20s 0x%08X 0x%08X %s", 0Ah, 0
g_pid dd 0

; 内存查看相关
szMemDumpAddr db "%08X: ", 0
szMemDumpByte db "%02X ", 0
szCRLF db 0Dh, 0Ah, 0
g_lpLastDumpAddr dd 0
szReadMemError db "CmdDumpMem read error lpAddress=%08X", 0dh, 0ah, 0

;修改寄存器
; 寄存器名称
szRegEAX db "EAX", 0
szRegEBX db "EBX", 0
szRegECX db "ECX", 0
szRegEDX db "EDX", 0
szRegESI db "ESI", 0
szRegEDI db "EDI", 0
szRegEBP db "EBP", 0
szRegESP db "ESP", 0
szRegEIP db "EIP", 0
szRegEFL db "EFL", 0
szRegDR0 db "DR0", 0
szRegDR1 db "DR1", 0
szRegDR2 db "DR2", 0
szRegDR3 db "DR3", 0
szRegDR6 db "DR6", 0
szRegDR7 db "DR7", 0

szRegDbgMsg db "寄存器命令: %s", 0
szRegGetError db "无法获取寄存器状态", 0Dh, 0Ah, 0
szRegSetError db "无法设置寄存器值", 0Dh, 0Ah, 0
szRegSetFmt db "%s %x", 0
szRegNameFmt db "%s", 0
szUnknownReg db "未知寄存器: %s", 0Dh, 0Ah, 0
szRegUpdated db "寄存器 %s 已更新为 0x%08X", 0Dh, 0Ah, 0
szRegValue db "%s=0x%08X", 0Dh, 0Ah, 0

;trace
g_isTrace dd 0 
szRecardFileName db "trace.txt",0
szFileError db "无法创建文件 %s", 0Dh, 0Ah, 0
szTraceFormat db "%08X: %s", 0Dh, 0Ah, 0  

;bat
g_bScriptRunning  dd 0            ; 脚本执行标志：1=脚本正在执行
g_szScriptFile    db 256 dup(0)   ; 当前执行的脚本文件名
g_pScriptFile     dd 0            ; 脚本文件指针
g_nScriptCmdCount dd 0            ; 已执行命令计数

MAX_SCRIPT_LINES equ 1000  ; 最大脚本行数
ScriptLines db MAX_SCRIPT_LINES * 256 dup(0)  ; 每行最大256字节
g_nScriptLines DWORD 0    ; 脚本总行数
g_nCurrentLine DWORD 0    ; 当前执行的行索引

; 字符串常量
szReadMode      db "r", 0                                   ; 文件打开模式：只读文本
szBatUsage      db "用法: bat <脚本文件名>", 0Dh, 0Ah, 0     ; 用法信息
szScriptError   db "无法打开脚本文件 %s", 0Dh, 0Ah, 0        ; 错误信息
szExecuteCmdWithArgs db "执行命令: %s %s", 0Dh, 0Ah, 0
szExecuteCmd db "执行命令: %s", 0Dh, 0Ah, 0
szBatComplete   db "脚本执行完成，共执行 %d 条命令", 0Dh, 0Ah, 0  ; 完成信息



    szFmtBpDelete db "%s %x",0
    szBpDeleteUsage db "Usage: bc [s/m/h] [id/address]",0Dh,0Ah,0
    szInvalidBpType db "Invalid breakpoint type, use s/m/h",0Dh,0Ah,0
    
    szSoftBpDeleted db "[+] Deleted software breakpoint ID: %d",0Dh,0Ah,0
    szSoftBpDeletedByAddr db "[+] Deleted software breakpoint at 0x%08X",0Dh,0Ah,0
    szMemBpDeleted db "[+] Deleted memory breakpoint ID: %d",0Dh,0Ah,0
    szMemBpDeletedByAddr db "[+] Deleted memory breakpoint at 0x%08X",0Dh,0Ah,0 
    szHardBpDeleted db "[+] Deleted hardware breakpoint ID: %d",0Dh,0Ah,0
    
    szBpNotFound db "[-] Breakpoint ID %d not found",0Dh,0Ah,0
    szBpAddrNotFound db "[-] No breakpoint at 0x%08X",0Dh,0Ah,0
    szInvalidHardBpId db "[-] Hardware breakpoint ID must be 0-3",0Dh,0Ah,0
    szProtectRestoreFailed db "[-] Failed to restore protection at 0x%08X",0Dh,0Ah,0
    
    g_TempBpAddr dd 0    ; 临时断点地址
g_TempBpCode db 0    ; 临时断点的原指令
g_isCmdNextHit dd 0  ; 标记是否命中了步过断点

;内存dump
g_lpStartAddress dd 0
g_lpBaseOfImage dd 0 ;在EventCreateProcess中保存的基址
g_pDosHeader dd 0
g_pNtHeader dd 0
g_pFileHeader dd 0
g_pOptionHeader dd 0
g_pSecHeader dd 0
g_numberOfSection dd 0
;iat
g_pIatStart dd 0 
g_IatSize dd 0
g_pIatEnd dd 0
szIatInfo  db "IAT信息: 起始=%08X 大小=%08X", 0Dh, 0Ah, 0
szTestApiResult db "测试地址=%08X 结果=%d API=%s", 0Dh, 0Ah, 0

szFmtDumpPE db "%s", 0    ; 参数格式
szDumpPEUsage db "用法: dump <输出文件路径>", 0Dh, 0Ah, 0
szDumpPEFileError db "DumpPEFileError 创建输出文件 %s 失败", 0Dh, 0Ah, 0
szDumpPEReadError db "umpPEReadError 读取内存地址 0x%08X 失败", 0Dh, 0Ah, 0
szDumpPENoDebug db "没有正在调试的进程，无法进行PE转储", 0Dh, 0Ah, 0
szDumpPECopyError db "复制文件到 %s 失败", 0Dh, 0Ah, 0
szDumpPEMemError db "内存分配失败", 0Dh, 0Ah, 0
szDumpPEStart db "开始转储PE文件到 %s (内存基址: 0x%08X)", 0Dh, 0Ah, 0
szDumpPEReadSecError db "读取第 %d 个节表失败", 0Dh, 0Ah, 0
szDumpPESkipSection db "跳过节 %s (无原始数据)", 0Dh, 0Ah, 0
szDumpPESetPtrError db "设置文件指针到节 %s 失败", 0Dh, 0Ah, 0
szDumpPEReadSecDataError db "读取节 %s 数据失败", 0Dh, 0Ah, 0
szDumpPEWriteError db "写入节 %s 数据失败", 0Dh, 0Ah, 0
szDumpPESectionDumped db "已转储节 %s: RVA=0x%08X Size=0x%08X Written=%d bytes", 0Dh, 0Ah, 0
szDumpPEComplete db "PE文件转储完成: %s", 0Dh, 0Ah, 0
szDumpPEProtectError db "更改节 %s 内存保护失败", 0Dh, 0Ah, 0

;DbgGetPEinfo
szPEInfoInvalidDOS db "无效的DOS头，不是PE文件", 0Dh, 0Ah, 0
szPEInfoInvalidNT db "无效的NT头，不是PE文件", 0Dh, 0Ah, 0
szPEInfoSuccess db "PE解析成功: 映像基址=%08X, DOS头=%08X, NT头=%08X, 文件头=%08X, 可选头=%08X, 节表=%08X, 节数=%d, IAT地址=%08X, IAT大小=%08X", 0ah,0dh,0

;GetApiName
szApiNameFound db "找到API名称: %s", 0Dh, 0Ah, 0
szApiNameNotFound db "未找到API名称", 0Dh, 0Ah, 0
szModuleSnapError db "CreateToolhelp32Snapshot失败，错误码=%d", 0Dh, 0Ah, 0
szEnumModulesError db "EnumProcessModules失败，错误码=%d", 0Dh, 0Ah, 0
szFmtApiName db "%s.%s", 0   ; 格式：DLL名.函数名
szFmtOrdinal db "%s.#%d", 0   ; 格式：DLL名.#序号
szApiDebug1 db "DbgGetApiName: 名称索引=%d, 名称RVA=0x%08X", 0Dh, 0Ah, 0
szApiDebug2 db "API调试: 函数名='%s'", 0Dh, 0Ah, 0
szApiEmptyName db "警告: 获取到空函数名", 0Dh, 0Ah, 0
szAddrFormat db "地址_0x%08X", 0
szApiIndirect db "尝试间接获取API: 地址0x%08X -> 0x%08X", 0Dh, 0Ah, 0
g_bApiRecursive dd TRUE;递归标志
szBracketEnd db "]", 0

;识别跳转表
NUM_JCC_INST equ 18  ; JCC指令数量
g_JCCTable dd offset szJE, offset szJNE, offset szJZ, offset szJNZ
                 dd offset szJA, offset szJAE, offset szJB, offset szJBE
                 dd offset szJG, offset szJGE, offset szJL, offset szJLE
                 dd offset szJC, offset szJNC, offset szJO, offset szJNO
                 dd offset szCall2, offset szJmp
szJE  db "je", 0
szJNE db "jne", 0
szJZ  db "jz", 0
szJNZ db "jnz", 0
szJA  db "ja", 0
szJAE db "jae", 0
szJB  db "jb", 0
szJBE db "jbe", 0
szJG  db "jg", 0
szJGE db "jge", 0
szJL  db "jl", 0
szJLE db "jle", 0
szJC  db "jc", 0
szJNC db "jnc", 0
szJO  db "jo", 0
szJNO db "jno", 0
szCall2 db "call", 0
szJmp db "jmp", 0

;reg显示API
szRegEax db "EAX=%08X", 0Dh, 0Ah, 0
szRegEbx db "EBX=%08X", 0Dh, 0Ah, 0
szRegEcx db "ECX=%08X", 0Dh, 0Ah, 0
szRegEdx db "EDX=%08X", 0Dh, 0Ah, 0
szRegEsi db "ESI=%08X", 0Dh, 0Ah, 0
szRegEdi db "EDI=%08X", 0Dh, 0Ah, 0
szRegEbp db "EBP=%08X", 0Dh, 0Ah, 0
szRegEsp db "ESP=%08X", 0Dh, 0Ah, 0
szRegEip db "EIP=%08X", 0Dh, 0Ah, 0
szRegEfl db "EFL=%08X", 0Dh, 0Ah, 0
szRegApi db "    --> %s", 0Dh, 0Ah, 0

;记录功能
g_bLogCmd DWORD FALSE  ; 命令记录标志
g_hLogFile DWORD 0     ; 日志文件句柄
szLogFileName db "log1.txt", 0
szLogEnabled db "命令记录功能已启用", 0Dh, 0Ah, 0
szLogDisabled db "命令记录功能已关闭", 0Dh, 0Ah, 0
szLogOpenError db "无法打开日志文件: %s", 0Dh, 0Ah, 0
szLogLine db "%s %s", 0Dh, 0Ah, 0  ; 命令格式：命令+参数+换行