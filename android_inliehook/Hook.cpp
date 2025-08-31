#include "Hook.h"

#include <string.h>
#include <sys/mman.h>
#include <stdio.h>

#define SMALL_TRIMPLINE 16
#define PAGE_SZIE 0x1000
#define PAGE_BASE(x) (void*)((uint64_t)(x) & (~0xFFF))

extern "C"{
    void Saved_OldCode();
    void RetDst_addr();
    void SmallTramplie();
    void Trimpline();
    void g_pfnCallback();
}

bool InstallHook(void* pDestAddr, void* pfnCallback, void** ppfnNewDstAddr)
{
    //0. 修改内存属性
    int nRet = mprotect(PAGE_BASE(pDestAddr), PAGE_SZIE, PROT_READ|PROT_WRITE|PROT_EXEC);
    if (nRet < 0)
    {
        perror("修改目标函数内存属性失败");
        return false;
    }

    nRet = mprotect(PAGE_BASE(Saved_OldCode), PAGE_SZIE, PROT_READ|PROT_WRITE|PROT_EXEC);
    if (nRet < 0)
    {
        perror("修改跳板内存属性失败");
        return false;
    }
    
    //1. 保存原函数指令
    memcpy((void*)Saved_OldCode, (void*)pDestAddr, SMALL_TRIMPLINE);
    auto pRetAddr = (char*)pDestAddr+SMALL_TRIMPLINE;
    memcpy((void*)RetDst_addr, (void*)&pRetAddr, sizeof(pDestAddr));
    memcpy((void*)g_pfnCallback, (void*)&pfnCallback, sizeof(pfnCallback));

    //2. 修改原函数头，跳转到跳板
    memcpy((void*)pDestAddr, (void*)SmallTramplie, SMALL_TRIMPLINE);
    void* pTrimpline = (void*)Trimpline;
    memcpy((void*)((char*)pDestAddr+8), (void*)&pTrimpline, sizeof(pTrimpline));

    //3. 将原函数新的地址返回
    *ppfnNewDstAddr = (void*)Saved_OldCode;

    return true;
}


bool UninstallHook(void* pDestAddr, void* pfnNewDstAddr)
{
    return false;
}