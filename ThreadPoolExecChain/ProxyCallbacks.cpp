#include "Callback.h"
#include "PatternSearch.h"
#include "ProxyCallbacks.h"

#include <psapi.h>

uint64_t ProxyPooling(WorkItemContext* ctx, UINT chain) {

    // Alright, this is the address of the final backward proxy
    void* addressToPush = find_address_to_push();

    // This is the array of work items (1 x callback)
    WorkItemContext* workItems = (WorkItemContext*)malloc(sizeof(WorkItemContext) * WORK_ITEM_COUNT);
    if (!workItems) {
        return ERROR_CODE;
    }

    memset(workItems, 0, sizeof(WorkItemContext) * WORK_ITEM_COUNT);

    // This is stuff for the callback itself
    // https://github.com/aahmad097/AlternativeShellcodeExec

    if (chain == 0) {

        HDC dc = GetDC(NULL);
        
        // Set up work item 0 - In this case EnumThreadWindows
        workItems[0].func = (FARPROC)&EnumThreadWindows;
        workItems[0].retAddress = NULL;
        workItems[0].argc = 3;
        workItems[0].args[0] = (void*)0;
        workItems[0].args[1] = (void*)&GenericCallback;
        workItems[0].args[2] = (void*)0;

        // Set up work item 1 - EnumObjects
        workItems[1].func = (FARPROC)&EnumObjects;
        workItems[1].retAddress = NULL;
        workItems[1].argc = 4;
        workItems[1].args[0] = (void*)dc;
        workItems[1].args[1] = (void*)OBJ_BRUSH;
        workItems[1].args[2] = (void*)&LastCallback;
        workItems[1].args[3] = (void*)NULL;
    }
    else if (chain == 1) {
        
        HDC dc = GetDC(NULL);

        // Set up work item 0 - In this case K32EnumPageFilesW
        workItems[0].func = (FARPROC)&K32EnumPageFilesW;
        workItems[0].retAddress = NULL;
        workItems[0].argc = 2;
        workItems[0].args[0] = (void*)&GenericCallback;
        workItems[0].args[1] = (void*)0;

        // Set up work item 1 - EnumFontFamiliesW
        workItems[1].func = (FARPROC)&EnumFontFamiliesW;
        workItems[1].retAddress = NULL;
        workItems[1].argc = 4;
        workItems[1].args[0] = (void*)dc;
        workItems[1].args[1] = (void*)NULL;
        workItems[1].args[2] = (void*)&LastCallback;
        workItems[1].args[3] = (void*)NULL;

    }
    else {
        PVOID lpContext;
        INIT_ONCE g_InitOnce = INIT_ONCE_STATIC_INIT;

        // Set up work item 0 - In this case InitOnceExecuteOnce
        workItems[0].func = (FARPROC)&InitOnceExecuteOnce;
        workItems[0].retAddress = NULL;
        workItems[0].argc = 4;
        workItems[0].args[0] = (void*)&g_InitOnce;
        workItems[0].args[1] = (void*)&GenericCallback;
        workItems[0].args[2] = (void*)0;
        workItems[0].args[3] = (void*)&lpContext;

        // Set up work item 1 - EnumUILanguagesW
        workItems[1].func = (FARPROC)&EnumUILanguagesW;
        workItems[1].retAddress = NULL;
        workItems[1].argc = 3;
        workItems[1].args[0] = (void*)&LastCallback;
        workItems[1].args[1] = (void*)MUI_LANGUAGE_ID;
        workItems[1].args[2] = (void*)NULL;

    }

    // Set up work item 2 - Target function + proxy frame
    workItems[2].func = (FARPROC)&ctx->func;
    workItems[2].retAddress = addressToPush;
    workItems[2].argc = ctx->argc;
    memcpy(workItems[2].args, ctx->args, 8 * ctx->argc);

    // Create a work object
    PTP_WORK work = CreateThreadpoolWork(FirstCallback, workItems, nullptr);
    if (!work) {
        printf("Failed CreateThreadpoolWork: %lu\n", GetLastError());
        free(workItems);
        return ERROR_CODE;
    }

    // Submit it
    SubmitThreadpoolWork(work);

    // Wait for completion
    WaitForThreadpoolWorkCallbacks(work, FALSE);
    CloseThreadpoolWork(work);

    // Voilat, we got the return value where we wanted
    uint64_t rax = (uint64_t)(workItems[0].retAddress);

    // Just free this
    free(workItems);
    
    return rax;

}


uint64_t ProxyLoadLibrary(const char* library, unsigned int chain) {

    // We wanna proxy LoadLibraryA
    FARPROC targetFunction = reinterpret_cast<FARPROC>(LoadLibraryA);

    if (!targetFunction) {
        printf("Failed to load function: %lu\n", GetLastError());
        return ERROR_CODE;
    }

    unsigned char* libName = (unsigned char*)malloc(256);
    if (!libName) {
        printf("Memory allocation failed\n");
        return ERROR_CODE;
    }
    snprintf(reinterpret_cast<char*>(libName), 256, library);

    WorkItemContext* ctx = (WorkItemContext*)malloc(sizeof(WorkItemContext));
    if (ctx == NULL) {
        printf("[-] Failed to allocate context structure\n");
        return ERROR_CODE;
    }
    memset(ctx, 0, sizeof(WorkItemContext));

    ctx->func = targetFunction;
    ctx->argc = 1;
    ctx->args[0] = (void*)libName;

    // We proxy LoadLibraryA via the chain
    uint64_t ret = ProxyPooling(ctx, chain);
    free(ctx);

    return ret;

}

uint64_t ProxyMessageBox(const char* titleText, const char* msgText, unsigned int chain) {

    // We wanna proxy MessageBoxA
    FARPROC targetFunction = reinterpret_cast<FARPROC>(MessageBoxA);

    if (!targetFunction) {
        printf("Failed to load function: %lu\n", GetLastError());
        return ERROR_CODE;
    }

    unsigned char* title = (unsigned char*)malloc(256);
    unsigned char* msg = (unsigned char*)malloc(256);
    if (!title || !msg) {
        printf("Memory allocation failed\n");
        return ERROR_CODE;
    }
    snprintf(reinterpret_cast<char*>(title), 256, titleText);
    snprintf(reinterpret_cast<char*>(msg), 256, msgText);

    WorkItemContext* ctx = (WorkItemContext*)malloc(sizeof(WorkItemContext));
    if (ctx == NULL) {
        printf("[-] Failed to allocate context structure\n");
        return ERROR_CODE;
    }
    memset(ctx, 0, sizeof(WorkItemContext));

    ctx->func = targetFunction;
    ctx->argc = 4;
    ctx->args[0] = (void*)NULL;
    ctx->args[1] = (void*)msg;
    ctx->args[2] = (void*)title;
    ctx->args[3] = (void*)(MB_OK | MB_ICONINFORMATION);

    // We proxy MessageBoxA via the chain
    uint64_t ret = ProxyPooling(ctx, chain);
    free(ctx);
    return ret;
}
