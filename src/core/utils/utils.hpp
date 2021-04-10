#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

#include "../log.hpp"

int change_cwd_to_selfpath();
int file_to_str(const std::string& filename, std::string& buffer);
int file_to_str(const char* filename, std::string& buffer);
int greyscale_to_bmp(const char* filename, int w, int h, const unsigned char* buffer);
void wstrcpy(wchar_t* dest, const wchar_t* source, uint max);
size_t ucs2wcs(wchar_t* dest, const unsigned char* source, uint max);
int get_cpu_model(char* stringbuffer);
unsigned long long get_sys_memory();

#endif
