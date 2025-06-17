; 主程序文件
.386
.model flat, stdcall
option casemap:none

include mydebug.Inc
include log.inc
include psapi.inc
includelib psapi.lib

;include BpList.inc
;include vector.inc

.code
;--------------------异常处理-------------------------
;断点异常
ExecBreakPoint proc uses ecx edx ebx
    LOCAL @Context:CONTEXT
    LOCAL @dwContinueStatus:DWORD
	LOCAL @ExceptionAddress:DWORD
	LOCAL @hitBp:DWORD
	LOCAL @index:DWORD
	LOCAL @dwTemp:DWORD
	LOCAL @sztempBuf[128]:BYTE
	LOCAL @i:DWORD
	
	LOCAL @memInfo:MEMORY_BASIC_INFORMATION
    LOCAL @pageAddr:DWORD
    LOCAL @memindex:DWORD
    LOCAL @oldProtect:DWORD
    
    mov @dwContinueStatus, DBG_EXCEPTION_NOT_HANDLED
    ;invoke LogMsg, LOG_DEBUG, offset g_szEXCEPTION_DEBUG_EVENT
    
    ; 记录日志
    mov esi, offset g_DebugEv
    assume esi:ptr DEBUG_EVENT
	mov eax, [esi].u.Exception.pExceptionRecord.ExceptionCode
	mov ecx, [esi].u.Exception.pExceptionRecord.ExceptionAddress
	mov edx, [esi].u.Exception.pExceptionRecord.ExceptionFlags
	mov ebx, [esi].u.Exception.pExceptionRecord.NumberParameters
	
		mov @ExceptionAddress, ecx
		
    invoke LogMsg, LOG_DEBUG, offset szExeceptionInfo,offset szExecBreakPoint, eax,ecx,edx,ebx

	mov eax,@ExceptionAddress
   	.if   eax ==  g_lpStartAddress
 		

    ;获取保护属性 ,每1000h保存1次

	    mov @memindex,0
	    mov @pageAddr,g_oldprotStart
	    .while @pageAddr <7FFFFFFFh && @memindex < MAX_MEMORY_REGIONS
	    	invoke VirtualQueryEx, g_hProcess, @pageAddr, addr @memInfo, sizeof MEMORY_BASIC_INFORMATION
	    	.if eax == 0
	    		;invoke crt_cprintf,xxxx
	    		add @pageAddr, 1000h
	            .continue
	    	.endif
	    	
	    			
		
	    	mov ebx,@memInfo.Protect
			mov eax,@memindex
	    	shl eax,2  ;index*4	
			mov [g_oldprotAry+eax], ebx
			add @pageAddr,1000h
			inc @memindex
	    .endw
    
		invoke DbgGetPEinfo
  	
	 .endif  	
  	 
	 mov @hitBp, 0
   	

	.if g_IsSytemBp == TRUE
        ; 系统断点处理
        mov g_IsSytemBp, FALSE
        
        ; 获取寄存器状态
        invoke DbgGetRegs, addr @Context
        
        ; 显示寄存器
        invoke DbgShowRegs, addr @Context
        
        ; 显示反汇编
        mov g_lpAddress, 0
        invoke DbgDiasm, @Context.regEip, 1

        ; 等待命令
        invoke DbgGetCmd,0
        mov @dwContinueStatus, eax
    .else
	; 遍历软件断点
	 	mov @index,0  
	    .while @index < MAX_SOFTBP_COUNT
	    lea edi, g_softBpAry
	    	mov eax,@index
	    	imul eax,sizeof SoftBpInfo
	    	add edi,eax
	        mov esi, edi
	        assume esi:ptr SoftBpInfo
	        
			mov eax, [esi].Address
	        .if eax == @ExceptionAddress
	        	.if [esi].Enable==1
	        	 ; 找到匹配的断点
		            mov @hitBp, esi
		            
		            ; 记录命中日志
		            invoke LogMsg, LOG_INFO, offset szSoftBpHit, \
		                @index, [esi].Address
		            
		            ; 跳出循环
		            .break
	        	.endif
	           
	        .endif
	        ; 获取下一个断点
	        inc @index
	    .endw    
    
 ; 检查是否命中断点
        .if @hitBp != 0
            ; 获取寄存器状态
            invoke DbgGetRegs, addr @Context
            
            ; 修正EIP (INT 3指令长度为1字节)
            dec @Context.regEip
            invoke DbgSetRegs, addr @Context
            
            ; 显示寄存器
            invoke DbgShowRegs, addr @Context
            
            ; 恢复原始代码
            mov esi, @hitBp
            assume esi:ptr SoftBpInfo
            invoke DbgWriteMemory, [esi].Address, addr [esi].OldCode, sizeof BYTE
            
			; 设置单步以便恢复断点
            mov g_IsBpStep, TRUE
            mov eax, [esi].Address  ; 
            mov g_lpBpAddress,eax
            
            invoke DbgSetStep
            
            ; 显示反汇编
            mov g_lpAddress, 0
            invoke DbgDiasm, [esi].Address, 1
            
            ; 等待命令
            invoke DbgGetCmd,0
            mov @dwContinueStatus, eax
        .endif
    .endif
	assume esi:nothing
	mov eax, @dwContinueStatus
	ret
ExecBreakPoint endp

;访问异常
; 内存访问异常处理
ExecAccess proc uses ebx ecx edx esi edi
    LOCAL @Context:CONTEXT
    LOCAL @dwContinueStatus:DWORD
    LOCAL @ExceptionInfo_Addr:DWORD
    LOCAL @ExceptionInfo_Flag:DWORD
    LOCAL @ExceptionAddress:DWORD
    LOCAL @pageBase:DWORD
    LOCAL @tmpProtect:DWORD
    LOCAL @hitBp:DWORD ;找到的断点指针
    LOCAL @i:DWORD
    LOCAL @count:DWORD
    LOCAL @samePageFlag:DWORD
	LOCAL @pageIndex:DWORD
	LOCAL @origProtect:DWORD 
    LOCAL @index:DWORD     
    LOCAL @MemPage:DWORD
    LOCAL @OldProtect:DWORD
       
    ; 默认继续处理
    mov @dwContinueStatus, DBG_EXCEPTION_NOT_HANDLED
    
    ; 获取异常信息
    mov esi, offset g_DebugEv
    assume esi:ptr DEBUG_EVENT
    mov eax, [esi].u.Exception.pExceptionRecord.ExceptionInformation[0]
    mov @ExceptionInfo_Flag, eax    ; 0=读取, 1=写入, 8=执行
    mov eax, [esi].u.Exception.pExceptionRecord.ExceptionInformation[4]
    
    mov @ExceptionInfo_Addr, eax    
	mov eax, [esi].u.Exception.pExceptionRecord.ExceptionAddress
    mov @ExceptionAddress, eax   

;	invoke LogMsg, LOG_DEBUG, offset szExceptionAccessInfo, \
;	    offset szExecAccess, \
;	    [esi].u.Exception.pExceptionRecord.ExceptionCode, \
;	    [esi].u.Exception.pExceptionRecord.ExceptionAddress, \
;	    [esi].u.Exception.pExceptionRecord.ExceptionFlags, \
;	    [esi].u.Exception.pExceptionRecord.NumberParameters, \
;	    @ExceptionInfo_Flag, \
;	    @ExceptionInfo_Addr
    assume esi:nothing
    
;    .if @ExceptionInfo_Addr< 00100000h
;		mov eax, @ExceptionAddress
;        and eax, 0FFFFF000h 
;        mov @MemPage, eax
;        int 3
;        invoke VirtualProtectEx, g_hProcess, @MemPage, 1, PAGE_READWRITE, addr @OldProtect
;		mov @dwContinueStatus, DBG_CONTINUE
;		mov eax, @dwContinueStatus
;   	 	ret	
;    .endif
    
	; 获取异常地址所在页面基址
	mov eax, @ExceptionInfo_Addr
    and eax, 0FFFFF000h
    mov @pageBase, eax
    
    ;查找原始保护属性
    sub eax, g_oldprotStart
    shr eax, 12  ; 除以4096(1000h)
    mov @pageIndex, eax
    .if eax < MAX_MEMORY_REGIONS
    	shl eax,2
        mov eax, [g_oldprotAry + eax]
        
        mov @origProtect, eax
    .endif
    
    ; 初始化命中标志
    mov @hitBp, 0
	mov @count, 0
  
   ;选数组空位存断点信息
    mov @index,0  

    .while @index < MAX_MEMBP_COUNT
    	lea edi, g_memBpAry
    	mov eax,@index
    	imul eax,sizeof MemBpInfo
    	add edi,eax
        mov esi, edi
        assume esi:ptr MemBpInfo
        
               ; 检查是否在同一页面
        mov eax, [esi].StartAddr
        and eax, 0FFFFF000h
        .if eax == @pageBase
            ; 在同一页面有断点，还原内存保护属性  


            invoke DbgMemoryProtected, [esi].StartAddr, [esi].dwSize, @origProtect
            
            ;只要有在同一页面的 都要设单步重新保护
        	mov g_IsMemStep, TRUE

        	invoke DbgSetStep
        	
            ; 标记为临时禁用
            mov [esi].TempDisabled, 1
            
            ; 记录日志
          ;  invoke crt__cprintf, offset szBpTempDisabled,[esi].BpId, [esi].StartAddr, [esi].EndAddr
            
            ; 检查是否命中断点范围
            mov eax, @ExceptionInfo_Addr
            .if eax >= [esi].StartAddr && eax <= [esi].EndAddr
                ; 检查访问类型
                mov eax, [esi].dwType
                mov ecx, @ExceptionInfo_Flag
                
                .if (al == 'r' && ecx == 0) || \
                    (al == 'w' && ecx == 1) || \
                    (al == 'e' && ecx == 8) || \
                    (al == 'a' && (ecx == 0 || ecx == 1))
                    
                    ; 找到匹配的断点
		            ; 获取寄存器状态
		
		            invoke DbgGetRegs, addr @Context
		            invoke DbgShowRegs, addr @Context
		            
		            ; 显示反汇编
		            mov g_lpAddress, 0
		
		            invoke DbgDiasm, @ExceptionAddress, 1
		            
		            ; 等待命令
		            invoke DbgGetCmd,0
		            mov @dwContinueStatus, eax
                    ; 记录命中日志
                    invoke crt__cprintf, offset szMemBpHit, \
                        @index, [esi].StartAddr, [esi].EndAddr, al, @ExceptionInfo_Addr
                        
                    .break
                .endif
            .endif
            assume esi:nothing
            	mov @dwContinueStatus, DBG_CONTINUE
    			mov eax, @dwContinueStatus
    			ret
		 .endif
        ; 获取下一个断点
        inc @index
    .endw    
    
        
  	;遍历完了 不是我们设置的断点导致的异常
  	
  		mov ecx, @ExceptionInfo_Addr
        and ecx, 0FFFFF000h
        invoke VirtualProtectEx, g_hProcess, ecx, 4, PAGE_READWRITE, addr @OldProtect
    	.if eax==0
;    		int 3 
    		mov eax,eax
    	.endif
    assume esi:nothing
    
	mov @dwContinueStatus, DBG_CONTINUE
    mov eax, @dwContinueStatus
    ret
ExecAccess endp

;DbgSetStep->TF->单步异常
ExecStep proc uses ecx edx ebx esi
	LOCAL @Context:CONTEXT
    LOCAL @dwContinueStatus:DWORD
	LOCAL @count:DWORD
	LOCAL @hitAddr:DWORD
    LOCAL @ExceptionAddress:DWORD
    LOCAL @RetBuffer[16]:BYTE
    LOCAL @index:DWORD
    LOCAL @cmdnextindex:DWORD
    mov @dwContinueStatus, DBG_EXCEPTION_NOT_HANDLED
    
	; 记录日志
    mov esi, offset g_DebugEv
    assume esi:ptr DEBUG_EVENT
	mov eax, [esi].u.Exception.pExceptionRecord.ExceptionCode
	mov ecx, [esi].u.Exception.pExceptionRecord.ExceptionAddress
	mov @ExceptionAddress,ecx
	mov edx, [esi].u.Exception.pExceptionRecord.ExceptionFlags
	mov ebx, [esi].u.Exception.pExceptionRecord.NumberParameters

    ;invoke LogMsg, LOG_DEBUG, offset szExeceptionInfo,offset szExecStep, eax,ecx,edx,ebx

    ; 获取寄存器状态
    invoke DbgGetRegs, addr @Context

    	
	.if g_isTrace !=0
		invoke DbgRecard
		invoke DbgSetStep
        mov @dwContinueStatus, DBG_CONTINUE
        ret
	.endif
	

	.if g_isFindRet != 0
	    ; 读取当前指令
	    invoke DbgReadMemory, @Context.regEip, addr @RetBuffer, 1
	    
	    ; 检查是否是RET指令
	    mov al, byte ptr @RetBuffer
	    .if al == 0C3h || al == 0CBh || al == 0C2h || al == 0CAh
	        ; 找到RET，停止查找
	        mov g_isFindRet, 0
	        
	        ; 显示信息和寄存器
	
	        invoke DbgShowRegs, addr @Context
	        invoke crt_printf, addr szFoundReturn,@Context.regEip
	         mov g_lpAddress, 0
	        invoke DbgDiasm, @Context.regEip, 1
	    
	        ; 等待命令而不是继续单步
	        invoke DbgGetCmd,0
	        mov @dwContinueStatus, eax
	    .else
	        ; 继续单步但避免重复触发
	        mov eax, @Context.regFlag
	        and eax, 0FFFFFEFFh  ; 清除TF位(0x100)
	        mov @Context.regFlag, eax
	        invoke DbgSetRegs, addr @Context
	        invoke DbgSetStep
	        mov @dwContinueStatus, DBG_CONTINUE
	    .endif
	    
	    ; 确保函数返回正确的继续状态
	    mov eax, @dwContinueStatus
	    ret
	.endif

	.if g_IsMemStep != 0
	    mov g_IsMemStep,0
        ; 遍历断点链表，恢复所有临时禁用的断点
            mov @index,0  
	    
	    .while @index < MAX_MEMBP_COUNT
	    lea edi, g_memBpAry
	    	mov eax,@index
	    	imul eax,sizeof MemBpInfo
	    	add edi,eax
	        mov ebx, edi
	        assume ebx:ptr MemBpInfo
	        
	 			; 检查是否被临时禁用
	            .if [ebx].TempDisabled == 1
	                ; 增加计数
	                inc @count
	                
	            
	                ; 恢复这个断点范围的保护属性
					invoke DbgMemoryProtected, [ebx].StartAddr, [ebx].dwSize, PAGE_NOACCESS
	                
	                ; 清除临时禁用标志
	                mov [ebx].TempDisabled, 0
	                
	                ; 记录恢复日志
	               ; invoke crt__cprintf, offset szBpRestored, [esi].BpId, [esi].StartAddr, [esi].EndAddr
	            .endif
	          assume ebx:nothing  
	        ; 获取下一个断点
	        inc @index
	    .endw
	        
		mov @dwContinueStatus, DBG_CONTINUE
	    mov eax, @dwContinueStatus
	    ret
	.endif
	
    
    ; 断步配合：恢复禁用的硬件断点
    .if g_IsHardStep != 0
 
        mov g_IsHardStep, 0
        
        ; 根据保存的索引恢复断点
        mov eax, g_IsHardIndex
        .if eax == 0
            ; 启用DR0
            or @Context.iDr7, 1
        .elseif eax == 1
            ; 启用DR1
            or @Context.iDr7, 4
        .elseif eax == 2
            ; 启用DR2
            or @Context.iDr7, 16
        .elseif eax == 3
            ; 启用DR3
            or @Context.iDr7, 64
        .endif
        
        ; 更新寄存器
        invoke DbgSetRegs, addr @Context
        
        mov @dwContinueStatus, DBG_CONTINUE
		 mov eax, @dwContinueStatus
		ret
    .endif
    
    
    mov ebx, 0
    .while ebx < 4
        ; 检查此索引的断点是否启用
        mov eax, ebx
        imul eax, sizeof HardBpInfo
        lea edi, g_HardBpArray
        add edi, eax
		
