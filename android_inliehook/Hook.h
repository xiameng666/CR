
bool InstallHook(void* pDestAddr, void* pfnCallback, void** ppfnNewDstAddr);
bool UninstallHook(void* pDestAddr, void* pfnNewDstAddr);