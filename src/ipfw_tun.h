#pragma once
#include "tun.h"
#include "ipfw.h"
#include <thread>

class ipfw_tun
{
public:
	// TODO use DI
	ipfw_tun(const std::string & fw_https_host, uint16_t fw_https_port,
			 const std::string & fw_http_host, uint16_t fw_http_port, uint64_t tun_fd);
	void start();
	void stop();
	boost::asio::io_service & io();
private:
	boost::asio::io_service _io;
	ipfw_callbacks _ipfw_callbacks;
	std::thread _thread;
	std::shared_ptr<tun> _tun;
	std::shared_ptr<ipfw> _ipfw;
	std::shared_ptr<tun> create_tun(uint64_t tun_fd);
	std::shared_ptr<ipfw> create_ipfw(const std::string & fw_https_host, uint16_t fw_https_port,
									  const std::string & fw_http_host, uint16_t fw_http_port);
};