;		assume edi:ptr HardBpInfo
;		invoke crt_printf, offset szHardBpContent, ebx, [edi].Enable,  [edi].Address, [edi].dwType, [edi].dwSize
;		assume edi:nothing
;	                      
                          
        .if [edi].HardBpInfo.Enable == 1
            ; 检查类型是否为执行断点，地址是否匹配
            mov eax, [edi].HardBpInfo.dwType
            .if eax == 0  ; 执行断点
				mov ecx,@ExceptionAddress
		        .if ecx == [edi].HardBpInfo.Address
		            ; 找到匹配的断点，暂时禁用它
		            mov eax, ebx
		            .if eax == 0
		                ; 禁用DR0
		                and @Context.iDr7, 0FFFFFFFEh
		            .elseif eax == 1
		                ; 禁用DR1
		                and @Context.iDr7, 0FFFFFFFBh
		            .elseif eax == 2
		                ; 禁用DR2
		                and @Context.iDr7, 0FFFFFFEFh
		            .elseif eax == 3
		                ; 禁用DR3
		                and @Context.iDr7, 0FFFFFFBFh
		            .endif
		            
		            ; 设置断步配合标志
		            mov g_IsHardStep, 1
					 mov g_IsHardIndex, ebx
					 
		            ; 更新寄存器
		     		invoke DbgSetRegs, addr @Context
		             ; 设置单步执行
					invoke DbgSetStep
		            
		            ; 显示寄存器状态
		            invoke DbgShowRegs, addr @Context
       				invoke DbgGetCmd,0
		            mov @dwContinueStatus, DBG_CONTINUE
		            mov eax, @dwContinueStatus
					ret 
		        .endif
            .endif
        .endif
        
        inc ebx
    .endw
    
    
  ; 检查DR6的B0-B3位（位0-3）判断是否有硬件断点触发
	mov eax, @Context.iDr6
	and eax, 0Fh ;提取B0-B3位
	.if eax != 0
	    ; 确定哪个断点触发并禁用它
	    mov ecx, eax  ; 保存B0-B3位到ecx
	    
	    ; 检查B0位 (DR0断点)
	    test ecx, 1
	    jz not_bp0
	    ; DR0触发
	    and @Context.iDr7, 0FFFFFFFEh  ; 禁用DR0 (清除L0位)
	    mov g_IsHardStep, 1
	    mov g_IsHardIndex, 0
	    jmp bp_disable_done
not_bp0:
    
    ; 检查B1位 (DR1断点)
    test ecx, 2
    jz not_bp1
    ; DR1触发
    and @Context.iDr7, 0FFFFFFFBh  ; 禁用DR1 (清除L1位)
    mov g_IsHardStep, 1
    mov g_IsHardIndex, 1
    jmp bp_disable_done
not_bp1:
    
    ; 检查B2位 (DR2断点)
    test ecx, 4
    jz not_bp2
    ; DR2触发
    and @Context.iDr7, 0FFFFFFEFh  ; 禁用DR2 (清除L2位)
    mov g_IsHardStep, 1
    mov g_IsHardIndex, 2
    jmp bp_disable_done
not_bp2:
    
    ; 检查B3位 (DR3断点)
    test ecx, 8
    jz bp_disable_done
    ; DR3触发
    and @Context.iDr7, 0FFFFFFBFh  ; 禁用DR3 (清除L3位)
    mov g_IsHardStep, 1
    mov g_IsHardIndex, 3
bp_disable_done:

    ; 清除DR6状态位
    mov @Context.iDr6, 0
     or @Context.regFlag, 100h
    invoke DbgSetRegs, addr @Context
    
    ; 显示断点触发信息
    
    invoke crt_printf, offset szHardBpTriggered, @Context.iDr6
    
    ; 显示寄存器
    invoke DbgShowRegs, addr @Context
    
    ; 显示反汇编
    mov g_lpAddress, 0
    invoke DbgDiasm, @Context.regEip, 1
    
    ;没考虑这里设置标志和单步?
    mov g_IsHardStep, 1
    
    ; 等待命令
    invoke DbgGetCmd,0
    mov @dwContinueStatus, eax
    

.endif  

    ; 普通单步
    .if g_IsStep != 0
    
    
        mov g_IsStep, 0
        
        invoke DbgGetRegs, addr @Context
        invoke DbgShowRegs, addr @Context
        mov g_lpAddress, 0
        invoke DbgDiasm, @Context.regEip, 1
        
        invoke DbgGetCmd,0
        

        mov @dwContinueStatus, eax
    .endif
    
    	
	
    
    
    ; 断步
    .if g_IsBpStep != 0

  		mov g_IsBpStep , 0
  		
  		mov eax,g_lpBpAddress
 		.if eax!=0
 	
 			mov ebx,0cch
			invoke DbgWriteMemory, eax,  ebx, sizeof BYTE
			
			
		mov g_lpBpAddress, 0
 		.endif
 		
;  		mov @cmdnextindex, 0  ; 数组索引
;        .while @cmdnextindex < MAX_SOFTBP_COUNT
;            ; 计算当前元素地址
;            mov eax, @cmdnextindex
;            imul eax, sizeof SoftBpInfo
;            lea edi, g_softBpAry
;            add edi, eax
;            
;            ; 检查地址是否匹配
;            assume edi:ptr SoftBpInfo
;            mov eax, [edi].SoftBpInfo.Enable
;            .if eax == 1
;                ; 找到匹配的断点，将启用标志设为0表示删除
;                mov [edi].Enable, 0
;               mov[edi].Address,0
;               mov[edi].OldCode,0
;                ; 可选：输出调试信息
;              
;                invoke crt_printf, addr szBpRemoved, g_lpBpAddress
;                
;                ; 找到并删除后退出循环
;                .break
;            .endif
;            
;            inc @cmdnextindex
;        .endw
  			;找到g_lpBpAddress的断点把断点从数组中删除

        mov @dwContinueStatus, DBG_CONTINUE
    .endif
    

    
    assume esi:nothing
endstep:
    mov eax, @dwContinueStatus
    ret
ExecStep endp

ExecCmd proc szCmd:DWORD, szArgs:DWORD
    LOCAL @result:DWORD
    
    ; 查找命令
    xor ebx, ebx
    mov esi, offset g_Commands
    
    .while ebx < [g_nCommands]
        invoke crt_strcmp, [esi].Command.szCmd, szCmd
        .if eax == 0
            ; 找到命令，执行
            push szArgs
            call [esi].Command.pfnCmd
            mov @result, eax
            .break
        .endif
        
        ; 下一个命令
        add esi, sizeof Command
        inc ebx
    .endw
    
    ; 命令未找到
    .if ebx >= [g_nCommands]
        invoke crt_printf, offset g_szCmdUnknown
        mov @result, E_ERR
    .endif
    
    mov eax, @result
    ret
ExecCmd endp

;--------------------命令处理-------------------------
;g / g xxx
CmdRun proc szArgs:DWORD
    LOCAL @lpAddress:DWORD

    mov @lpAddress,0
    
;;   	;有参数
;;    .if szArgs!=NULL
;;	    invoke crt_sscanf, szArgs, offset szFmtHex, addr @lpAddress
;;    	.if @lpAddress !=0
;;    		;invoke DbgSetBpSoft,@lpAddress
;;
;;			mov eax,DBG_CONTINUE
;;    		ret    		
;;    	.endif
;;    .endif

	;没有参数 继续运行
	mov eax,DBG_CONTINUE
	ret

CmdRun endp

;t 步入
CmdSetSingleStep proc szArgs:DWORD
    mov g_IsStep, TRUE
    invoke DbgSetStep
    
    
	ret

CmdSetSingleStep endp

;p 步过 

CmdNext proc szArgs:DWORD
    LOCAL @Context:CONTEXT
    LOCAL @buffer[16]:BYTE
    LOCAL @szAsm[256]:BYTE
    LOCAL @nCodeSize:DWORD
    LOCAL @nextAddr:DWORD
   
    ;获取eip
    invoke DbgGetRegs, addr @Context
	invoke DbgReadMemory, @Context.regEip, addr @buffer, sizeof @buffer
    .if eax <= 0
    	mov eax,E_ERR
    	ret
    .endif
    
    ;反汇编 转大写 查找字符串call	TODO call到哪里？？
	invoke Decode2Asm, addr @buffer, addr @szAsm, addr @nCodeSize, @Context.regEip
	invoke crt__strupr,addr @szAsm
	invoke crt_strstr,addr @szAsm,addr szCall
	.if eax!=0
		;call指令 计算下一条指令地址
		mov eax,@Context.regEip
		add eax,@nCodeSize
		mov g_TempBpAddr, eax
		
		mov @nextAddr, eax
		invoke DbgSetBpSoft,@nextAddr
        
;	invoke DbgSetBpSoft, @nextAddr
;	;这里会设断点并添加进列表 是不合适的 我们确实该在断点处理还原代码 同
	.else
		;不是call 普通单步
		mov g_IsStep, 1
		invoke DbgSetStep
	.endif
	
	mov eax,DBG_CONTINUE
	ret
CmdNext endp

;执行到返回

szStartGetAPIName db 'DbgGetApiName ',0
szApiSuccess db "API Find :Address=%08X name=%s", 0Dh, 0Ah, 0
szApiNotFound db "API Not Find :Address=%08X", 0Dh, 0Ah, 0
szFoundInIAT db "发现地址在IAT范围内: 0x%08X", 0Dh, 0Ah, 0
szIATContent db "IAT[0x%08X] ===> 0x%08X", 0Dh, 0Ah, 0
;szExportErr db "  ExportErr", 0Dh, 0Ah, 0
szParseErr db "Invalid PE", 0Dh, 0Ah, 0
;szNoModule db "@bFound !=1",0Dh , 0Ah, 0
DbgGetApiName proc uses ebx ecx edx lpAddress:DWORD, lpBuffer:DWORD
	LOCAL @hModuleSnap:DWORD
    LOCAL @me32:MODULEENTRY32
    LOCAL @bFound:DWORD 
    LOCAL @dosHeader:IMAGE_DOS_HEADER
    LOCAL @ntHeaders:IMAGE_NT_HEADERS
    LOCAL @exportDir:IMAGE_EXPORT_DIRECTORY
    LOCAL @i:DWORD
    LOCAL @dwIndex:DWORD
    LOCAL @dwTemp:DWORD
    LOCAL @dwNameIndex:DWORD
	LOCAL @szFuncNameBuf[256]:BYTE
	LOCAL @szFuncName[128]:BYTE
	LOCAL @funcNameAddr:DWORD
