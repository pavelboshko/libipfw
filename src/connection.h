#pragma once
#include <boost/asio.hpp>

class connection
{
	typedef std::function<void(const std::vector<uint8_t> &&)> onDataReady_t;
	typedef std::function<void(const std::string &)> onError_t;
	typedef std::function<void()> onConnected_t;
	typedef std::function<void()> onDisconnected_t;

	boost::asio::io_service &  _io;
	boost::asio::ip::tcp::resolver _resolver;
	boost::asio::ip::tcp::socket _socket;
	const std::string  _host, _port;
	onDataReady_t _onDataReady;
	onError_t _onError;
	onConnected_t _onConnected;
	onDisconnected_t _onDisconnected;
	bool _is_connected;
	bool _is_closed;
	std::array<uint8_t, 1024> _rbuf; // TODO сдедать из расчета mss
public:
	connection(boost::asio::io_service & io, const std::string & host, uint16_t port,
			   const onDataReady_t & onDataReady, const onError_t & onError, onConnected_t onConnected, onDisconnected_t onDisconnected)
		: _io(io),
		  _resolver(io),
		  _socket(io), _host(host), _port(std::to_string(port)),
		  _onDataReady(onDataReady), _onError(onError), _onConnected(onConnected), _onDisconnected(onDisconnected),
		  _is_connected(false), _is_closed(false)
	{

	}

	~connection();

	void start();
	void write(const std::vector<uint8_t> && wbuf);
	bool is_connected() const;
	void stop();

private:
	void startConnect(const boost::asio::ip::tcp::resolver::results_type & results){
		boost::asio::async_connect(_socket,results.begin(),results.end(),
								   [this](const boost::system::error_code& err, const boost::asio::ip::tcp::resolver::iterator & endpoint)
		{
			if(err) {
				_onError("async_connect");
				return;
			}
			_is_connected = true;
			doRead();
			_onConnected();
		});
	}
	void doRead();

};
