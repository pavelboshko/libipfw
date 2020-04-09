
#pragma once

#include "format.h"
#include <iostream>

template <typename ...Args>
void ExecuteCommand(Args&&... args) {
	std::string command = Format(std::forward<Args>(args)...);
	std::cerr << "ExecuteCommand: " << command << "\n";
	system(command.c_str());
}


std::string ExecuteCommand(const char* cmd) {
	std::cerr << "ExecuteCommand: " << cmd << "\n";
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}