;    LOCAL @hModules[100]:DWORD  
;    LOCAL @cbNeeded:DWORD  

	;invoke crt__cprintf,offset szStartGetAPIName,g_bApiRecursive
	
    ; 检查地址是否在IAT范围内
    mov eax, lpAddress
    mov ecx, g_pIatStart
    mov edx, g_pIatEnd
    

        ; 然后判断地址是否在IAT范围内
        .if eax >= ecx 
        	.if eax <= edx
        		
				; 是IAT中的地址，读取内容获取实际API地址
	            ;invoke LogMsg, LOG_DEBUG, addr szFoundInIAT, eax
	            
	            ; 读取IAT项内容(实际API地址)
	            invoke DbgReadMemory, lpAddress, addr @dwTemp, 4
	            .if eax != -1
	                ; 用读取到的实际地址递归调用
	                ;invoke LogMsg, LOG_DEBUG, addr szIATContent, lpAddress, @dwTemp
	                invoke DbgGetApiName, @dwTemp, lpBuffer
	                ret
	            .endif
            
        	.endif

        .endif

    
    
  

    
	;模块快照
	invoke CreateToolhelp32Snapshot,TH32CS_SNAPMODULE,g_pid
	mov @hModuleSnap,eax
	.if eax == INVALID_HANDLE_VALUE
	    invoke GetLastError
	    invoke LogMsg, LOG_ERROR, addr szModuleSnapError, eax
	    mov eax, E_ERR
	    ret
	.endif
	
	mov @me32.dwSize,sizeof MODULEENTRY32
	mov @bFound,0
	
	invoke Module32First,@hModuleSnap,addr @me32
	.while eax != 0
		;地址是否在模块内
		mov eax,lpAddress
		mov ecx,@me32.modBaseAddr
		.if eax >= ecx
			add ecx,@me32.modBaseSize
			.if eax <= ecx
				mov @bFound,1
				.break
			.endif
		.endif
		
		invoke Module32Next, @hModuleSnap, addr @me32
	.endw
	
	; 如果未找到模块，返回失败
    .if @bFound !=1
   	;	invoke crt__cprintf,offset szNoModule
        invoke CloseHandle, @hModuleSnap
        mov eax, E_ERR
        ret
    .endif
	
	; 解析模块PE头获取导出表
	invoke DbgReadMemory, @me32.modBaseAddr, addr @dosHeader, sizeof IMAGE_DOS_HEADER
    .if eax == 0
    	invoke crt__cprintf,offset szParseErr
        invoke CloseHandle, @hModuleSnap
        mov eax, E_ERR
        ret
    .endif
	
	; 检查DOS头签名
    mov ax, @dosHeader.e_magic
    .if ax != 5A4Dh  ; 'MZ'
    	invoke crt__cprintf,offset szParseErr
        invoke CloseHandle, @hModuleSnap
        mov eax, E_ERR
        ret
    .endif
    
    ; 获取NT头
    mov ecx, @me32.modBaseAddr
    add ecx, @dosHeader.e_lfanew
    
    ; 读取NT头
    invoke DbgReadMemory, ecx, addr @ntHeaders, sizeof IMAGE_NT_HEADERS
    .if eax == 0
    	invoke crt__cprintf,offset szParseErr
        invoke CloseHandle, @hModuleSnap
        mov eax, E_ERR
        ret
    .endif
    
    ; 检查NT头签名
    mov eax, @ntHeaders.Signature
    .if eax != 00004550h  ; 'PE\0\0'
    	invoke crt__cprintf,offset szParseErr
        invoke CloseHandle, @hModuleSnap
        mov eax, E_ERR
        ret
    .endif
    
    ; 获取导出表信息
    mov eax, @ntHeaders.OptionalHeader.DataDirectory[0].VirtualAddress
    .if eax == 0
    ;	invoke crt__cprintf,offset szExportErr
        invoke CloseHandle, @hModuleSnap
        mov eax, E_ERR
        ret
    .endif

    ; 计算导出表地址
    mov ebx, @me32.modBaseAddr
    add ebx, eax
    
    
    ; 读取导出表
    invoke DbgReadMemory, ebx, addr @exportDir, sizeof IMAGE_EXPORT_DIRECTORY
    .if eax == 0
    ;	invoke crt__cprintf,offset szExportErr
        invoke CloseHandle, @hModuleSnap
        mov eax, E_ERR
        ret
    .endif
    
     ; 查找匹配的导出函数地址
    mov @dwIndex, -1
    mov @i, 0
    mov ebx,@exportDir.NumberOfFunctions
    .while @i < ebx
        ; 读取函数地址
        mov eax, @me32.modBaseAddr
        add eax, @exportDir.AddressOfFunctions
        mov ecx, @i
        imul ecx, 4  ; 每个函数地址是4字节
        add eax, ecx
        mov edx,eax
        
        invoke DbgReadMemory, edx, addr @dwTemp, 4
        .if eax != 0
            mov ecx, @dwTemp
            add ecx, @me32.modBaseAddr  ; 计算实际函数地址
            
            ; 检查是否匹配
            .if ecx == lpAddress
                mov eax, @i
                mov @dwIndex, eax
                .break
            .endif
        .endif
        
        inc @i
    .endw
    
    
     ; 未找到，返回失败 
    .if @dwIndex == -1

    	
    	;invoke crt__cprintf, addr szApiNotFound, lpAddress
        invoke CloseHandle, @hModuleSnap
        mov eax, E_ERR
        ret
    .endif
    

    
    ; 从序号表查找名称索引
    mov @dwNameIndex,-1
    mov @i,0
    mov ebx,@exportDir.NumberOfNames
    .while @i<ebx
    	mov eax,@me32.modBaseAddr
    	add eax,@exportDir.AddressOfNameOrdinals
    	mov ecx,@i;@i是序号表的当前下标
    	imul ecx,2
    	add eax,ecx
    	mov edx,eax;edx中是序号表当前项的VA
    	
    	invoke DbgReadMemory, edx, addr @dwTemp, 2
    	.if eax!=0
    		movzx ecx, word ptr @dwTemp  ; @dwTemp 是 序号表当前项的值  对应 地址表的下表  对应 名称表的下表
    		
			; 检查是否匹配之前找到的函数索引
	        .if ecx == @dwIndex
	            mov eax, @i
	            mov @dwNameIndex, eax
	            .break
	        .endif
    	.endif
    	
    	inc @i
    .endw
    
    ;如果没找到名称，使用序号
	.if @dwNameIndex == -1
	
        ; 使用模块名+序号
        mov eax, @dwIndex
        add eax, @exportDir.nBase  ; 序号=下标+base
        invoke crt_sprintf, lpBuffer, addr szFmtOrdinal, 
                           addr @me32.szModule, eax
                           
        invoke crt__cprintf,lpBuffer 
        
        invoke CloseHandle, @hModuleSnap
        mov eax, E_OK
        ret
    .endif    
    
    ; 找到，读取函数名称


    mov eax, @me32.modBaseAddr
    add eax, @exportDir.AddressOfNames
    mov ecx, @dwNameIndex
    imul ecx, 4  ; 
    add eax, ecx	
    mov edx, eax;edx保存的是名称表对应下标的地址   还需要读下标地址内的数据（RVA）转成VA后就是字符串的地址 以0结尾	
    
    ; 获得下标地址内的数据（RVA）
    invoke DbgReadMemory, edx, addr @dwTemp, 4
    .if eax == FALSE
        invoke CloseHandle, @hModuleSnap
        mov eax, E_ERR
        ret
    .endif
    
    ;转化成VA
	mov eax, @me32.modBaseAddr
	add eax, @dwTemp  ; eax现在是函数名字符串的VA
	mov ebx, eax      ; 保存VA到ebx
    
    
    ;读字符串
    invoke DbgReadMemory, ebx, addr @szFuncNameBuf, 255
    invoke crt_strcpy,addr @szFuncName,addr @szFuncNameBuf
    
    
invoke crt_sprintf, lpBuffer, addr szFmtApiName, addr @me32.szModule, addr @szFuncName
;invoke crt__cprintf, addr szApiSuccess, lpAddress, lpBuffer

	                      
	invoke CloseHandle, @hModuleSnap
    mov eax, E_OK
    ret                  
	
DbgGetApiName endp

DbgGoRet proc 
    LOCAL @Context:CONTEXT
    LOCAL @buffer[16]:BYTE
    LOCAL @szAsm[256]:BYTE
    LOCAL @nCodeSize:DWORD
    LOCAL @nextAddr:DWORD
  
    ;获取eip
    invoke DbgGetRegs, addr @Context
	invoke DbgReadMemory, @Context.regEip, addr @buffer, sizeof @buffer
    .if eax <= 0
    	mov eax,E_ERR
    	ret
    .endif
    
    ;反汇编 转大写 查找字符串ret
	invoke Decode2Asm, addr @buffer, addr @szAsm, addr @nCodeSize, @Context.regEip
	invoke crt__strupr,addr @szAsm
	invoke crt_strstr,addr @szAsm,addr szRet
	.if eax!=0
			 mov g_isFindRet, 0
			  invoke crt_printf, addr szFoundReturn, @Context.regEip
			  
			invoke DbgShowRegs, addr @Context
       		invoke DbgDiasm, @Context.regEip, 1 
        	invoke DbgGetCmd,0
        	ret
        .elseif
       		; 不是RET，继续单步
        	invoke DbgSetStep
    	.endif
	
	mov eax,DBG_CONTINUE
	ret
DbgGoRet endp

CmdGoRet proc szArgs:DWORD

    mov g_isFindRet, 1
    invoke DbgSetStep
    
    mov eax, DBG_CONTINUE
    ret
CmdGoRet endp
 
;查看、修改寄存器管理功能  
CmdSetReg proc
	
	
	ret

CmdSetReg endp

;查看、修改内存功能
DumpSize equ  80h  
CmdDumpMem proc szArgs:DWORD
    LOCAL @lpAddress:DWORD
    LOCAL @buffer[DumpSize]:BYTE
    LOCAL @i:DWORD

    mov @lpAddress, 0
    
    .if szArgs != NULL
        invoke crt_sscanf, szArgs, addr szFmtHex, addr @lpAddress
    .endif
    
    ; 使用上次地址
    .if @lpAddress == 0
        mov eax, g_lpLastDumpAddr
        mov @lpAddress, eax
    .endif
    
    ; 读取内存
    invoke DbgReadMemory, @lpAddress, addr @buffer, DumpSize
    .if eax <= 0
        invoke crt_printf, addr szReadMemError, @lpAddress
        mov eax, E_ERR
        ret
    .endif
    
    ; 保存下次地址
    mov eax, @lpAddress
    add eax, DumpSize
    mov g_lpLastDumpAddr, eax
    
    mov @i, 0
    .while @i < DumpSize
        ; 每行开始显示地址
        mov eax, @i
        xor edx, edx
        mov ecx, 16
        div ecx
        .if edx == 0
            mov eax, @lpAddress
            add eax, @i
            push @i
            invoke crt_printf, addr szMemDumpAddr, eax
            pop @i
        .endif
        
        ; 显示字节
        mov edx, @i
        movzx eax, byte ptr @buffer[edx]
        push @i
        invoke crt_printf, addr szMemDumpByte, eax
        pop @i
        
        ; 每行结束换行
        inc @i
        mov eax, @i
        xor edx, edx
        mov ecx, 16
        div ecx
        .if edx == 0
            push @i
            invoke crt_printf, addr szCRLF
            pop @i
        .endif
    .endw
    
    mov eax, E_OK
    ret
    
CmdDumpMem endp
szFmtEditMem db "%x %x", 0
szEditUsage db "用法: e 地址 字节值1 [字节值2] ...", 0Dh, 0Ah, 0
szSpace db " ", 0
szByteValueError db "错误: 值 0x%08X 超过一个字节(0xFF)", 0Dh, 0Ah, 0
szNoValues db "错误: 未指定要写入的值", 0Dh, 0Ah, 0
szWriteMemError db "错误: 写入地址 0x%08X 失败", 0Dh, 0Ah, 0
szWriteSuccess db "成功写入 0x%08X 到地址 0x%08X", 0Dh, 0Ah, 0
;修改内存 - e [地址] [字节值]
CmdEditMem proc szArgs:DWORD
    LOCAL @lpAddress:DWORD
    LOCAL @value:DWORD
    
    .if szArgs == NULL
        mov eax, E_ERR
        ret
    .endif
    
    invoke crt_sscanf, szArgs, addr szFmtEditMem, addr @lpAddress, addr @value
    .if eax < 2
        invoke crt_printf, addr szEditUsage
        mov eax, E_ERR
        ret
    .endif
    
    ; 写入内存
    invoke DbgWriteMemory, @lpAddress, addr @value, 4
    .if eax <= 0
        invoke crt_printf, addr szWriteMemError, @lpAddress
        mov eax, E_ERR
    .else
        invoke crt_printf, addr szWriteSuccess, @value, @lpAddress
        mov eax, E_OK
    .endif
    
    ret
CmdEditMem endp

; 寄存器管理功能
CmdShowRegisters proc uses eax ecx edx szArgs:DWORD
    LOCAL @Context:CONTEXT
    LOCAL @regName[16]:BYTE
    LOCAL @regValue:DWORD
	
    invoke DbgGetRegs, addr @Context
  	
    invoke crt_sscanf, szArgs, addr szRegSetFmt, addr @regName, addr @regValue
    .if eax == 2
        ; 解析到两个参数，要修改寄存器
        invoke crt__strupr, addr @regName
        
        invoke crt_strcmp, addr @regName, addr szRegEAX
        .if eax == 0
            mov eax, @regValue
            mov @Context.regEax, eax
            jmp FindDone
        .endif
        
        invoke crt_strcmp, addr @regName, addr szRegEBX
        .if eax == 0
            mov eax, @regValue
            mov @Context.regEbx, eax
            jmp FindDone
        .endif
        
        invoke crt_strcmp, addr @regName, addr szRegECX
        .if eax == 0
            mov eax, @regValue
            mov @Context.regEcx, eax
            jmp FindDone
        .endif
        
        invoke crt_strcmp, addr @regName, addr szRegEDX
        .if eax == 0
            mov eax, @regValue
            mov @Context.regEdx, eax
            jmp FindDone
        .endif
        
        invoke crt_strcmp, addr @regName, addr szRegESI
        .if eax == 0
            mov eax, @regValue
            mov @Context.regEsi, eax
            jmp FindDone
        .endif
        
        invoke crt_strcmp, addr @regName, addr szRegEDI
        .if eax == 0
            mov eax, @regValue
            mov @Context.regEdi, eax
            jmp FindDone
        .endif
        
        invoke crt_strcmp, addr @regName, addr szRegEBP
        .if eax == 0
            mov eax, @regValue
            mov @Context.regEbp, eax
            jmp FindDone
        .endif
        
        invoke crt_strcmp, addr @regName, addr szRegESP
        .if eax == 0
            mov eax, @regValue
            mov @Context.regEsp, eax
            jmp FindDone
        .endif
        
        invoke crt_strcmp, addr @regName, addr szRegEIP
        .if eax == 0
            mov eax, @regValue
            mov @Context.regEip, eax
            jmp FindDone
        .endif
        
        invoke crt_strcmp, addr @regName, addr szRegEFL
        .if eax == 0
            mov eax, @regValue
            mov @Context.regFlag, eax
            jmp FindDone
        .endif
        
        ; 调试寄存器
        invoke crt_strcmp, addr @regName, addr szRegDR0
        .if eax == 0
            mov eax, @regValue
            mov @Context.iDr0, eax
            jmp FindDone
        .endif
        
        invoke crt_strcmp, addr @regName, addr szRegDR1
        .if eax == 0
            mov eax, @regValue
            mov @Context.iDr1, eax
            jmp FindDone
        .endif
        
        invoke crt_strcmp, addr @regName, addr szRegDR2
        .if eax == 0
            mov eax, @regValue
            mov @Context.iDr2, eax
            jmp FindDone
        .endif
        
        invoke crt_strcmp, addr @regName, addr szRegDR3
        .if eax == 0
            mov eax, @regValue
            mov @Context.iDr3, eax
            jmp FindDone
        .endif
        
        invoke crt_strcmp, addr @regName, addr szRegDR6
        .if eax == 0
            mov eax, @regValue
            mov @Context.iDr6, eax
            jmp FindDone
        .endif
        
        invoke crt_strcmp, addr @regName, addr szRegDR7
        .if eax == 0
            mov eax, @regValue
            mov @Context.iDr7, eax
            jmp FindDone
        .endif
        
        ; 未知寄存器
        invoke crt_printf, offset szUnknownReg, addr @regName
    .endif
    
    	;没有参数
        invoke DbgShowRegs,addr @Context
        mov eax,E_OK
        ret     

FindDone:
        ; 更新寄存器
        invoke DbgSetRegs, addr @Context
        .if eax < 0
            invoke crt_printf, offset szRegSetError
            mov eax, E_ERR
            ret
        .endif
        
		mov eax, E_OK
        ret
CmdShowRegisters endp

;记录输入的命令到文件 输入log执行
CmdLogCmd proc szArgs:DWORD
    LOCAL @written:DWORD

     invoke CreateFile, offset szLogFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
            .if eax == INVALID_HANDLE_VALUE
                invoke crt_printf, offset szLogOpenError, offset szLogFileName
                mov eax, E_ERR
                ret
            .endif
            
	mov g_hLogFile, eax      
    mov g_bLogCmd ,TRUE        
	invoke crt_printf, offset szLogEnabled
	
	mov eax,E_OK
	ret
CmdLogCmd endp

; 读脚本自动执行
CmdBat proc szArgs:DWORD
    LOCAL @szLine[1024]:BYTE
    LOCAL @szCmd[256]:BYTE
    LOCAL @szArgs[512]:BYTE
    LOCAL @cmdResult:DWORD
    LOCAL @linePtr:DWORD
	LOCAL @hFile:DWORD
	LOCAL @szFullCmd[256]:BYTE

    ; 检查是否继续执行已加载的脚本
    .if g_bScriptRunning == 1
        jmp execute_commands
    .endif
    
    ; 新脚本，检查参数
    .if szArgs == NULL
        invoke crt_printf, offset szBatUsage
        mov eax, E_ERR
        ret
    .endif
    
    ; 保存脚本文件名
    invoke crt_strcpy, offset g_szScriptFile, szArgs
    
    ; 打开脚本文件
    invoke crt_fopen, szArgs, offset szReadMode
    .if eax == NULL
        invoke crt_printf, offset szScriptError, szArgs
        mov eax, E_ERR
        ret
    .endif
    mov @hFile, eax
    
    ; 初始化变量
    mov g_nScriptLines, 0
    mov g_nCurrentLine, 0
    mov g_nScriptCmdCount, 0
    
    ; 读取所有行到数组
    .while g_nScriptLines < MAX_SCRIPT_LINES
        ; 计算当前行的存储位置
        mov eax, g_nScriptLines
        mov edx, 256  ; 每行最大长度
        mul edx
        lea edi, ScriptLines
        add edi, eax
        mov @linePtr, edi
        
        ; 清空行缓冲区
        invoke RtlZeroMemory, addr @szLine, sizeof @szLine
        
        ; 读取一行
        invoke crt_fgets, addr @szLine, sizeof @szLine, @hFile
        .if eax == NULL
            ; 文件结束
            .break
        .endif
        
