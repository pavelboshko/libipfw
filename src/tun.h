#pragma once

#include <functional>
#include <boost/asio.hpp>

typedef std::function<void(const std::vector<uint8_t> &&)>  tun_read_callback;
typedef std::function<void()>  tun_error_callback;

class tun
{
public:
	tun(int fd, boost::asio::io_service & io, const tun_error_callback & err_callback);
	static int alloc_tun(const std::string & if_name);
	void read(const tun_read_callback &);
	void write(const std::vector<uint8_t> &&);
	~tun();
private:
	int _tun_handle;
	boost::asio::io_service & _io;
	boost::asio::posix::stream_descriptor _input;
	boost::asio::posix::stream_descriptor _output;
	std::vector<uint8_t> _read_buf;
	const tun_error_callback _tun_error_callback;
	void doRead(const tun_read_callback &);
};

