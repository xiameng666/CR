.386
.model flat, stdcall
option casemap:none

include windows.inc
include kernel32.inc
include user32.inc
include msvcrt.inc
include shell32.inc  

includelib kernel32.lib
includelib user32.lib
includelib msvcrt.lib
includelib shell32.lib 

StartDebug PROTO :LPCTSTR, :LPCTSTR

.data
szInvalidCmd db "Invalid Commandline", 0dh, 0ah, 0
argc dd 0
argv dd 256 dup(0)  ; 存储命令行参数指针

.code

main proc

	;int 3 
	;命令行怎么解析
	;invoke GetCommandLine
    ;invoke crt___getmainargs, addr argc, addr argv, 0, 0, 0
    ;invoke CommandLineToArgvW,eax,offset argc
    ;mov argv,eax
    
    ;先不管参数了 = =
	jmp fortest
    
    ;if (argc < 2)
    mov eax,argc
    cmp eax,2
    jge params_valid
    
    ;失败
    invoke crt__cprintf,offset szInvalidCmd
    invoke LocalFree,offset argv
    jmp exit_proc
    
params_valid:

	mov eax,argv
	mov ebx,[eax+4] ; argv[1]
	mov ecx,[eax+8] ; argv[2]
	
fortest:
	invoke StartDebug, ebx, ecx
	
exit_proc:
	invoke ExitProcess, eax
	ret

main endp


start:
 
	invoke main
	
end start