;        ; 查找命令结束的 #
;        invoke crt_strchr, addr @szLine, '#'
;        .if eax != NULL
;            ; 替换为字符串结束符
;            mov byte ptr [eax], 0
;        .endif
        
        ; 复制到数组
        invoke crt_strcpy, @linePtr, addr @szLine
        
        ; 增加行计数
        inc g_nScriptLines
    .endw
    
    ; 关闭文件
    invoke crt_fclose, @hFile
    
    ; 设置脚本运行状态
    mov g_bScriptRunning, 1
    
execute_commands:
    ; 检查是否已执行完所有命令
    mov eax, g_nCurrentLine
    .if eax >= g_nScriptLines
        ; 脚本执行完毕
        mov g_bScriptRunning, 0
        invoke crt_printf, offset szBatComplete, g_nScriptCmdCount
        mov eax, E_OK
		invoke DbgGetCmd,0
        ret
    .endif
    
    ; 获取当前行
    mov eax, g_nCurrentLine
    mov edx, 256  ; 每行最大长度
    mul edx
    lea edi, ScriptLines
    add edi, eax
    
    ; 解析命令和参数
    invoke RtlZeroMemory, addr @szCmd, sizeof @szCmd
    invoke RtlZeroMemory, addr @szArgs, sizeof @szArgs
    
    invoke crt_sscanf, edi, offset g_szFmtCmdArgs, addr @szCmd, addr @szArgs
    invoke crt__strlwr, addr @szCmd
    
	.if byte ptr [@szArgs] != 0  ; 如果有参数
	    invoke crt_printf, offset szExecuteCmdWithArgs, addr @szCmd, addr @szArgs
	.else  ; 无参数命令
	    invoke crt_printf, offset szExecuteCmd, addr @szCmd
	.endif
	

    ; 执行命令
    invoke ExecCmd, addr @szCmd, addr @szArgs
    mov @cmdResult, eax
    inc g_nScriptCmdCount
    inc g_nCurrentLine
    
;    ; 检查命令返回值
    .if @cmdResult == DBG_CONTINUE
        ; 命令导致程序继续执行
        mov eax, DBG_CONTINUE
        ret
    .endif
    
    ; 继续执行下一行命令
    jmp execute_commands
    
    ; 脚本执行结束
    mov g_bScriptRunning, 0
    

    
    invoke crt_printf, offset szBatComplete, g_nScriptCmdCount
    
        
        
        
    mov eax, @cmdResult
    

    invoke DbgGetCmd,0
    

    ret
CmdBat endp

; 指令记录功能
CmdToggleRecord proc szArgs:DWORD
    ; 开启/关闭指令记录
    ret
CmdToggleRecord endp

CmdShowHistory proc szArgs:DWORD
    ; 显示已记录的指令
    ret
CmdShowHistory endp

;内存dump
szDumpPESetEntryPoint db "设置入口点为当前EIP: 0x%08X", 0
CmdDumpPE proc uses edi ebx ecx szArgs:DWORD 
    LOCAL hSrcFile:DWORD
    LOCAL hDstFile:DWORD
    LOCAL szSrcPath[MAX_PATH]:BYTE
    LOCAL szDstPath[MAX_PATH]:BYTE
    LOCAL dosHeader:IMAGE_DOS_HEADER
    LOCAL secHeader:IMAGE_SECTION_HEADER
    LOCAL pBuffer:DWORD
    LOCAL dwBytesRead:DWORD
    LOCAL dwBytesWritten:DWORD
    LOCAL i:DWORD
	LOCAL szSectionName[9]:BYTE
    LOCAL dwSecTableOffset:DWORD
    LOCAL oldProtect:DWORD
	LOCAL @tempSecAddr:DWORD
    LOCAL @secNum:DWORD
    LOCAL @context:CONTEXT
    
	; 解析参数，获取输出文件名
    invoke crt_sscanf, szArgs, addr szFmtDumpPE, addr szDstPath
    .if eax != 1
        invoke LogMsg, LOG_ERROR, addr szDumpPEUsage
        ret
    .endif
	
	; 复制原始被调试程序文件到目标路径
	invoke CopyFile, addr szDebugPath, addr szDstPath, FALSE
	.if eax == 0
	    invoke LogMsg, LOG_ERROR, addr szDumpPECopyError, addr szDstPath
	    ret
	.endif
    
	; 打开目标文件进行读写
    invoke CreateFile, addr szDstPath, GENERIC_READ or GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0
    mov hDstFile, eax
    .if eax == INVALID_HANDLE_VALUE
        invoke LogMsg, LOG_ERROR, addr szDumpPEFileError, addr szDstPath
        ret
    .endif
    
	; 分配内存缓冲区
    invoke crt_malloc, 22000h
    mov pBuffer, eax
    .if eax == 0
        invoke CloseHandle, hDstFile
        invoke LogMsg, LOG_ERROR, addr szDumpPEMemError
        ret
    .endif
    
   ; 读取DOS头
    invoke ReadFile, hDstFile, addr dosHeader, sizeof IMAGE_DOS_HEADER, addr dwBytesRead, 0
    
    ; 计算NT头位置
    invoke SetFilePointer, hDstFile, dosHeader.e_lfanew, 0, FILE_BEGIN
    
    ; 读取NT头
    invoke ReadFile, hDstFile, pBuffer, sizeof IMAGE_NT_HEADERS, addr dwBytesRead, 0
    
    ;修改当前EIP为入口
    invoke DbgGetRegs,addr @context
    mov ebx,@context.regEip
    sub ebx, g_lpBaseOfImage   ; AddressOfEntryPoint = EIP - ImageBase
    
    mov edi,pBuffer
    assume edi:ptr IMAGE_NT_HEADERS  
    mov [edi].OptionalHeader.AddressOfEntryPoint,ebx
    assume edi:nothing 
    invoke SetFilePointer, hDstFile, dosHeader.e_lfanew, 0, FILE_BEGIN
	invoke WriteFile, hDstFile, pBuffer, sizeof IMAGE_NT_HEADERS, addr dwBytesWritten, 0
	invoke LogMsg, LOG_INFO, addr szDumpPESetEntryPoint, @context.regEip

    ; 从NT头中获取节数
    mov eax, pBuffer
    add eax, 6  ; 偏移到NumberOfSections字段
    movzx ecx, word ptr [eax]
    mov @secNum, ecx  ; 保存节数

    ; 重置节索引
    mov i, 0
    
    ; 计算节表位置
    mov eax, dosHeader.e_lfanew
    add eax, sizeof IMAGE_NT_HEADERS
    mov dwSecTableOffset, eax
    
    ; 输出操作信息
    invoke LogMsg, LOG_INFO, addr szDumpPEStart, addr szDstPath, g_lpBaseOfImage
    
    ; 处理每个节
    mov ebx,@secNum
    .while i < ebx
        ; 设置文件指针到节表项位置
        mov eax, dwSecTableOffset
        mov ecx, i
        imul ecx, sizeof IMAGE_SECTION_HEADER
        add eax, ecx
        
        invoke SetFilePointer, hDstFile, eax, 0, FILE_BEGIN
        
        ; 读取节表项
        invoke ReadFile, hDstFile, addr secHeader, sizeof IMAGE_SECTION_HEADER, addr dwBytesRead, 0
        .if eax == 0
            invoke LogMsg, LOG_ERROR, addr szDumpPEReadSecError, i
            jmp next_section
        .endif
        
        ; 复制节名到临时缓冲区
		invoke crt_memcpy, addr szSectionName, addr secHeader, 8  ; 拷贝8字节的节名
		mov byte ptr [szSectionName+8], 0  ; 添加字符串结束符
		
        ; 如果节没有原始数据，跳过
        mov eax, secHeader.SizeOfRawData
        .if eax == 0
            invoke LogMsg, LOG_INFO, addr szDumpPESkipSection, addr szSectionName
            jmp next_section
        .endif
        
        ; 设置文件指针到节数据位置
        invoke SetFilePointer, hDstFile, secHeader.PointerToRawData, 0, FILE_BEGIN
        .if eax == 0FFFFFFFFh
            invoke LogMsg, LOG_ERROR, addr szDumpPESetPtrError, addr szSectionName
            jmp next_section
        .endif
        
        ; 计算内存中节的地址
        mov eax, g_lpBaseOfImage
        add eax, secHeader.VirtualAddress
        mov @tempSecAddr,eax

		;修改内存保护属性
		invoke VirtualProtectEx, g_hProcess, @tempSecAddr, secHeader.SizeOfRawData, PAGE_READWRITE, addr oldProtect
		.if eax == 0
		    invoke LogMsg, LOG_ERROR, addr szDumpPEProtectError, addr szSectionName
		    jmp next_section
		.endif
		
        ; 读取节数据
        invoke DbgReadMemory, @tempSecAddr, pBuffer, secHeader.SizeOfRawData
        ;invoke ReadProcessMemory, g_hProcess, @tempSecAddr, pBuffer, secHeader.SizeOfRawData, addr dwBytesRead
        .if eax == 0
            invoke LogMsg, LOG_ERROR, addr szDumpPEReadSecDataError, addr szSectionName
            jmp next_section
        .endif
        
		; 恢复原始保护
		invoke VirtualProtectEx, g_hProcess, @tempSecAddr, secHeader.SizeOfRawData, oldProtect, addr oldProtect
		
        ; 写入节数据到文件
        invoke WriteFile, hDstFile, pBuffer, secHeader.SizeOfRawData, addr dwBytesWritten, 0
        .if eax == 0
            invoke LogMsg, LOG_ERROR, addr szDumpPEWriteError, addr szSectionName
            jmp next_section
        .endif
        
        ; 输出成功信息
        invoke LogMsg, LOG_INFO, addr szDumpPESectionDumped, \
                      addr szSectionName, \
                      secHeader.VirtualAddress, \
                      secHeader.SizeOfRawData, \
                      dwBytesWritten
        
next_section:

        inc i
        
    .endw
    
    ; 释放资源
	invoke CloseHandle, hDstFile
    ;invoke crt_free, pBuffer

    ; 输出完成信息
    invoke LogMsg, LOG_INFO, addr szDumpPEComplete, addr szDstPath
    
    mov eax, E_OK 
    ret
CmdDumpPE ENDP

CmdTrace proc szArgs:DWORD
invoke CreateFile, offset szRecardFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
    .if eax == INVALID_HANDLE_VALUE
        invoke crt_printf, offset szFileError, offset szRecardFileName
        mov eax, E_ERR
        ret
    .endif
    mov g_hRecordFile, eax
    
	mov g_isTrace,1
	invoke DbgSetStep
    ret
CmdTrace endp

;查看模块列表
CmdListModule proc uses esi edi
    LOCAL me32:MODULEENTRY32
    
    invoke CreateToolhelp32Snapshot, TH32CS_SNAPMODULE, g_pid

    .if eax == INVALID_HANDLE_VALUE
        invoke crt_printf, addr szSnapError
        mov eax, E_ERR
        ret
    .endif
    mov esi, eax
    
    mov me32.dwSize, sizeof MODULEENTRY32
    
    ; 获取第一个模块
    invoke Module32First, esi, addr me32
    .if eax == 0
        invoke crt_printf, addr szNoModules
        invoke CloseHandle, esi
        mov eax, E_ERR
        ret
    .endif
    
    ; 表头
    invoke crt_printf, addr szModuleHeader
    
    ; 遍历
@loop:
    ; 显示
    invoke crt_printf, addr szModuleFmt, addr me32.szModule, me32.modBaseAddr, me32.modBaseSize, addr me32.szExePath
    
    ; 下一个模块
    invoke Module32Next, esi, addr me32
    test eax, eax
    jnz @loop
    
    invoke CloseHandle, esi
    mov eax, S_OK
    ret
CmdListModule endp


;u 反汇编
CmdDiasm proc szArgs:DWORD
    LOCAL @lpAddress:DWORD
    LOCAL @Context:CONTEXT
	
    mov @lpAddress, 0
    
    ;有参数
    .if szArgs != NULL
        invoke crt_sscanf, szArgs, offset szFmtHex, addr @lpAddress
    .endif
    
    ; 如果没有参数且全局地址也为NULL，使用当前EIP
    .if @lpAddress == 0 && g_lpAddress == 0
        invoke DbgGetRegs, addr @Context
        mov eax, @Context.regEip
        mov @lpAddress, eax
    .endif
    
    invoke DbgDiasm, @lpAddress, 7
    
    mov eax, E_OK
    ret
    
CmdDiasm endp

;bp xxx 设置软件断点
CmdSetBpSoftware proc szArgs:DWORD
    LOCAL @lpAddress:DWORD

	mov @lpAddress, 0
	mov g_setbp,1
	;invoke LogMsg, LOG_DEBUG, offset szBpCmdArgs, szArgs    
	
	;读参数
	.if szArgs != NULL
        invoke crt_sscanf, szArgs, offset szFmtHex, addr @lpAddress
    .endif


    ; 地址是否有效
    .if @lpAddress == 0
        invoke crt_printf, offset szBpAddrInvalid
        mov eax, E_ERR
        ret
    .endif
  
    ; 调用断点管理添加bp断点
    invoke DbgSetBpSoft,@lpAddress
    .if eax == E_ERR
        ; 添加断点失败
        mov eax, E_ERR
        ret
    .endif
    
   ; invoke crt_printf, offset szBpAdded, @lpAddress

    mov eax, E_OK  ;这里用DBG_CONTINUE和E_OK的区别?
    ret


CmdSetBpSoftware endp

; 删除断点命令处理函数
CmdDeleteBp proc szArgs:DWORD
    LOCAL @szType[8]:BYTE
    LOCAL @target:DWORD
    LOCAL @result:DWORD
    
    ; 解析参数格式: bc [s/m/h] [id/addr]
    invoke crt_sscanf, szArgs, offset szFmtBpDelete, addr @szType, addr @target
    .if eax < 1
        invoke crt_printf, offset szBpDeleteUsage
        mov eax, E_ERR
        ret
    .endif

    ; 根据类型调用不同删除函数
    invoke crt__strlwr, addr @szType
    mov al, [@szType]
    
    .if al == 's'      ; 软件断点
        invoke DbgDeleteSoftBp, @target
    .elseif al == 'm'  ; 内存断点
        invoke DbgDeleteMemBp, @target
    .elseif al == 'h'  ; 硬件断点
        invoke DbgDeleteHardBp, @target
    .else
        ;invoke crt_printf, offset szInvalidBpType
        mov eax, E_ERR
    .endif
    
    mov eax, @result
    ret
