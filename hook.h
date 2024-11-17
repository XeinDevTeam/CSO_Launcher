#pragma once

#include <Windows.h>

typedef float vec_t;
typedef float vec2_t[2];
typedef float vec3_t[3];
typedef int (*pfnUserMsgHook)(const char* pszName, int iSize, void* pbuf);

#include <wrect.h>
#include <cdll_int.h>

void Hook(HMODULE hEngineModule, HMODULE hFileSystemModule);
void Unhook();

#define CreateHook(funcType, returnType, funcName, ...) \
returnType (funcType* g_pfn##funcName)(__VA_ARGS__); \
returnType funcType Hook_##funcName(__VA_ARGS__)

#define CreateHookClassType(returnType,funcName,classType, ...) \
returnType (__thiscall* g_pfn##funcName)(classType*ptr, __VA_ARGS__); \
returnType __fastcall Hook_##funcName(classType*ptr, int reg, __VA_ARGS__)

#define CreateHookClass(returnType, funcName, ...) CreateHookClassType(returnType, funcName, void, __VA_ARGS__)