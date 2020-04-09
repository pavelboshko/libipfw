/*
 * connection.cpp
 *
 *  Created on: 1 13 2020
 *      Author: pbozhko
 */

#include "connection.h"
#include <boost/asio.hpp>

void connection::write(const std::vector<uint8_t> &&wbuf)
{
	if(_is_closed) {
		return;
	}

	boost::asio::async_write(_socket, boost::asio::buffer(wbuf),[this](const boost::system::error_code& ec,
							 size_t bytes_transferred) {

		if(ec){
			_onError("async_write");
			return;
		}
	});
}

bool connection::is_connected() const
{
	return _is_connected;
}

void connection::stop()
{
	if(_is_closed) {
		return;
	}

	_is_closed = true;

	 boost::system::error_code ignore;
	_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignore);
	_socket.close(ignore);
}

connection::~connection() {
	stop();
}

void connection::start()
{
	_resolver.async_resolve(_host, _port,  [this](const boost::system::error_code& err,
							boost::asio::ip::tcp::resolver::results_type results)
	{
		if(err) {
			_onError("async_resolve");
			return;
		}
		startConnect(results);
	});
}


void connection::doRead() {
	if(_is_closed) {
		return;
	}

	_socket.async_read_some(boost::asio::buffer(_rbuf),[this](const boost::system::error_code& ec ,size_t bytes_read) {
		if(ec == boost::asio::error::eof) {
			_onDisconnected();
			return;
		}
		else
			if(ec){
				_onError(std::string("async_read ") + ec.message());
				return;
			} else {
				_onDataReady(std::vector<uint8_t>(_rbuf.data(), _rbuf.data() + bytes_read));
				doRead();
			}

	});

}