CmdDeleteBp endp

DbgDeleteSoftBp proc dwBpId:DWORD
    LOCAL @index:DWORD
    LOCAL @origCode:BYTE
    
    ; 通过ID查找
    .if dwBpId < MAX_SOFTBP_COUNT
        mov eax, dwBpId
        imul eax, sizeof SoftBpInfo
        lea esi, g_softBpAry
        add esi, eax
        assume esi:ptr SoftBpInfo
        
        .if [esi].Enable == 1
            ; 恢复原始代码
            movzx eax, [esi].OldCode
            mov @origCode, al
            invoke DbgWriteMemory, [esi].Address, addr @origCode, 1
            
            ; 清除断点信息
            mov [esi].Address, 0
            mov [esi].OldCode, 0
            mov [esi].Enable, 0
            
            invoke LogMsg, LOG_INFO, offset szSoftBpDeleted, dwBpId
            mov eax, E_OK
        .else
            invoke LogMsg, LOG_WARNING, offset szBpNotFound, dwBpId
            mov eax, E_ERR
        .endif
    .endif
    ret
DbgDeleteSoftBp endp

; 删除内存断点
DbgDeleteMemBp proc dwBpId:DWORD
    LOCAL @index:DWORD
    LOCAL @pageIndex:DWORD
    LOCAL @origProtect:DWORD
    
    ; 通过ID删除
    .if dwBpId < MAX_MEMBP_COUNT
        mov eax, dwBpId
        imul eax, sizeof MemBpInfo
        lea esi, g_memBpAry[eax]
        assume esi:ptr MemBpInfo
        
        .if [esi].Enable == 1
            ; 计算页面索引
            mov eax, [esi].StartAddr
            and eax, 0FFFFF000h
            sub eax, g_oldprotStart
            shr eax, 12
            .if eax < MAX_MEMORY_REGIONS
                mov @pageIndex, eax
                shl eax, 2
                mov eax, [g_oldprotAry + eax]
                mov @origProtect, eax
                
                ; 恢复内存保护
                invoke DbgMemoryProtected, [esi].StartAddr, [esi].dwSize, @origProtect
                
                ; 清除断点信息
                mov [esi].StartAddr, 0
                mov [esi].EndAddr, 0
                mov [esi].dwSize, 0
                mov [esi].dwType, 0
                mov [esi].Enable, 0
                
                invoke LogMsg, LOG_INFO, offset szMemBpDeleted, dwBpId
                mov eax, E_OK
            .else
                invoke LogMsg, LOG_ERROR, offset szProtectRestoreFailed, [esi].StartAddr
                mov eax, E_ERR
            .endif
        .else
            invoke LogMsg, LOG_WARNING, offset szBpNotFound, dwBpId
            mov eax, E_ERR
        .endif
    .endif

    
    ret
DbgDeleteMemBp endp

; 删除硬件断点
DbgDeleteHardBp proc dwBpId:DWORD
    LOCAL @Context:CONTEXT
    
    ; 通过ID删除
    .if dwBpId < 4
        mov eax, dwBpId
        imul eax, sizeof HardBpInfo
        lea esi, g_HardBpArray[eax]
        assume esi:ptr HardBpInfo
        
        .if [esi].Enable == 1
            ; 获取当前上下文
            invoke DbgGetRegs, addr @Context
            
            ; 清除调试寄存器
            .if dwBpId == 0
                mov @Context.iDr0, 0
                and @Context.iDr7, 0FFFFFFFEh  ; 清除L0位
            .elseif dwBpId == 1
                mov @Context.iDr1, 0
                and @Context.iDr7, 0FFFFFFFBh  ; 清除L1位
            .elseif dwBpId == 2
                mov @Context.iDr2, 0
                and @Context.iDr7, 0FFFFFFEFh  ; 清除L2位
            .elseif dwBpId == 3
                mov @Context.iDr3, 0
                and @Context.iDr7, 0FFFFFFBFh  ; 清除L3位
            .endif
            
            ; 更新上下文
            invoke DbgSetRegs, addr @Context
            
            ; 清除结构体信息
            mov [esi].Address, 0
            mov [esi].dwType, 0
            mov [esi].dwSize, 0
            mov [esi].Enable, 0
            
            invoke LogMsg, LOG_INFO, offset szHardBpDeleted, dwBpId
            mov eax, E_OK
        .else
            invoke LogMsg, LOG_WARNING, offset szBpNotFound, dwBpId
            mov eax, E_ERR
        .endif
    .else
        invoke LogMsg, LOG_WARNING, offset szInvalidHardBpId
        mov eax, E_ERR
    .endif
    
    ret
DbgDeleteHardBp endp

;bm xxx 设置内存断点
CmdSetBpMemory proc szArgs:DWORD
    LOCAL @lpAddress:DWORD
    LOCAL @dwMemType:DWORD
    LOCAL @dwSize:DWORD
    LOCAL @szType[16]:BYTE
    LOCAL @pMemBp:DWORD
    LOCAL @tmpProtect:DWORD
   
    mov @lpAddress, 0
    mov @dwMemType, 0
    mov @dwSize, 0
    
	;读参数
	.if szArgs != NULL
        invoke crt_sscanf, szArgs, offset szFmtBpMem, addr @dwMemType, addr @dwSize, addr @lpAddress
    .endif

    invoke LogMsg, LOG_DEBUG, offset szBpMemArgs, szArgs, @dwMemType, @dwSize, @lpAddress
	
    ; 地址是否有效
    .if @lpAddress == 0
        invoke crt_printf, offset szBpAddrInvalid
        mov eax, E_ERR
        ret
    .endif
    
    ; 设置断点
	lea eax,  @szType
    mov al, [eax]
    .if al == 'r' || al == 'w' || al == 'e' || al == 'a'
        mov @dwMemType, eax
    .endif
    
    invoke DbgSetBpMemory, @lpAddress, @dwSize, @dwMemType
    .if eax==E_ERR
		invoke crt_printf, offset szBpMemAdded, @lpAddress, @dwMemType
    .endif

    mov eax, E_OK
    ret


CmdSetBpMemory endp

;bh xxx 设置硬件断点
CmdSetBpHardware proc szArgs:DWORD
    LOCAL @szBpType:DWORD
    LOCAL @bpSize:DWORD
    LOCAL @bpAddr:DWORD
    
	;读参数
	.if szArgs != NULL
        invoke crt_sscanf, szArgs, addr szFmtBpHard, addr @szBpType, addr @bpSize, addr @bpAddr
    .endif
    
    
;invoke crt_printf, offset szDebugParams, @szBpType, @bpSize, @bpAddr


    invoke LogMsg, LOG_DEBUG, offset szBpCmdArgs, szArgs
    ; 地址是否有效
    .if @bpAddr == 0
        invoke crt_printf, offset szBpAddrInvalid
        mov eax, E_ERR
        ret
    .endif



    ; 设置断点
    invoke DbgSetBpHard, @bpAddr,  @szBpType, @bpSize

    mov eax, E_OK
    ret


CmdSetBpHardware endp

;bl 列出断点
CmdListBreakPoint proc szArgs:DWORD
    LOCAL @bpType:BYTE
	LOCAL @pageAddr:DWORD
	
	 mov @bpType, 0
	 
	.if szArgs != NULL
        invoke crt_sscanf, szArgs, offset szFmtBpListCmd, addr @bpType
    .endif

	.if @bpType == 's'
        ; 显示软件断点列表
        invoke PrintSoftBpVec
    .elseif @bpType == 'm'
        ; 显示内存断点列表
        invoke PrintMemBpVec
    .elseif @bpType == 'h'
        ; 显示硬件断点列表
        invoke PrintHardBpVec
    .else
        ; 显示所有断点列表
        invoke PrintSoftBpVec
        invoke PrintMemBpVec
        invoke PrintHardBpVec
    .endif
    
    mov eax, E_OK
    ret
CmdListBreakPoint endp


;--------------------事件函数-------------------------
; 处理异常事件
EventException proc uses esi
    LOCAL @dwContinueStatus:DWORD
    
    ;默认不处理
    mov @dwContinueStatus, DBG_EXCEPTION_NOT_HANDLED
    
    ; 获取异常代码
    mov esi,offset g_DebugEv
    assume esi:ptr DEBUG_EVENT
    mov eax, [esi].u.Exception.pExceptionRecord.ExceptionCode
    
    .if eax == EXCEPTION_BREAKPOINT
        ; 断点异常
        invoke ExecBreakPoint
        mov @dwContinueStatus, eax
        
    .elseif eax == EXCEPTION_ACCESS_VIOLATION
        ; 访问异常
        invoke ExecAccess
        mov @dwContinueStatus, eax
        
    .elseif eax == EXCEPTION_SINGLE_STEP
        ; 单步异常
        invoke ExecStep
        mov @dwContinueStatus, eax
        
    .else
        ; 其他异常，不处理
        ;TODO log
        
    .endif
	
	assume esi:nothing 
	mov eax, @dwContinueStatus
	ret
EventException endp

; 处理进程创建事件
EventCreateProcess proc uses esi ecx
    ;LOCAL @lpStartAddress:DWORD
    LOCAL @Context:CONTEXT

    
    mov esi, offset g_DebugEv
    assume esi:ptr DEBUG_EVENT

    mov ecx, [esi].u.CreateProcessInfo.lpStartAddress
    

   	
    ;保存相关信息
	mov eax,[esi].u.CreateProcessInfo.lpBaseOfImage
	mov g_lpBaseOfImage,eax
    mov eax,[esi].u.CreateProcessInfo.lpStartAddress
    mov g_lpStartAddress,eax
    
	invoke LogMsg, LOG_INFO, offset szCreateProcessInfo, \
           offset g_szCREATE_PROCESS_DEBUG_EVENT, \
           [esi].u.CreateProcessInfo.lpBaseOfImage, \
           [esi].u.CreateProcessInfo.lpStartAddress, \
           [esi].u.CreateProcessInfo.lpThreadLocalBase
             

	

;invoke DbgSetBpMemory, 0040300ch, 1h, 'r'  ; 执行断点g
;invoke DbgSetBpMemory, 00401020h, 4h, 'a'  ; 执行断点

	


   	invoke DbgSetBpSoft,g_lpStartAddress


    assume esi:nothing

  	
    mov eax, DBG_CONTINUE
    ret
EventCreateProcess endp

; 处理线程创建事件
EventCreateThread proc
    invoke LogMsg, LOG_DEBUG, offset g_szCREATE_THREAD_DEBUG_EVENT
    mov eax, DBG_CONTINUE
    ret
EventCreateThread endp

; 处理线程退出事件
EventExitThread proc
    invoke LogMsg, LOG_DEBUG, offset g_szEXIT_THREAD_DEBUG_EVENT
    mov eax, DBG_CONTINUE
    ret
EventExitThread endp

; 处理进程退出事件
EventExitProcess proc
    invoke LogMsg, LOG_DEBUG, offset g_szEXIT_PROCESS_DEBUG_EVENT
    mov eax, E_ERR
    ret
EventExitProcess endp

; 处理DLL加载事件
EventLoadDll proc uses esi
    LOCAL @dwAddr:DWORD
    LOCAL @dwByteReaded:DWORD
    LOCAL @szwPath[512]:WORD      ; Unicode 路径缓冲区
    LOCAL @szPath[512]:BYTE       ; ANSI 路径缓冲区
    
    ; 打印事件类型
    ;invoke LogMsg, LOG_DEBUG, offset g_szLOAD_DLL_DEBUG_EVENT
    
    ; 获取DLL文件名指针的地址
   	mov eax,g_DebugEv.u.LoadDll.lpImageName;
   	mov @dwAddr,eax
   	
    ; 读取DLL文件名指针
   	invoke DbgReadMemory,@dwAddr,addr @dwAddr,sizeof DWORD
   	

    ; 读取DLL文件名(Unicode)
	invoke DbgReadMemory, @dwAddr, addr @szwPath, sizeof @szwPath 
	test eax,eax
	jz load_failed
	
	; Unicode转ANSI
    invoke WideCharToMultiByte,
        CP_ACP,                ; ANSI代码页
        0,                     ; 默认flags
        addr @szwPath,         ; Unicode源
        -1,                    ; 自动计算长度
        addr @szPath,          ; ANSI目标
        sizeof @szPath,        ; 目标缓冲区大小
        NULL,                  ; 默认字符
        NULL                   ; 使用默认字符标志
        
    test eax, eax
    jz load_failed
    
    ;读取成功
    invoke LogMsg, LOG_DEBUG, offset szLoadDllInfo, \
           addr @szPath, \                          ; ANSI文件名
           g_DebugEv.u.LoadDll.lpBaseOfDll, \      ; DLL基址
           g_DebugEv.u.LoadDll.hFile               ; 文件句柄
           
    invoke CloseHandle, g_DebugEv.u.LoadDll.hFile
    mov eax, DBG_CONTINUE
    ret
    
load_failed:
    invoke GetLastError
    invoke LogMsg, LOG_ERROR, offset szLoadDllError, eax
    mov eax, DBG_CONTINUE
    ret
    
EventLoadDll endp

; 处理DLL卸载事件
EventUnLoadDll proc
    mov eax, DBG_CONTINUE
    ret
EventUnLoadDll endp

; 处理调试字符串事件
EventOutputDebugString proc uses esi
    mov eax, DBG_CONTINUE
    ret
EventOutputDebugString endp

; 处理RIP事件
EventRip proc
    mov eax, DBG_CONTINUE
    ret
EventRip endp

;--------------------工具函数-------------------------
;解析输入的命令
DbgGetCmd proc fgetsBuf:DWORD
    LOCAL @szBuffer[1024]:BYTE
    LOCAL @szCmd[256]:BYTE
    LOCAL @szArgs[512]:BYTE
    LOCAL @dwContinueStatus:DWORD   
	LOCAL @hStdIn:DWORD
	LOCAL @logLine[512]:BYTE
	LOCAL @written:DWORD
        
        
        
    mov @dwContinueStatus, DBG_CONTINUE
    
        .if g_bScriptRunning == 1
	        ; 在脚本模式下，自动继续执行脚本而不等待用户输入
	        invoke CmdBat, NULL  ; 继续执行已加载的脚本
	        ret
    	.endif
    

GetCmd_loop:
	invoke RtlZeroMemory,addr @szBuffer,sizeof @szBuffer
	invoke RtlZeroMemory,addr @szCmd,sizeof @szCmd
	invoke RtlZeroMemory,addr @szArgs,sizeof @szArgs
	    
	invoke crt_printf, offset g_szCmdPrompt
	
	.if fgetsBuf !=NULL

	    ; 获取字符串长度以复制正确大小
