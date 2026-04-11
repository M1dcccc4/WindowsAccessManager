#pragma once
#include <iostream>
#include <string>
#ifdef _DEBUG
inline void Debug(std::string text) {
	std::cout << "[DEBUG] " << text << std::endl;
}
#else 
#define Debug(text) 
#endif // _DEBUG
