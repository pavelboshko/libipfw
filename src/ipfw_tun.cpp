#include "ipfw_tun.h"

ipfw_tun::ipfw_tun(const std::string & fw_https_host, uint16_t fw_https_port,
				   const std::string & fw_http_host, uint16_t fw_http_port, uint64_t tun_fd) :
	_tun(create_tun(tun_fd)), _ipfw(create_ipfw(fw_https_host, fw_https_port, fw_http_host, fw_http_port))
{
}

void ipfw_tun::start()
{
	_ipfw->start();

	_tun->read([this](const std::vector<uint8_t> && r_data) {
		_ipfw->handle_ip(std::move(r_data));
	});

	_thread = std::thread([this]() {
		_io.run();
	});
}

void ipfw_tun::stop()
{
	_ipfw->stop();
	_io.stop();
	if(_thread.joinable()) {
		_thread.join();
	}
}

boost::asio::io_service &ipfw_tun::io()
{
	return _io;
}

std::shared_ptr<tun> ipfw_tun::create_tun(uint64_t tun_fd)
{
	auto tun_err_fn = []() {
		ipfw_logger->error("Tun error");
	};
	return std::make_shared<tun>(tun_fd, _io, tun_err_fn);
}

std::shared_ptr<ipfw> ipfw_tun::create_ipfw(const std::string & fw_https_host, uint16_t fw_https_port,
											const std::string & fw_http_host, uint16_t fw_http_port)
{

	ipfw_args args {
		._http_fw_host =  fw_http_host,
		._http_fw_port = fw_http_port,
		._https_fw_host = fw_https_host,
		._https_fw_port = fw_https_port
	};

	_ipfw_callbacks._on_ip_packet = [this](const std::vector<uint8_t> && w_data) {
		_tun->write(std::move(w_data));
	};

	return	std::make_shared<ipfw>(_ipfw_callbacks, args);
}
