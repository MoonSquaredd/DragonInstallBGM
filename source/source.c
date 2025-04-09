#include <windows.h>
#include "GG.h"

HMODULE exe;
BYTE *target;
BYTE *patchLocation;
BYTE *StopBGM;
BYTE *RequestBGM;
BYTE *DIBGM_Back;

void DI_BGM() { // made it in assembly because of parameters in register
	__asm__ (
		".intel_syntax noprefix\n\t"
		"push eax\n\t"
		"call [%0]\n\t" // stops the current music
		"mov eax,0x12\n\t"
		"push 0x1\n\t"
		"call [%1]\n\t" // starts ride the fire
		"add esp,4\n\t"
		"pop eax\n\t"
		"push esi\n\t" // original overwritten code
		"mov esi,[ebp+0x8]\n\t"
		"jmp [%2]\n\t" // goes back
		".att_syntax prefix\n\t"
		:
		: "m"(StopBGM), "m"(RequestBGM), "m"(DIBGM_Back)
	);
}

void Patch() {
	DWORD oldProtect;
	char jump[7] = {0xE9};
	*(DWORD*)(jump+1) = (DWORD)DI_BGM - ((DWORD)patchLocation + 5);
	jump[5] = 0x90;
	jump[6] = 0x90;
	VirtualProtect(patchLocation,7,PAGE_EXECUTE_READWRITE,&oldProtect);
	memcpy(patchLocation,jump,7);
	VirtualProtect(patchLocation,7,oldProtect,&oldProtect);
	return;
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved) {
	if (reason == DLL_PROCESS_ATTACH) {
		exe = GetModuleHandle(NULL);
		target = (BYTE*)exe;
		patchLocation = target+0x251570;
		StopBGM = target+0x11EB50;
		RequestBGM = target+0x11EC40;
		DIBGM_Back = patchLocation+7;
		Patch();
	}
	return TRUE;
}
