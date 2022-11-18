#include <iostream>
#include <locale>
#include <string>

int main(){
	std::ios_base::sync_with_stdio(false);
	std::locale::global (std::locale (""));
	std::wcin.imbue( std::locale("") );
	std::wcout.imbue( std::locale("") );
	std::wstring buffer;
	while(std::wcin >> buffer) {
		std::wcout << buffer << std::endl;
	}
	return 0;
}