;	    invoke crt_strlen, fgetsBuf
;	    .if eax > 0
;	        ; 只复制实际字符串长度，避免垃圾数据
;	        invoke crt_strcpy, addr @szBuffer, fgetsBuf
;	        mov fgetsBuf,0
;	    .endif
;	    
;	    ; 检查是否为空命令
;	    cmp byte ptr [@szBuffer], 0
;	    je exit_command
	.else

	    invoke crt_gets, addr @szBuffer
	    cmp byte ptr [@szBuffer], 0
	    je GetCmd_loop
	.endif
	

    
	invoke crt_sscanf, addr @szBuffer, offset g_szFmtCmdArgs, addr @szCmd, addr @szArgs
	invoke crt__strlwr,addr @szCmd
	
    	.if g_bLogCmd == TRUE
	        invoke crt_sprintf, addr @logLine, offset szLogLine, addr @szCmd, addr @szArgs
	        
	        ; 写入命令
	        invoke crt_strlen, addr @logLine
	        mov ecx,eax
	        invoke WriteFile, g_hLogFile, addr @logLine, ecx, addr @written, NULL
   		.endif
   		
 	xor ebx,ebx
	xor esi,esi
	mov esi,offset g_Commands

compare_cmd:
	.while(ebx<[g_nCommands])
	invoke GetStdHandle, STD_INPUT_HANDLE
	invoke FlushConsoleInputBuffer, eax
		invoke crt_strcmp,[esi].Command.szCmd,addr @szCmd
		test eax,eax
		;匹配
		jz find_cmd
		
		;不匹配 找下一项
		add esi,sizeof Command
		inc ebx
		jmp compare_cmd
	
	.endw
	

    ; 命令未找到
    invoke crt_printf, offset g_szCmdUnknown
    jmp GetCmd_loop
 
find_cmd:

	; 调用pfnCmd
	lea eax, @szArgs
	push eax
	call [esi].Command.pfnCmd
	
    mov @dwContinueStatus, eax
	
	; 检查是否需要继续循环
    cmp eax, DBG_CONTINUE
    je exit_command
    cmp eax, DBG_EXCEPTION_NOT_HANDLED
    je exit_command
    
    ; 清空输入缓冲区
	;invoke crt_fflush, offset _iob

 	;invoke FlushInputBuffer
 	
 	invoke GetStdHandle, STD_INPUT_HANDLE
	invoke FlushConsoleInputBuffer, eax
    jmp GetCmd_loop
    
exit_command:
	
    mov eax, @dwContinueStatus
	ret

DbgGetCmd endp
DbgReadMemory proc lpBaseAddress:DWORD, lpBuffer:DWORD, nSize:DWORD
    local @bytesRead:DWORD
    local @hProcess:DWORD
    
	invoke LogMsg,offset LOG_DEBUG,offset szDbgRead,lpBaseAddress,nSize
	
    invoke OpenProcess, PROCESS_ALL_ACCESS, FALSE, g_DebugEv.dwProcessId
    mov @hProcess, eax
    .if eax == NULL
        invoke GetLastError
        invoke LogMsg, LOG_ERROR, offset szDbgOpenError, eax
        mov eax, FALSE
        ret
    .endif
	
    invoke ReadProcessMemory,
 		@hProcess,         ; 被调试进程句柄
        lpBaseAddress,        ; 源地址
        lpBuffer,            ; 目标缓冲区
        nSize,               ; 读取大小
        addr @bytesRead      ; 实际读取的字节数
        
    test eax,eax
    jz read_failed
    
    ;检查读取字节数
    mov eax,@bytesRead
    cmp nSize,eax
    jne read_size_err
    
    ;成功 关闭句柄返回
    invoke LogMsg, LOG_DEBUG, offset szDbgReadSuccess, @bytesRead
    invoke CloseHandle, @hProcess
    mov eax,TRUE
    ret
    
open_failed:
	invoke GetLastError
    invoke LogMsg, LOG_ERROR, offset szDbgOpenError, eax
    mov eax, FALSE
    ret

read_failed:
    invoke GetLastError
    invoke LogMsg, LOG_ERROR, offset szDbgReadError, eax
    mov eax, FALSE
    ret
    
read_size_err:
    invoke LogMsg, LOG_ERROR, offset szDbgReadSizeError, @bytesRead, nSize
    mov eax, FALSE
    ret

DbgReadMemory endp

DbgWriteMemory proc lpBaseAddress:DWORD, lpBuffer:DWORD, nSize:DWORD
    local @bytesWritten:DWORD
    local @hProcess:DWORD

    invoke LogMsg, LOG_DEBUG, offset szDbgWrite, nSize,lpBaseAddress
    
    invoke OpenProcess, PROCESS_ALL_ACCESS, FALSE, g_DebugEv.dwProcessId
    mov @hProcess, eax
    .if eax == NULL
        invoke GetLastError
        invoke LogMsg, LOG_ERROR, offset szDbgOpenError, eax
        mov eax, FALSE
        ret
    .endif
    
    ; 写入内存
    invoke WriteProcessMemory,
        @hProcess,
        lpBaseAddress,
        lpBuffer,
        nSize,
        addr @bytesWritten
        
    test eax, eax
    jz write_failed
    

    
    ; 成功,记录日志并关闭句柄
    invoke LogMsg, LOG_DEBUG, offset szDbgWriteSuccess, @bytesWritten
    invoke CloseHandle, @hProcess
    mov eax, TRUE
    ret
    
    
write_failed:
    invoke GetLastError
    invoke LogMsg, LOG_ERROR, offset szDbgWriteError, eax
    invoke CloseHandle, @hProcess
    mov eax, FALSE
    ret
    

    
DbgWriteMemory endp

DbgGetPEinfo PROC uses ebx ecx eax edx esi edi
    LOCAL dosHeader:IMAGE_DOS_HEADER
    LOCAL dwOffset:DWORD
    LOCAL dwTemp:DWORD
    LOCAL @OPTHeader:IMAGE_OPTIONAL_HEADER 

    ; 读取DOS头
    invoke DbgReadMemory, g_lpBaseOfImage, addr dosHeader, sizeof IMAGE_DOS_HEADER
    .if eax == 0
        invoke crt__cprintf, addr szPEInfoInvalidDOS
        mov eax, 0
        ret
    .endif
    
    ; 验证DOS头标识
    mov ax, dosHeader.e_magic
    .if ax != 5A4Dh  ; 'MZ'
        invoke crt__cprintf, addr szPEInfoInvalidDOS
        mov eax, 0
        ret
    .endif
    
    ; 保存DOS头指针
    mov eax, g_lpBaseOfImage
    mov g_pDosHeader, eax
    
    ; 获取e_lfanew
    mov eax, dosHeader.e_lfanew
    mov dwOffset, eax
    
    ; 计算NT头的位置
    mov eax, g_lpBaseOfImage
    add eax, dwOffset
    mov g_pNtHeader, eax
    
    ; 读取NT头签名
    invoke DbgReadMemory, g_pNtHeader, addr dwTemp, 4
    .if eax == 0
        invoke crt__cprintf, addr szPEInfoInvalidNT
        mov eax, 0
        ret
    .endif
    
    ; 验证NT头签名
    .if dwTemp != 00004550h  ; 'PE\0\0'
        invoke crt__cprintf, addr szPEInfoInvalidNT
        mov eax, 0
        ret
    .endif
    
    ; 计算文件头和可选头的位置

       
    mov eax, g_pNtHeader
    add eax, 4  ; 跳过PE签名
    mov g_pFileHeader, eax
    
    ; 计算可选头位置
    mov eax, g_pFileHeader
    add eax, sizeof IMAGE_FILE_HEADER
 
    mov g_pOptionHeader, eax
    mov ecx,eax
    
   
  
    
    ; 读取节数量
    invoke DbgReadMemory, g_pFileHeader, addr dwTemp, sizeof IMAGE_FILE_HEADER
    .if eax == -1
        ret
    .endif
    
    movzx eax, word ptr [dwTemp + 2]  ; NumberOfSections偏移
    mov g_numberOfSection, eax
    
    ; 计算节表位置
    mov eax, g_pOptionHeader
    add eax, sizeof IMAGE_OPTIONAL_HEADER
    mov g_pSecHeader, eax
    
      
	    ; 保存IAT地址和大小
	; 读取可选头内容
	
	mov ecx,g_pOptionHeader
	add ecx,192
	invoke DbgReadMemory, ecx, addr dwTemp, sizeof dwTemp
	mov eax,dwTemp
	add eax,g_lpBaseOfImage
	mov g_pIatStart,eax
	
	mov g_pIatEnd,eax
	
	mov ecx,g_pOptionHeader
	add ecx,196
	invoke DbgReadMemory, ecx, addr dwTemp, sizeof dwTemp
	mov eax,dwTemp
	mov g_IatSize,eax
	add g_pIatEnd,eax


;mov eax, @OPTHeader.DataDirectory[1].VirtualAddress
;mov eax, @OPTHeader.DataDirectory[1].Size
    
   invoke LogMsg, LOG_DEBUG, addr szPEInfoSuccess, \
               g_lpBaseOfImage, \
               g_pDosHeader, \
               g_pNtHeader, \
               g_pFileHeader, \
               g_pOptionHeader, \
               g_pSecHeader, \
               g_numberOfSection,
               g_pIatStart, \
               g_IatSize
    
    mov eax, E_OK  ; 返回成功
    ret
DbgGetPEinfo ENDP

DbgRecard proc
    LOCAL @Context:CONTEXT
    LOCAL @buffer[16]:BYTE
    LOCAL @szAsm[256]:BYTE
    LOCAL @nCodeSize:DWORD
    LOCAL @nextAddr:DWORD
    LOCAL @written:DWORD
	LOCAL @szLine[512]:BYTE  
	;
    ;获取eip
    invoke DbgGetRegs, addr @Context
	invoke DbgReadMemory, @Context.regEip, addr @buffer, sizeof @buffer
    .if eax <= 0
    	mov eax,E_ERR
    	ret
    .endif
    
	invoke Decode2Asm, addr @buffer, addr @szAsm, addr @nCodeSize, @Context.regEip
	invoke crt_sprintf, addr @szLine, offset szTraceFormat, @Context.regEip, addr @szAsm
	invoke crt__strupr,addr @szLine
	;记录读的指令到文本
	invoke crt_strlen, addr @szLine
	mov ecx,eax
    invoke WriteFile, g_hRecordFile, addr @szLine, ecx, addr @written, NULL
    
    ;不需要关闭句柄吧，因为追踪完程序就结束了
	ret
DbgRecard endp

DbgGetRegs proc lpContext:DWORD
	LOCAL @hThread:HANDLE
	

	invoke OpenThread,THREAD_ALL_ACCESS,FALSE, g_DebugEv.dwThreadId
	mov @hThread,eax
	
	test eax,eax
	jz GetRegs_OpenThread_failed
	
	;  lpContext->ContextFlags = CONTEXT_ALL;
	mov eax,lpContext
	mov [eax].CONTEXT.ContextFlags,CONTEXT_ALL;怎么只有这个没有
	
	invoke GetThreadContext,@hThread,lpContext
	test eax,eax
	jz GetThreadContext_failed
	
	;成功
	invoke CloseHandle,@hThread
	mov eax,TRUE	
	ret
	
GetThreadContext_failed:
    invoke GetLastError
    invoke LogMsg, LOG_ERROR, offset szGetContextError, eax
    invoke CloseHandle, @hThread
    jmp failed
    
GetRegs_OpenThread_failed::
    invoke GetLastError
    invoke LogMsg, LOG_ERROR, offset szOpenThreadError, eax
    
failed:
    mov eax, FALSE
    ret
DbgGetRegs endp

DbgSetRegs proc lpContext:DWORD
    LOCAL @hThread:HANDLE
    
    invoke OpenThread, THREAD_ALL_ACCESS, FALSE, g_DebugEv.dwThreadId
    mov @hThread, eax
    
    test eax, eax
    jz SetRegs_OpenThread_failed
    
    mov eax, lpContext
    mov [eax].CONTEXT.ContextFlags, CONTEXT_ALL
    
    invoke SetThreadContext, @hThread, lpContext
    test eax, eax
    jz SetThreadContext_failed
    
    ; 成功
    invoke CloseHandle, @hThread
    mov eax, TRUE    
    ret
    
SetThreadContext_failed:
    invoke GetLastError
    invoke LogMsg, LOG_ERROR, offset szSetContextError, eax
    invoke CloseHandle, @hThread
    jmp failed
    
SetRegs_OpenThread_failed:
    invoke GetLastError
    invoke LogMsg, LOG_ERROR, offset szOpenThreadError, eax
    
failed:
    mov eax, FALSE
    ret
DbgSetRegs endp

DbgShowRegs proc uses ebx lpContext:DWORD 
	LOCAL @szApi[256]:BYTE
	
	mov ebx, lpContext
    assume ebx:ptr CONTEXT
	
	; EAX
    invoke crt_printf, offset szRegEax, [ebx].regEax
    invoke DbgGetApiName, [ebx].regEax, addr @szApi
    .if eax == E_OK
        invoke crt_printf, offset szRegApi, addr @szApi
    .endif
    
    ; EBX
    invoke crt_printf, offset szRegEbx, [ebx].regEbx
    invoke DbgGetApiName, [ebx].regEbx, addr @szApi
    .if eax == E_OK
        invoke crt_printf, offset szRegApi, addr @szApi
    .endif
    
    ; ECX
    invoke crt_printf, offset szRegEcx, [ebx].regEcx
    invoke DbgGetApiName, [ebx].regEcx, addr @szApi
    .if eax == E_OK
        invoke crt_printf, offset szRegApi, addr @szApi
    .endif
    
    ; EDX
    invoke crt_printf, offset szRegEdx, [ebx].regEdx
    invoke DbgGetApiName, [ebx].regEdx, addr @szApi
    .if eax == E_OK
        invoke crt_printf, offset szRegApi, addr @szApi
    .endif
    
    ; ESI
    invoke crt_printf, offset szRegEsi, [ebx].regEsi
    invoke DbgGetApiName, [ebx].regEsi, addr @szApi
    .if eax == E_OK
        invoke crt_printf, offset szRegApi, addr @szApi
    .endif
    
    ; EDI
    invoke crt_printf, offset szRegEdi, [ebx].regEdi
    invoke DbgGetApiName, [ebx].regEdi, addr @szApi
    .if eax == E_OK
        invoke crt_printf, offset szRegApi, addr @szApi
    .endif
    
    ; EBP
    invoke crt_printf, offset szRegEbp, [ebx].regEbp
    
    ; ESP
    invoke crt_printf, offset szRegEsp, [ebx].regEsp
    
    ; EIP
    invoke crt_printf, offset szRegEip, [ebx].regEip
    invoke DbgGetApiName, [ebx].regEip, addr @szApi
    .if eax == E_OK
        invoke crt_printf, offset szRegApi, addr @szApi
    .endif
    
    ; EFLAGS
    invoke crt_printf, offset szRegEfl, [ebx].regFlag
    
    
;    invoke crt_printf, offset szFmtRegs1,[ebx].regEax, [ebx].regEbx,[ebx].regEcx, [ebx].regEdx
;           
;    invoke crt_printf, offset szFmtRegs2,[ebx].regEsi, [ebx].regEdi,[ebx].regEbp, [ebx].regEsp
;           
;    invoke crt_printf, offset szFmtRegs3, [ebx].regEip,[ebx].regFlag
           
	assume ebx:nothing
    ret

DbgShowRegs endp

