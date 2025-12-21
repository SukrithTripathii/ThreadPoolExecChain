#pragma once
#include <stdint.h>

#define ERROR_CODE 0xCAFEBABECAFEBABE

uint64_t ProxyLoadLibrary(const char* dllName, unsigned int chain);
uint64_t ProxyMessageBox(const char* title, const char* message, unsigned int chain);
