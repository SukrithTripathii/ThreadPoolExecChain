#pragma once
#pragma once
#include <windows.h>
#include <stdio.h>

void* find_pattern_masked(const void* haystack, size_t haystack_size, const unsigned char* pattern, const unsigned char* mask, size_t pattern_len);
void* find_pattern_in_module(HMODULE hmod, const unsigned char* pattern, const unsigned char* mask, size_t pattern_len);

void* find_address_to_push();