szFmtDisasmInstApi db "%s ; %s", 0Dh, 0Ah, 0  
szspace db 0
DbgDiasm proc uses ebx esi edi ecx edx lpAddress:DWORD, nLine:UINT

    LOCAL @buffer[1000h]:BYTE
    LOCAL @szAsm[256]:BYTE
    LOCAL @nCodeSize:UINT
    LOCAL @i:DWORD
    LOCAL @curAddr:DWORD
    LOCAL @totalSize:DWORD
    LOCAL @bpAddr:DWORD
    LOCAL @bufferEnd:DWORD       ; 缓冲区结束地址
	LOCAL @szApiName[256]:BYTE
	LOCAL @dwTargetAddr:DWORD
	LOCAL @bNeedAddr:DWORD	
	LOCAL @strbuffer[256]:BYTE
		
	
    ; 设置从参数传的地址开始反汇编
    .if g_lpAddress == 0
        mov eax, lpAddress
        mov g_lpAddress, eax
    .endif
    
    mov eax, g_lpAddress
    mov @curAddr, eax
    
    ; 读取内存内容 - 整个1000h字节
    invoke DbgReadMemory, @curAddr, addr @buffer, sizeof @buffer
    .if eax == 0
        invoke GetLastError
        invoke LogMsg, LOG_ERROR, offset szDisasmError, eax
        mov eax, FALSE
        ret
    .endif
    
    ; 计算缓冲区结束地址
    mov eax, @curAddr
    add eax, sizeof @buffer
    mov @bufferEnd, eax
    
    ; 修复缓冲区中的所有断点 - 遍历断点数组
    mov edx, 0  ; 索引
    .while edx < MAX_SOFTBP_COUNT
        ; 获取断点信息
        mov eax, edx
        imul eax, sizeof SoftBpInfo
        lea ebx, g_softBpAry
        add ebx, eax
        
        ; 检查是否是活动断点
        mov eax, [ebx].SoftBpInfo.Enable
        .if (eax == 1)
            ; 获取断点地址
            mov eax, [ebx].SoftBpInfo.Address
            mov @bpAddr, eax
            
            ; 检查断点是否在缓冲区范围内
            mov eax, @bpAddr
            cmp eax, @curAddr
            jl bp_out_of_range    ; 如果小于起始地址，跳过
            
            mov eax, @bpAddr
            cmp eax, @bufferEnd
            jge bp_out_of_range   ; 如果大于等于结束地址，跳过
            
            ; 在缓冲区范围内，计算偏移
            mov eax, @bpAddr
            sub eax, @curAddr
            
            ; 在缓冲区中恢复原始指令
            lea esi, @buffer
            add esi, eax
            mov al, [ebx].SoftBpInfo.OldCode
            mov byte ptr [esi], al
            
bp_out_of_range:
        .endif
        
        inc edx
    .endw
    
 ;----------------现在的buffer中已经修复了因为软件断点导致的CC 下面还需要解析CALL和JMP等跳转指令 如果跳转到地址需要比对API地址信息
 
    ; 初始化行计数和缓冲区
    mov @bNeedAddr, 0
    mov @i, 0
    mov @totalSize, 0
    lea esi, @buffer
    mov eax, @curAddr     ; 重置当前地址为起始地址
    mov @curAddr, eax
    
    ; 开始反汇编循环
    mov eax, @i
    .while eax < nLine
        ; 正常反汇编 (缓冲区已修复)
        invoke Decode2Asm, esi, addr @szAsm, addr @nCodeSize, @curAddr
        
        ; 显示地址 - 使用统一格式
        invoke crt_printf, offset szFmtDisasmAddr, @curAddr
        
        ; 显示所有字节
        mov ecx, 0
        .while ecx < @nCodeSize
            movzx eax, byte ptr [esi+ecx]
            push ecx
            invoke crt_printf, offset szFmtDisasmByte, eax
            pop ecx
            inc ecx
        .endw
        
        ; 补齐空格到固定宽度
        mov ecx, @nCodeSize
        .while ecx < 8
            push ecx
            invoke crt_printf, offset szFmtDisasmSpace
            pop ecx
            inc ecx
        .endw
        
