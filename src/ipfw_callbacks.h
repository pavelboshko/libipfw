#pragma once
#include <string>
#include <vector>
#include <functional>

class ipfw_callbacks
{
public:
	typedef std::function<void(const std::vector<uint8_t> &&)> on_ip_packet_t;
	on_ip_packet_t _on_ip_packet;
};
