#pragma once
#include <functional>
#include <vector>

typedef std::function<void(const std::vector<uint8_t> &&)> on_write_data;
typedef std::function<void(size_t id)> on_session_closed;

struct tcp_session_callbacks
{
	on_write_data _on_write_data;
	on_session_closed _on_session_closed;
};

typedef std::shared_ptr<tcp_session_callbacks> ptcp_session_callbacks;
