#pragma once

#include <windows.h>

// copied from metahook
struct hook_s
{
	void* pOldFuncAddr;
	void* pNewFuncAddr;
	void* pClass;
	int iTableIndex;
	int iFuncIndex;
	HMODULE hModule;
	const char* pszModuleName;
	const char* pszFuncName;
	struct hook_s* pNext;
	void* pInfo;
};

typedef struct hook_s hook_t;

void FreeAllHook(void);

DWORD GetModuleBase(HMODULE hModule);
DWORD GetModuleSize(HMODULE hModule);

hook_t* FindInlineHooked(void* pOldFuncAddr);
hook_t* FindVFTHooked(void* pClass, int iTableIndex, int iFuncIndex);
hook_t* FindIATHooked(HMODULE hModule, const char* pszModuleName, const char* pszFuncName);
BOOL UnHook(hook_t* pHook);
hook_t* InlineHook(void* pOldFuncAddr, void* pNewFuncAddr, void*& pCallBackFuncAddr);
hook_t* InlineHookFromCallOpcode(void* pOldFuncAddr, void* pNewFuncAddr, void*& pCallBackFuncAddr, void*& pFuncAddr);
hook_t* VFTHook(void* pClass, int iTableIndex, int iFuncIndex, void* pNewFuncAddr, void*& pCallBackFuncAddr);
hook_t* IATHook(HMODULE hModule, const char* pszModuleName, const char* pszFuncName, void* pNewFuncAddr, void*& pCallBackFuncAddr);
hook_t* IATHookOrdinal(HMODULE hModule, const char* pszModuleName, int ordinal, void* pNewFuncAddr, void*& pCallBackFuncAddr);
void *GetClassFuncAddr(...);
void WriteDWORD(void *pAddress, DWORD dwValue);
DWORD ReadDWORD(void *pAddress);
DWORD WriteMemory(void *pAddress, BYTE *pData, DWORD dwDataSize);
DWORD ReadMemory(void *pAddress, BYTE *pData, DWORD dwDataSize);
DWORD FindPattern(PCHAR pattern, PCHAR mask, DWORD start, DWORD end, DWORD offset = 0);
DWORD FindPattern(PCHAR pattern, DWORD start, DWORD end, DWORD offset = 0, DWORD refNumber = 1);
DWORD FindPush(DWORD start, DWORD end, PCHAR Message, DWORD refNumber = 1);