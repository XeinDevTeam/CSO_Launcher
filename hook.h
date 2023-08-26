#pragma once

#include <Windows.h>

typedef float vec_t;
typedef float vec2_t[2];
typedef float vec3_t[3];
typedef int (*pfnUserMsgHook)(const char* pszName, int iSize, void* pbuf);

#include <wrect.h>
#include <cdll_int.h>

void Hook(HMODULE hModule);
void Unhook();