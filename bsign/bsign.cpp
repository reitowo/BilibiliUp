#include <cassert>
#include <iostream>
#include <string>
#include <Windows.h>

int main(int argc, const char** argv)
{
	if (argc != 2)
		return -1;
	 
	SetDllDirectoryW(LR"(C:\Program Files (x86)\bililive\ugc_assistant\2.3.0.1063)");
	auto bilisec = LoadLibraryW(LR"(bililive_secret.dll)");
	void (*sign_func)(int*, const char*, int) = (decltype(sign_func))((uintptr_t)bilisec + 0x3F1D8);
	 
	assert(sign_func != nullptr);
	
	int signature[4] = {0}; 
	std::string str(argv[1]);
	sign_func(signature, str.data(), str.size());
	
	std::string sign;
	uint8_t* sign_c = reinterpret_cast<uint8_t*>(signature);
	for (int i = 0; i < 16; ++i)
	{
		char c[3] = {0};
		sprintf(c, "%02x", sign_c[i]);
		sign += c;
	}

	std::cout << sign;

	return 0;
}
