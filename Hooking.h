#pragma once

#define CALL_NEAR32 0xE8U
#define JMP_NEAR32 0xE9U
#define NOP 0x90U

struct CallHook {
	BYTE bOriginalCode[5];
	PBYTE pPlace;
	void* pOriginal;
	void* hook;

	void initialize(DWORD place, void *hookToInstall = NULL);
	void installHook(void* hookToInstall = NULL);
	void releaseHook();
};

struct StompHook {
	BYTE bOriginalCode[15];
	BYTE bCountBytes;
	PBYTE pPlace;
	void* hook;
	bool jump;

	void initialize(DWORD place, void *hookToInstall = NULL, BYTE countBytes = 5, bool useJump = true);
	void installHook(void* hookToInstall = NULL);
	void releaseHook();
};

void HookInstall(DWORD address, DWORD hookToInstall, int bCountBytes=5);

enum eCallPatcher
{
	PATCH_CALL,
	PATCH_JUMP,
	PATCH_NOTHING
};

void _patch(void* pAddress, DWORD data, DWORD iSize);
void _nop(void* pAddress, DWORD size);
void _call(void* pAddress, DWORD data, eCallPatcher bPatchType);

#define patch(a, v, s) _patch((void*)(a), (DWORD)(v), (s))
#define nop(a, v) _nop((void*)(a), (v))
#define call(a, v, bAddCall) _call((void*)(a), (DWORD)(v), (bAddCall))

#include <forward_list>

extern std::forward_list<void (*)()> g_initFunctions;

class InitFunction
{
public:

	InitFunction(void (* callback)())
	{
		g_initFunctions.push_front(callback);
	}

	static void RunFunctions();
};

template<typename T>
void call_member(DWORD address, T target, eCallPatcher patchType)
{
	DWORD dwFunc = 0;

	__asm
	{
		mov eax, target
			mov dwFunc, eax
	}

	call(address, dwFunc, patchType);
}

#define CALL_NO_ARGUMENTS(addr) ((void(*)())addr)()
#define EAXJMP(a) { _asm mov eax, a _asm jmp eax }
#define WRAPPER __declspec(naked)