;-------------------------- 打印反汇编文本-------------------------------------------------------------
        
		 ;TODO 如果是JMP /CALL 等指令 我们需要在反汇编后+识别地址到API的名称 然后再换行
		 
		;写一个JCC指令的表 如果匹配其中某个指令 需要提取地址
		 mov ebx,0
		 .while ebx < NUM_JCC_INST
		 	mov	ecx,g_JCCTable[ebx*4]
		 	invoke crt_strstr,addr @szAsm,ecx
		 	.if eax!=0
		 		mov @bNeedAddr,1
		 		.break
		 	.endif
		 	inc ebx
		 .endw
		 
		 ; 普通指令，直接打印
		 .if @bNeedAddr==0
        	invoke crt_printf, offset szFmtDisasmInst, addr @szAsm
        	jmp next_line
		 .endif
		
        ; 从指令中提取地址
       
		;这里是直接调用 比如 CALL xxxxxxxx  如果是间接调用 比如 call dword ptr ds:[004230A8]  怎么拿[]内的值取内容然后调用DbgGetApiName
        invoke crt_strchr,addr @szAsm,' '  
		.if eax !=0
			mov ebx,eax;保存字符串指针
			
			;区分 直接还是间接跳转
			invoke crt_strchr, ebx, '['
			.if eax!=0

			;int 3
				;是间接调用 跳过[
				mov edi,eax
				inc edi
				
				;复制[右边的字符串
       			 invoke crt_strcpy, addr @strbuffer, edi
        
				invoke crt_strtok, addr @strbuffer, offset szBracketEnd
				;此时得到了[XXX]中的数字 也可能是[reg+n]  
				invoke crt_strtoul,addr @strbuffer,0,16
				mov @dwTargetAddr,eax
			.else
				;直接调用
				invoke crt_strtoul,ebx,0,16
				mov @dwTargetAddr,eax
			.endif
			
			
		

			;查询API名称
			invoke DbgGetApiName, @dwTargetAddr, addr @szApiName
			
			.if eax !=-1
		        ; 显示带API名称的反汇编
		 
		        invoke crt_printf, offset szFmtDisasmInstApi, addr @szAsm, addr @szApiName
		    .else
		        invoke crt_printf, offset szFmtDisasmInst, addr @szAsm
		    .endif
		.else
    		invoke crt_printf, offset szFmtDisasmInst, addr @szAsm
		.endif
		
next_line:
        
        ; 更新地址和指针
        mov eax, @nCodeSize
        add esi, eax      ; 缓冲区指针前进
        add @curAddr, eax ; 当前地址前进
        add @totalSize, eax
        mov @bNeedAddr,0
        ; 增加行计数
        inc @i
        mov eax, @i
    .endw
    
    ; 更新全局地址
    mov eax, @curAddr
    mov g_lpAddress, eax
    
    mov eax, TRUE
    ret
DbgDiasm endp

;设置软件断点
DbgSetBpSoft proc uses ebx lpAddress:DWORD
	LOCAL @origCode:BYTE
	LOCAL @index:DWORD
	
	mov @index,0  
	
    ; 检查断点是否已存在
    
    .while @index < MAX_SOFTBP_COUNT
    lea edi, g_softBpAry
    	mov eax,@index
    	imul eax,sizeof SoftBpInfo
    	add edi,eax
        mov esi, edi
        assume esi:ptr SoftBpInfo
        
        ; 检查地址是否匹配
        mov eax, [esi].Address
        .if eax == lpAddress
            ; 已存在相同地址的断点
            invoke LogMsg, LOG_WARNING, offset szBpAlreadyExists, lpAddress
            mov eax, E_ERR
            ret
        .endif
        
        ; 获取下一个断点
        inc @index
    .endw    
    
    ; 读取原始代码
    invoke DbgReadMemory, lpAddress, addr @origCode, sizeof BYTE
    .if eax <= 0
        invoke LogMsg, LOG_ERROR, offset szBpReadFailed, lpAddress
        mov eax, E_ERR
        ret
    .endif
    
    ;选数组空位存断点信息
    mov @index,0  
    
    .while @index < MAX_SOFTBP_COUNT
    lea edi, g_softBpAry
    	mov eax,@index
    	imul eax,sizeof SoftBpInfo
    	add edi,eax
        mov esi, edi
        assume esi:ptr SoftBpInfo
       
        .if[esi].Address==0
        	;找到空位
        	mov eax,lpAddress
        	mov [esi].Address,eax
        	mov al,@origCode
        	mov [esi].OldCode,al
        	mov [esi].Enable,1        	
        	.break
        .endif
        ; 获取下一个断点
        inc @index
    .endw    

    
  	;修改
    mov @origCode,0CCh
    invoke DbgWriteMemory,lpAddress, addr @origCode,sizeof BYTE
    .if eax <= 0
	  ;  invoke LogMsg, LOG_ERROR, offset szSetBpWriteError, lpAddress
	    mov eax, E_ERR
	    ret
	.endif

	; 保存断点地址
    mov eax, lpAddress
    mov g_lpBpAddress, eax
    
    ; 输出保存的断点信息
   ;invoke LogMsg, LOG_INFO, offset szBreakpointSet, lpAddress, @origCode
    mov eax, DBG_CONTINUE
    ret

DbgSetBpSoft endp

; 设置内存断点函数
DbgSetBpMemory proc uses ebx esi edi lpAddress:DWORD, dwSize:DWORD, dwType:DWORD
    LOCAL @pMemBp:DWORD
    LOCAL @tmpProtect:DWORD
    LOCAL @endAddr:DWORD
    LOCAL @hProcess:DWORD
	LOCAL @index:DWORD
	
    ; 调试输出 - 参数信息
   ; invoke LogMsg, LOG_DEBUG, offset szDbgBpParams, lpAddress, dwSize, dwType
    
    ; 计算结束地址
    mov eax, lpAddress
    add eax, dwSize
    dec eax
    mov @endAddr, eax
    
	  ;选数组空位存断点信息
    mov @index,0  
    
    .while @index < MAX_MEMBP_COUNT
    lea edi, g_memBpAry
    	mov eax,@index
    	imul eax,sizeof MemBpInfo
    	add edi,eax
        mov esi, edi
        assume esi:ptr MemBpInfo
        
        .if [esi].StartAddr==0
        	;找到空位
			 mov eax, lpAddress
            mov [esi].StartAddr, eax          ; 设置起始地址
            mov eax, lpAddress
            add eax, dwSize
            dec eax                           ; 计算结束地址 = 起始地址 + 大小 -1
            mov [esi].EndAddr, eax
            mov eax, dwSize
            mov [esi].dwSize, eax             ; 设置区域大小
            mov eax, dwType
            mov [esi].dwType, eax             ; 设置断点类型（读/写/执行）
            mov [esi].TempDisabled, 0         ; 初始化临时禁用标志
            mov [esi].Enable, 1               ; 启用断点
        	.break
        .endif
        ; 获取下一个断点
        inc @index
    .endw    


    
    ; 修改内存保护属性
    invoke DbgMemoryProtected, lpAddress, dwSize, PAGE_NOACCESS
    mov @tmpProtect, eax
    
    ; 记录设置成功信息
    invoke LogMsg, LOG_INFO, offset szDbgBpComplete, lpAddress, dwSize, dwType
    ;invoke PrintMemBpList
    ; 返回成功
    mov eax, DBG_CONTINUE
    ret
DbgSetBpMemory endp

PrintDR7Bits PROC uses ebx ecx edx dr7value:DWORD
    LOCAL @L0:DWORD, @G0:DWORD, @L1:DWORD, @G1:DWORD
    LOCAL @L2:DWORD, @G2:DWORD, @L3:DWORD, @G3:DWORD
    LOCAL @LE:DWORD, @GE:DWORD, @GD:DWORD
    LOCAL @RW0:DWORD, @LEN0:DWORD, @RW1:DWORD, @LEN1:DWORD
    LOCAL @RW2:DWORD, @LEN2:DWORD, @RW3:DWORD, @LEN3:DWORD
    
    ; 提取各个位
    mov eax, dr7value
    
    ; 提取L0-G3位 (0-7)
    and eax, 1
    mov @L0, eax
    
    mov eax, dr7value
    shr eax, 1
    and eax, 1
    mov @G0, eax
    
    mov eax, dr7value
    shr eax, 2
    and eax, 1
    mov @L1, eax
    
    mov eax, dr7value
    shr eax, 3
    and eax, 1
    mov @G1, eax
    
    mov eax, dr7value
    shr eax, 4
    and eax, 1
    mov @L2, eax
    
    mov eax, dr7value
    shr eax, 5
    and eax, 1
    mov @G2, eax
    
    mov eax, dr7value
    shr eax, 6
    and eax, 1
    mov @L3, eax
    
    mov eax, dr7value
    shr eax, 7
    and eax, 1
    mov @G3, eax
    
    ; 提取LE, GE, GD位 (8, 9, 13)
    mov eax, dr7value
    shr eax, 8
    and eax, 1
    mov @LE, eax
    
    mov eax, dr7value
    shr eax, 9
    and eax, 1
    mov @GE, eax
    
    mov eax, dr7value
    shr eax, 13
    and eax, 1
    mov @GD, eax
    
    ; 提取RW和LEN字段
    mov eax, dr7value
    shr eax, 16
    and eax, 3
    mov @RW0, eax
    
    mov eax, dr7value
    shr eax, 18
    and eax, 3
    mov @LEN0, eax
    
    mov eax, dr7value
    shr eax, 20
    and eax, 3
    mov @RW1, eax
    
    mov eax, dr7value
    shr eax, 22
    and eax, 3
    mov @LEN1, eax
    
    mov eax, dr7value
    shr eax, 24
    and eax, 3
    mov @RW2, eax
    
    mov eax, dr7value
    shr eax, 26
    and eax, 3
    mov @LEN2, eax
    
    mov eax, dr7value
    shr eax, 28
    and eax, 3
    mov @RW3, eax
    
    mov eax, dr7value
    shr eax, 30
    and eax, 3
    mov @LEN3, eax
    
    ; 打印所有位
    invoke crt_printf, offset szDR7Bits,
        @L0, @G0, @L1, @G1, @L2, @G2, @L3, @G3,
        @LE, @GE, @GD,
        @RW0, @LEN0, @RW1, @LEN1,
        @RW2, @LEN2, @RW3, @LEN3
    
    ret
PrintDR7Bits ENDP

DbgSetBpHard PROC uses esi edi ecx dwAddr:DWORD, dwType:DWORD, dwSize:DWORD
    LOCAL @Context:CONTEXT
    LOCAL @dwIdx:DWORD
    LOCAL @dr7:DWORD
    ; 打印接收到的参数
    invoke crt_printf, offset szHardBpParams, dwAddr, dwType, dwSize
    
    ; 初始化索引为-1表示未找到可用寄存器
    mov @dwIdx, -1
    
    ; 获取当前线程上下文
    invoke RtlZeroMemory, addr @Context, sizeof @Context
    invoke DbgGetRegs, addr @Context
    
    ; 打印设置前的DR值
    invoke crt_printf, offset szDebugDR, @Context.iDr0, @Context.iDr1, @Context.iDr2, @Context.iDr3, @Context.iDr7
    
    ; 保存原始DR7值
    mov eax, @Context.iDr7
    mov @dr7,eax
    
    ; 查找可用的硬件断点槽位
    mov edx, @Context.iDr7
    and edx, 1h
    .if edx == 0 ; DR0可用
        mov ebx, dwAddr
        mov @Context.iDr0, ebx
        mov @dwIdx, 0
    .else
        mov edx, @Context.iDr7
        and edx, 4h
        .if edx == 0 ; DR1可用
            mov ebx, dwAddr
            mov @Context.iDr1, ebx
            mov @dwIdx, 1
        .else
            mov edx, @Context.iDr7
            and edx, 10h
            .if edx == 0 ; DR2可用
                mov ebx, dwAddr
                mov @Context.iDr2, ebx
                mov @dwIdx, 2
            .else
                mov edx, @Context.iDr7
                and edx, 40h
                .if edx == 0 ; DR3可用
                    mov ebx, dwAddr
                    mov @Context.iDr3, ebx
                    mov @dwIdx, 3
                .endif
            .endif
        .endif
    .endif
    
    ; 检查是否找到可用位置
    mov eax, @dwIdx
    .if eax == -1 ; 没有寄存器可用
        invoke crt_printf, offset szHardBpFull
        mov eax, -1
        ret
    .endif
    
    ; 根据索引设置DR7寄存器
    mov eax, @dwIdx
    .if eax == 0
        ; 启用L0位
        or @dr7, 1
        
        ; 清除RW0和LEN0位 (位16-19)
        and @dr7, 0FFF0FFFFh
        
        ; 直接使用dwType作为RW值 (位16-17)
        mov eax, dwType
        and eax, 3       ; 确保只使用低2位
        shl eax, 16      ; 移动到RW0位置
        or @dr7, eax
        
        ; 直接使用dwSize作为LEN值 (位18-19)
        mov eax, dwSize
        and eax, 3       ; 确保只使用低2位
        shl eax, 18      ; 移动到LEN0位置
        or @dr7, eax
        
    .elseif eax == 1
        ; 启用L1位
        or @dr7, 4
        
        ; 清除RW1和LEN1位 (位20-23)
        and @dr7, 0FF0FFFFFh
        
        ; 直接使用dwType作为RW值 (位20-21)
        mov eax, dwType
        and eax, 3       ; 确保只使用低2位
        shl eax, 20      ; 移动到RW1位置
        or @dr7, eax
        
        ; 直接使用dwSize作为LEN值 (位22-23)
        mov eax, dwSize
        and eax, 3       ; 确保只使用低2位
        shl eax, 22      ; 移动到LEN1位置
        or @dr7, eax
        
    .elseif eax == 2
        ; 启用L2位
        or @dr7, 10h
        
        ; 清除RW2和LEN2位 (位24-27)
        and @dr7, 0F0FFFFFFh
        
        ; 直接使用dwType作为RW值 (位24-25)
        mov eax, dwType
        and eax, 3       ; 确保只使用低2位
        shl eax, 24      ; 移动到RW2位置
        or @dr7, eax
        
        ; 直接使用dwSize作为LEN值 (位26-27)
        mov eax, dwSize
        and eax, 3       ; 确保只使用低2位
        shl eax, 26      ; 移动到LEN2位置
        or @dr7, eax
        
    .elseif eax == 3
        ; 启用L3位
        or @dr7, 40h
        
        ; 清除RW3和LEN3位 (位28-31)
        and @dr7, 00FFFFFFh
        
        ; 直接使用dwType作为RW值 (位28-29)
        mov eax, dwType
        and eax, 3       ; 确保只使用低2位
        shl eax, 28      ; 移动到RW3位置
        or @dr7, eax
        
        ; 直接使用dwSize作为LEN值 (位30-31)
        mov eax, dwSize
        and eax, 3       ; 确保只使用低2位
        shl eax, 30      ; 移动到LEN3位置
        or @dr7, eax
    .endif
    
    ; 设置DR7的最终值
    mov eax, @dr7
    mov @Context.iDr7, eax
    
    ; 清除DR6状态寄存器
    mov @Context.iDr6, 0

    invoke crt_printf, offset szDR7Modified, @Context.iDr7

    invoke DbgSetRegs, addr @Context
    
    ;输出调试信息
    invoke RtlZeroMemory, addr @Context, sizeof @Context
    invoke DbgGetRegs, addr @Context
    invoke crt_printf, offset szDebugDR, @Context.iDr0, @Context.iDr1, @Context.iDr2, @Context.iDr3, @Context.iDr7
    invoke PrintDR7Bits, @Context.iDr7
    
    ; 更新硬件断点数组
    mov eax, @dwIdx
    imul eax, sizeof HardBpInfo
    lea edi, g_HardBpArray
    add edi, eax
    
    ; 填充断点信息
    assume edi:ptr HardBpInfo
    mov eax, dwAddr
    mov [edi].HardBpInfo.Address, eax
    mov eax, @dwIdx
    mov [edi].HardBpInfo.Index, eax
    mov eax, dwType
    mov [edi].HardBpInfo.dwType, eax
    mov eax, dwSize
    mov [edi].HardBpInfo.dwSize, eax
    mov [edi].HardBpInfo.Enable, 1
    
    ; 打印设置成功信息
    invoke crt_printf, offset szHardBpSet, @dwIdx, dwAddr
    
    ; 返回断点索引
    mov eax, @dwIdx
    ret
DbgSetBpHard ENDP

;修改TF标志实现单步
DbgSetStep proc

    LOCAL @Context:CONTEXT
    

    invoke DbgGetRegs, addr @Context
    or @Context.regFlag, 100h  ; EFLAG_TF = 100h
    invoke DbgSetRegs, addr @Context
	
    mov eax, DBG_CONTINUE
	ret

DbgSetStep endp

;修改内存页属性
DbgMemoryProtected proc uses ebx ecx edx lpBaseAddress:DWORD, dwSize:DWORD, flNewProtect:DWORD;我们不关心旧属性 让他保存在局部变量自己销毁就可以
	LOCAL @oldProtect:DWORD
	
    ; 检查进程句柄是否有效
    .if g_hProcess == 0
        invoke LogMsg, LOG_ERROR, offset szInvalidProcessHandle
        mov eax, 0
        ret
    .endif
    
    invoke VirtualProtectEx, 
    g_hProcess,             
    lpBaseAddress,          
    dwSize,                
    flNewProtect,          ; 新的保护属性
    addr @oldProtect         
    mov ebx, eax

	 .if ebx == 0
        invoke GetLastError
        invoke LogMsg, LOG_ERROR, offset szDbgMemoryProtectedError, eax
        mov eax, E_ERR
    .else
        ;invoke LogMsg, LOG_DEBUG, offset szDbgMemoryProtectedDetail, 
       ;     lpBaseAddress, dwSize, @oldProtect, flNewProtect
      	mov eax, E_OK
    .endif
    
	ret

DbgMemoryProtected endp

;int DbgSetMemBp(LPVOID lpAddress, char nType, unsigned nSize)
;int DbgMemoryProtected(LPVOID lpBaseAddress, SIZE_T dwSize, DWORD flNewProtect, PDWORD lpflOldProtect);

;--------------------断点链表-------------------------
PrintMemBpVec PROC
 LOCAL @index:DWORD
	  ;选数组空位存断点信息
    mov @index,0  
    .while @index < MAX_MEMBP_COUNT
    lea edi, g_memBpAry
    	mov eax,@index
    	imul eax,sizeof MemBpInfo
    	add edi,eax
        mov esi, edi
        assume esi:ptr MemBpInfo
        
        .if [esi].Enable==1
		;打印
		 invoke crt_printf,offset szMemBpFormat,
                @index,                     ; ID
                [esi].StartAddr,            ; 起始地址
                [esi].EndAddr,              ; 结束地址
                [esi].dwSize,               ; 大小
                [esi].dwType,               ; 类型
                [esi].TempDisabled,         ; 临时禁用标志
                [esi].Enable                ; 启用状态
        .endif
        ; 获取下一个断点
        inc @index
    .endw    


	ret

PrintMemBpVec endp

PrintSoftBpVec PROC
    LOCAL @index:DWORD
    LOCAL @base:DWORD
    LOCAL buffer[256]:BYTE

    mov @index, 0
    lea eax, g_softBpAry           ; 获取软件断点数组基址
    mov @base, eax

    .while @index < MAX_SOFTBP_COUNT
        ; 计算当前元素地址 = 基址 + index * sizeof(SoftBpInfo)
        mov eax, @index
        imul eax, sizeof SoftBpInfo
        add eax, @base
        mov esi, eax               ; esi指向当前断点结构体
        assume esi:ptr SoftBpInfo

        .if [esi].Enable == 1       ; 检查断点是否启用
            ; 格式化输出信息
            invoke crt_printf,offset szSoftBpFormat,
                @index,             ; ID
                [esi].Address,      ; 地址
                [esi].OldCode,       ; 原始代码
                [esi].Enable         ; 启用状态
                
        .endif

        inc @index                  ; 处理下一个断点
    .endw

    ret
PrintSoftBpVec endp

; 打印硬件断点列表 - 直接显示原始值
PrintHardBpVec PROC uses esi edi ecx
    LOCAL @count:DWORD

    ; 遍历硬件断点数组
    mov @count, 0
    mov ebx, 0
    .while ebx < 4
        mov eax, ebx
        imul eax, sizeof HardBpInfo
        lea edi, g_HardBpArray
        add edi, eax
        
        ; 检查断点是否启用
        .if [edi].HardBpInfo.Enable == 1
            ; 直接打印原始值，不进行转换
            invoke crt_printf, offset szHardBpListEntry,
                [edi].HardBpInfo.Index,       ; 索引
                [edi].HardBpInfo.dwType,      ; 原始类型值
                [edi].HardBpInfo.dwSize,      ; 原始大小值
                [edi].HardBpInfo.Address      ; 地址
                
            inc @count
        .endif
        
        inc ebx
    .endw
    
    ; 打印统计信息
    invoke crt_printf, offset szHardBpListFooter, @count
    
    ret
PrintHardBpVec ENDP
;--------------------调试框架-------------------------

; 启动调试
StartDebug proc uses ebx lpApplicationName:LPCTSTR, lpCommandLine:LPCTSTR
    LOCAL @si:STARTUPINFO
    LOCAL @pi:PROCESS_INFORMATION  
	LOCAL @dwContinueStatus:DWORD
    LOCAL @dwCreationFlags:DWORD

	
	; 初始化日志系统
    invoke LogInit
    invoke LogSetLevel, LOG_DEBUG;日志级别
    invoke LogFilter, offset szDbgReadMemory
    invoke LogFilter, offset szEventLoadDll
    ;invoke LogFilter, offset szEventExitThread
    ;invoke LogFilter, offset szEventUnLoadDll
    ;invoke LogFilter, offset szEventCreateThread
	
	;LogMsg(LOG_DEBUG, "StartDebug lpApplicationName:%s lpCommandLine:%s\n", lpApplicationName, lpCommandLine);
	invoke LogMsg, LOG_DEBUG, offset szLogStart,lpApplicationName, lpCommandLine
           
	invoke RtlZeroMemory,addr @si,sizeof STARTUPINFO
	invoke RtlZeroMemory,addr @pi,sizeof PROCESS_INFORMATION
    invoke  RtlZeroMemory,offset g_DebugEv, size g_DebugEv
	mov @si.cb, sizeof STARTUPINFO

	invoke CreateProcess,
        NULL,     ; 应用程序名
        offset szDebugPath,        			; 命令行
        NULL,                  
        NULL,                  
        FALSE,                 
        DEBUG_PROCESS, 
        NULL,                 
        NULL,                  
        addr @si,              
        addr @pi               
    
    ;创建进程失败
    test eax, eax
    jz create_failed
    
    .if eax!=0
    	mov eax,@pi.hProcess
    	mov g_hProcess,eax
    .endif
    
	mov eax, @pi.dwProcessId
    mov g_pid, eax 
  	
  

    
    ;创建进程成功
    ;LogMsg(LOG_DEBUG, "StartDebug CreateProcess dwProcessId:%d dwThreadId:%d\n", pi.dwProcessId, pi.dwThreadId);
    invoke LogMsg, LOG_DEBUG, offset szLogCreateProcess,lpApplicationName, lpCommandLine
debug_loop:

    mov @dwContinueStatus, DBG_CONTINUE
    
	;等待
    invoke WaitForDebugEvent, offset g_DebugEv, INFINITE
     mov eax, g_DebugEv.dwDebugEventCode
    
    .if eax == EXCEPTION_DEBUG_EVENT
        invoke EventException
        mov @dwContinueStatus, eax
    
    .elseif eax == CREATE_PROCESS_DEBUG_EVENT
        invoke EventCreateProcess
        mov @dwContinueStatus, eax
    .elseif eax == CREATE_THREAD_DEBUG_EVENT
        invoke EventCreateThread
        mov @dwContinueStatus, eax
    .elseif eax == EXIT_THREAD_DEBUG_EVENT
        invoke EventExitThread
        mov @dwContinueStatus, eax
    .elseif eax == EXIT_PROCESS_DEBUG_EVENT
        invoke EventExitProcess
        mov @dwContinueStatus, eax   
    .elseif eax == LOAD_DLL_DEBUG_EVENT
        invoke EventLoadDll
        mov @dwContinueStatus, eax
    .elseif eax == UNLOAD_DLL_DEBUG_EVENT
        invoke EventUnLoadDll
        mov @dwContinueStatus, eax
    .elseif eax == OUTPUT_DEBUG_STRING_EVENT
        invoke EventOutputDebugString
        mov @dwContinueStatus, eax
    .elseif eax == RIP_EVENT
        invoke EventRip
        mov @dwContinueStatus, eax
    .endif

    mov eax, @dwContinueStatus
    cmp eax, E_ERR
    je debug_exit            ; 如果处理出错，退出调试循环


    
    ; 提交事件处理结果
    invoke ContinueDebugEvent,
        g_DebugEv.dwProcessId,
        g_DebugEv.dwThreadId,
        @dwContinueStatus
    
    ;继续循环    
    invoke  RtlZeroMemory,offset g_DebugEv, size g_DebugEv
    jmp debug_loop
    
create_failed:
    ;LogMsg(LOG_ERROR, "StartDebug CreateProcess ErrCode:%08x\n\n", GetLastError());
    invoke GetLastError
    invoke LogMsg, LOG_ERROR, offset szLogCreateError,eax
	mov eax, E_ERR
    ret
    
debug_exit:
    ; 清理资源
    invoke CloseHandle, @pi.hThread
    invoke CloseHandle, @pi.hProcess
    mov eax, E_OK
    ret
StartDebug endp

; 附加调试
AttachDebug proc dwProcessId:DWORD
	
	
	ret

AttachDebug endp

; 分离调试
DetachDebug proc dwProcessId:DWORD
    ret
DetachDebug endp

; 停止调试
StopDebug proc
    mov eax, E_OK
    ret
StopDebug endp



end	

