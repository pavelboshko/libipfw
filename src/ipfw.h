#pragma once
#include <map>
#include <thread>
#include "ipfw_args.h"
#include "ipfw_callbacks.h"
#include "tcp_session.h"

class ipfw
{
public:
	ipfw(const ipfw_callbacks & callbacks, const ipfw_args & args);
	~ipfw();
	void start();
	void stop();
	void handle_ip(const std::vector<uint8_t> &&);
private:
	void restart_wdog();

	const ipfw_callbacks _callbacks;
	const ipfw_args _args;
	std::map<session_id, ptcp_session> _tcp_sessions;
	boost::asio::io_service _io;
	boost::asio::deadline_timer _wdog_timer;
	std::thread _io_thread;

};
