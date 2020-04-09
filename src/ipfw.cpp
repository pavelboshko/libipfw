#include "ipfw.h"
#include "ip_utils.h"
#include "tcp_packet.h"
#include "tcp_session.h"
#include "tcp_utils.h"
#include "connection.h"
#include <boost/date_time/posix_time/posix_time.hpp>

ipfw::ipfw(const ipfw_callbacks &callbacks, const ipfw_args &args) :
	_callbacks(callbacks), _args(args), _wdog_timer(_io, boost::posix_time::seconds(5))
{

}

ipfw::~ipfw()
{
	if(_io_thread.joinable()){
		_io_thread.join();
	}
}

void ipfw::start()
{
	_io_thread = std::thread([this](){
		restart_wdog();
		_io.run();
		for(auto & session : _tcp_sessions) {
			if(session.second) session.second->shutdown();
		}
	});
}

void ipfw::stop()
{
	_io.stop();

}


void ipfw::handle_ip(const std::vector<uint8_t> && ip_packet)
{
	ip_utils::print_info(ip_packet.data(), ip_packet.size(), ">>>");
	auto pt_packet = std::make_shared<std::vector<uint8_t>>(std::move(ip_packet));
	_io.dispatch([this, pt_packet]() {
		ip_utils::print_info(pt_packet->data(), pt_packet->size(), ">>>");
		auto packet = tcp_packet::create(pt_packet->data(), pt_packet->size());

		if(packet) {
			auto id = tcp_session::calcId(packet);
			auto session = _tcp_sessions[id];
			if(!session ) {
				if(!packet->_tcp_flags.test(tcp_packet::tcp_flags::syn)) {
					return;
				}

				auto  callbacks = std::make_shared<tcp_session_callbacks>();
				callbacks->_on_write_data = [this](const std::vector<uint8_t> && w_data) {
					ip_utils::print_info(w_data.data(), w_data.size(), "<<<");
					_callbacks._on_ip_packet(std::move(w_data));
				};

				callbacks->_on_session_closed = [this](size_t id) {
					_io.post([id,this]{
						_tcp_sessions.erase(id);
					});
				};

				_tcp_sessions[id] = std::make_shared<tcp_session>(packet, callbacks, _io, _args);
			}
			if(_tcp_sessions[id]) {
				_tcp_sessions[id]->handle_packet(packet);
			}
		}
	});
}

void ipfw::restart_wdog()
{
	_wdog_timer.async_wait([this](const boost::system::error_code& ec) {
		if(!ec) {
			restart_wdog();
		}
	});
}
