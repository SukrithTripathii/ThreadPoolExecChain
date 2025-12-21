#include "PatternSearch.h"

PVOID g_SaveRaxInRbx = NULL;

void* find_pattern_masked(const void* haystack, size_t haystack_size,
    const unsigned char* pattern,
    const unsigned char* mask,
    size_t pattern_len) {
    const unsigned char* mem = (const unsigned char*)haystack;

    for (size_t i = 0; i <= haystack_size - pattern_len; i++) {
        size_t j = 0;
        for (; j < pattern_len; j++) {
            if (mask[j] == 0x00 && mem[i + j] != pattern[j])
                break;
        }
        if (j == pattern_len)
            return (void*)(mem + i);
    }

    return nullptr;
}


void* find_pattern_in_module(HMODULE hmod,
    const unsigned char* pattern,
    const unsigned char* mask,
    size_t pattern_len) {
    if (!hmod) return NULL;

    // Get DOS and NT headers
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)hmod;
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)((BYTE*)hmod + dos->e_lfanew);

    // Locate .text section
    IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
    for (int i = 0; i < nt->FileHeader.NumberOfSections; i++, sec++) {
        if (memcmp(sec->Name, ".text", 5) == 0) {
            BYTE* start = (BYTE*)hmod + sec->VirtualAddress;
            DWORD size = sec->Misc.VirtualSize;

            // Perform masked pattern scan
            for (DWORD offset = 0; offset <= size - pattern_len; offset++) {
                size_t j = 0;
                for (; j < pattern_len; j++) {
                    if (mask[j] == 0x00 && start[offset + j] != pattern[j])
                        break;
                }
                if (j == pattern_len)
                    return (void*)(start + offset);
            }

            break; // only .text
        }
    }

    return NULL;
}

void* find_address_to_push() {
    if (g_SaveRaxInRbx != 0) {
        return g_SaveRaxInRbx;
    }

    /*

       180152075 e8 ee af        CALL       FUN_18015d068                    undefined FUN_18015d068()
                 00 00
       18015207a 48 89 03        MOV        qword ptr [RBX],RAX
       18015207d b8 01 00        MOV        EAX,0x1
                 00 00
       180152082 eb 06           JMP        LAB_18015208a
                             LAB_180152084                                   XREF[3]:     180152060(j), 180152066(j),
                                                                                          18015206c(j)
       180152084 48 89 02        MOV        qword ptr [RDX],RAX
       180152087 48 89 03        MOV        qword ptr [RBX],RAX
                             LAB_18015208a                                   XREF[1]:     180152082(j)
       18015208a 48 83 c4 20     ADD        RSP,0x20
       18015208e 5b              POP        RBX
       18015208f c3              RET
       180152090 cc              ??         CCh

    */

    // Assembly pattern to match in memory

    const unsigned char pattern[] = {
    0xE8, 0x00, 0x00, 0x00, 0x00,    // CALL (wildcard offset)
    0x48, 0x89, 0x03,
    0xB8, 0x01, 0x00, 0x00, 0x00,
    0xEB, 0x06,
    0x48, 0x89, 0x02,
    0x48, 0x89, 0x03,
    0x48, 0x83, 0xC4, 0x20,
    0x5B,
    0xC3
    };

    // MASK - 0x00 strict match, 0xFF match everything
    const unsigned char mask[] = {
        0x00, 0xFF, 0xFF, 0xFF, 0xFF,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00,
        0x00
    };

    HMODULE hMod = LoadLibraryA("wininet.dll");
    if (!hMod) {
        printf("Failed to load wininet.dll: %lu\n", GetLastError());
        return nullptr;
    }

    void* address = find_pattern_in_module(hMod, pattern, mask, sizeof(pattern));

    if (!address) {
        printf("Pattern not found in wininet.dll\n");
        return nullptr;
    }

    // Adjust the address to point to the instruction after the CALL
    g_SaveRaxInRbx = (void*)((UINT64)address + 5);
    return g_SaveRaxInRbx;
}
