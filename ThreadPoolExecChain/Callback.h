#pragma once
#include <windows.h>
#include <cstdint>

#define WORK_ITEM_COUNT 4
#define MAX_ARGC 4

typedef struct WorkItemContext {
    FARPROC func;           // Function pointer
    void* retAddress;       // Return address to simulate stack return
    uint64_t argc;          // Argument count
    void* args[MAX_ARGC];   // Arguments (up to 4)
} WorkItemContext;


extern "C" void CALLBACK FirstCallback(PTP_CALLBACK_INSTANCE, PVOID, PTP_WORK);

extern "C" void CALLBACK GenericCallback(...);
extern "C" void CALLBACK LastCallback(...);
