//#define __STDC_WANT_LIB_EXT1_ 1
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <iostream>
/*
This program is a minimal sample of converting a wide string from uppercase to lowercase.
*/
inline wchar_t* copyw(const wchar_t* source, const size_t size) {
	wchar_t* buffer = new wchar_t[size];
	for(size_t i = 0; i < size; i++)
		buffer[i] = source[i];
	return buffer;
}
inline wchar_t* copyw_s(const wchar_t* source, size_t max_size) {
size_t size = wcsnlen(source, max_size);
	return copyw(source, size + 1);
}
inline wchar_t* copyw(const wchar_t* source) {
	size_t size = wcslen(source);
	return copyw(source, size + 1);
}
int main() {
	setlocale(LC_ALL, "");
	wchar_t* src = copyw(L"ÄáioTéçÇ");
	for(size_t i = 0, size = wcslen(src); i < size; i++)
		src[i] = towlower(src[i]);
	std::wcout << src << std::endl;
	delete[] src;
}
// compile with: g++ -g -fsanitize=leak -o tolower tolower.cpp
