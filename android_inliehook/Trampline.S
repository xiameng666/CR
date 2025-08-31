.arch armv8-a

.global Saved_OldCode
.global RetDst_addr
.global SmallTramplie
.global Trimpline
.global g_pfnCallback

.text
Trimpline:
    //调用C函数
    ldr x17, g_pfnCallback
    br x17 

Saved_OldCode: // 保存原函数的指令
    .space 0x10, 0

SmallTramplie:
Trampline_BrDst:
    ldr x17, RetDst_addr
    br x17
RetDst_addr: 
    .space 8, 0

old_version:
    adr x4, g_pfnCallback
    LDXR x0, [x4]
    add x0, x0, 1
    stlxr w2, x0, [x4]
    cbz w2, old_version

.data
    g_pfnCallback: .space 8, 0

