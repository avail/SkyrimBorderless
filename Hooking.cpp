#include "hooking.h"

void CallHook::initialize(DWORD place, void *hookToInstall)
{
	pPlace = (PBYTE)place;
	memcpy(bOriginalCode, pPlace, sizeof(bOriginalCode));
	pOriginal = pPlace + sizeof(bOriginalCode) + *(ptrdiff_t*) (bOriginalCode + 1);
	hook = hookToInstall;
}

void CallHook::installHook(void *hookToInstall)
{
	if (hookToInstall) hook = hookToInstall;
	if (hook) {
		*(ptrdiff_t*) (pPlace + 1) = (PBYTE) hook - pPlace - 5;
	}
}

void CallHook::releaseHook()
{
	//memcpy(pPlace + 1, bOriginalCode + 1, sizeof(bOriginalCode) - 1);
	*(ptrdiff_t*) (pPlace + 1) = (PBYTE) pOriginal - pPlace - 5;
}

void StompHook::initialize(DWORD place, void *hookToInstall, BYTE countBytes, bool useJump)
{
	pPlace = (PBYTE)place;
	bCountBytes = countBytes < sizeof(bOriginalCode) ? countBytes : sizeof(bOriginalCode);
	memcpy(bOriginalCode, pPlace, bCountBytes);
	hook = hookToInstall;
	jump = useJump;
}

void StompHook::installHook(void *hookToInstall)
{
	if (hookToInstall) hook = hookToInstall;
	if (hook) {
		memset(pPlace, NOP, bCountBytes);
		pPlace[0] = jump ? JMP_NEAR32 : CALL_NEAR32;
		*(ptrdiff_t*) (pPlace + 1) = (PBYTE) hook - pPlace - 5;
	}
}

void StompHook::releaseHook()
{
	memcpy(pPlace, bOriginalCode, bCountBytes);
}

void HookInstall(DWORD address, DWORD hookToInstall, int bCountBytes)
{
	PBYTE pPlace = (PBYTE)address;
	memset(pPlace, NOP, bCountBytes);
	pPlace[0] = CALL_NEAR32;
	*(ptrdiff_t*) (pPlace + 1) = (PBYTE) hookToInstall - pPlace - 5;
}

void _patch(void* pAddress, DWORD data, DWORD iSize)
{
	switch(iSize)
	{
	case 1: *(BYTE*)pAddress = (BYTE)data;
		break;
	case 2: *(WORD*)pAddress = (WORD)data;
		break;
	case 4: *(DWORD*)pAddress = (DWORD)data;
		break;
	}
}

void _nop(void* pAddress, DWORD size)
{
	memset(pAddress, 0x90, size);
	return;

	DWORD dwAddress = (DWORD)pAddress;
	if ( size % 2 )
	{
		*(BYTE*)pAddress = 0x90;
		dwAddress++;
	}
	if ( size - ( size % 2 ) )
	{
		DWORD sizeCopy = size - ( size % 2 );
		do
		{
			*(WORD*)dwAddress = 0xFF8B;
			dwAddress += 2;
			sizeCopy -= 2;
		}
		while ( sizeCopy );
	}
}

void _call(void* pAddress, DWORD data, eCallPatcher bPatchType)
{
	switch ( bPatchType )
	{
	case PATCH_JUMP:
		*(BYTE*)pAddress = (BYTE)0xE9;
		break;

	case PATCH_CALL:
		*(BYTE*)pAddress = (BYTE)0xE8;
		break;

	default:
		break;
	}

	*(DWORD*)((DWORD)pAddress + 1) = (DWORD)data - (DWORD)pAddress - 5;
}

void _charptr(void* pAddress, const char* pChar)
{
	*(DWORD*)pAddress = (DWORD)pChar;
}

std::forward_list<void (*)()> g_initFunctions;

void InitFunction::RunFunctions()
{
	for (auto& it : g_initFunctions)
	{
		(*it)();
	}
}