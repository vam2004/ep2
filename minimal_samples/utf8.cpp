#include <iostream>
#include <locale>
#include <string>
#include <codecvt>
/*
This program is a minimal sample of converting utf8 to wchar_t.
References:
https://cplusplus.com/forum/beginner/107125/
https://stackoverflow.com/questions/4324542/what-is-the-windows-equivalent-for-en-us-utf-8-locale
https://stackoverflow.com/questions/7232710/convert-between-string-u16string-u32string/7235204#7235204
https://stackoverflow.com/questions/7232710/convert-between-string-u16string-u32string/7235204#7235204
*/
template<typename charT>
using converter_t = std::wstring_convert<std::codecvt_utf8<charT>, charT>;
template<typename charT>
void print_utf8_code(const char* source, std::basic_ostream<charT>& output) {
	while(*source != 0) // there is at least one '\0' in source
		output << (int) *source++ << std::endl; // print each code point until '\0'
}
template<typename source_t, typename charT>
void conv_word(const source_t source, std::basic_string<charT>* into) {
	converter_t<charT> converter; // the converter
	std::basic_string<charT> converted = converter.from_bytes(source); // convert the source and store result into stack
	into->swap(converted); // move the result from stack to "into"
}
bool read_word(std::istream& source, std::wstring* obuffer) {
	std::string ibuffer; // a temporary buffer for input
	if(!(source >> ibuffer)) // read utf8 string
		return false; // error when reading
	#ifdef SHOW_CODEPOINTS
	print_utf8_code(ibuffer.c_str(), std::wcout); // print code points
	#endif
	conv_word(ibuffer, obuffer); // convert the input and put into "obuffer" 
	return true; // sucess
}
void simple_test(){
	const char* source_word = "d\xc3\xa9j\xc3\xa0 vu"; // "dèjá vu" encoded in utf8
	#ifdef SHOW_CODEPOINTS
	print_utf8_code(source_word, std::wcout); // print code points
	#endif
	std::wstring converted; // the buffer to store conversion
	conv_word(source_word, &converted); // convert the utf8 to std::wstring
	std::wcout << converted << std::endl; // print result
}
void echo_test () {
	std::cin.imbue( std::locale("") ); // set the locale of cin to actual locale
	std::wstring readed;
	while(read_word(std::cin, &readed)) {
		std::wcout << readed << std::endl; // print result
	}
}
int main(){
	std::locale::global (std::locale (""));
	simple_test();
	echo_test();
	return 0;
}


