#pragma once
#include <string>
#include <unistd.h>

struct ipfw_args
{
	std::string _http_fw_host;
	uint16_t _http_fw_port;

	std::string _https_fw_host;
	uint16_t _https_fw_port;
};
