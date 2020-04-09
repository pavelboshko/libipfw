#include "tun.h"
#include <linux/if_tun.h>
#include "log.h"


tun::tun(int tun_handle, boost::asio::io_service &io, const tun_error_callback & err_callback)
	: _tun_handle(tun_handle), _io(io),
	  _input(io, ::dup(_tun_handle)), _output(io, ::dup(_tun_handle)),
	  _read_buf(1024 * 2),
	  _tun_error_callback(err_callback)
{
}

int tun::alloc_tun(const std::string &if_name)
{
	auto _tun_handle = ::open("/dev/net/tun", O_RDWR);
	if (_tun_handle < 0) {
			throw std::runtime_error("Failed to open file /dev/net/tun");
	}

	ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	snprintf(ifr.ifr_name, IFNAMSIZ, "%s", if_name.c_str());
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

	if (ioctl(_tun_handle, TUNSETIFF, &ifr) < 0) {
			throw std::runtime_error("Failed to create virtual network interface " + if_name);
	}
	return _tun_handle;
}

void tun::read(const tun_read_callback & _tun_read_callback)
{
	doRead(_tun_read_callback);
}

void tun::write(const std::vector<uint8_t> && data)
{
	_output.async_write_some(boost::asio::buffer(data), [this](const boost::system::error_code& ec, std::size_t bytes_write){
		if(ec) {
			ipfw_logger->error("tun::write {}", ec.message());
			_tun_error_callback();
		}
	});
}

tun::~tun()
{
}

void tun::doRead(const tun_read_callback & _tun_read_callback)
{

	_input.async_read_some(boost::asio::buffer(_read_buf), [this, _tun_read_callback](const boost::system::error_code& ec, std::size_t bytes_read) {
		if(!ec) {
			_tun_read_callback(std::vector<uint8_t>(_read_buf.begin(), _read_buf.begin() +bytes_read));
			doRead(_tun_read_callback);
		} else {
			ipfw_logger->error("tun::doRead {}", ec.message());
			_tun_error_callback();
		}
	});
}


