#include <stdio.h>
#include "ProxyCallbacks.h"
#include "Rand.h"

int main() {

    uint64_t returnValue;
    unsigned int chain = RandintMod3();

    returnValue = ProxyMessageBox("Trying to load something here!", "What? Well... urlmon.dll, of course!", chain++ % 3);
    if (returnValue == ERROR_CODE) {
        printf("[-] Error Proxying MessageBoxA");
        return 1;
    }
    
    printf("[+] Returned from MessageBoxA: 0x%llx\n", returnValue);

    returnValue = ProxyLoadLibrary("urlmon.dll", chain++ % 3);
    if (returnValue == ERROR_CODE) {
        printf("[-] Error Proxying LoadLibraryA");
        return 1;
    }

    printf("[+] Returned from LoadLibraryA: 0x%llx\n", returnValue);

    char message[64];
    sprintf_s(message, "Address is: 0x%llx", (unsigned long long)returnValue);
    returnValue = ProxyMessageBox("Address of urlmon.dll ", message, chain++ % 3);
    if (returnValue == ERROR_CODE) {
        printf("[-] Error Proxying MessageBoxA");
        return 1;
    }

    printf("[+] Returned from MessageBoxA: 0x%llx\n", returnValue);

    return 0;
}
