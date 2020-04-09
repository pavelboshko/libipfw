#include "tcp_session.h"
#include "tcp_utils.h"
#include <functional>
#include <netinet/tcp.h>
#include <fstream>


constexpr static std::hash<std::string> hash_fn;

static inline bool isHttps(const std::shared_ptr<tcp_packet>& packet) {
	// FIXME
	return  packet->_dport == 443;
}


tcp_session::tcp_session(const std::shared_ptr<tcp_packet>&  sync_packet,
						 ptcp_session_callbacks callbacks, boost::asio::io_service & io, const ipfw_args & args) :
	_saddr (sync_packet->_saddr),
	_daddr (sync_packet->_daddr),
	_sport (sync_packet->_sport),
	_dport (sync_packet->_dport),
	_remote_seq(sync_packet->_seq_number), _local_seq(0),
	_id(calcId(sync_packet)),
	_callbacks(callbacks),
	_io(io),
	_ipfw_args(args)
{



	auto state_fn = [this](tcp_session_fsm::state old_state, tcp_session_fsm::state new_state) {
		ipfw_logger->debug("tcp_session id: {} {}  -> {}",_id , tcp_session_fsm::state_to_str(old_state),
						   tcp_session_fsm::state_to_str(new_state));
		switch (new_state) {
		case tcp_session_fsm::state::SYN_SENT:
			_remote_seq += 1;
			_callbacks->_on_write_data(tcp_utils::write_syn_ack(this));
			break;
		case tcp_session_fsm::state::ESTABLISHED:
			_local_seq += 1;
			break;
		case tcp_session_fsm::state::CLOSE_WAIT:
			_remote_seq += 1;
			_callbacks->_on_write_data(tcp_utils::write_fin_ack(this));
			break;
		case tcp_session_fsm::state::FIN_WAIT_1:
			_remote_seq += 1;
			_local_seq += 1;
			_callbacks->_on_write_data(tcp_utils::write_ack(this));
			break;
		case tcp_session_fsm::state::FIN_WAIT_2:
			// FIXME разобраться с последовательностью, хотя сессеия с приложением закрывается нормально
			_callbacks->_on_write_data(tcp_utils::write_ack(this));
			break;
		case tcp_session_fsm::state::CLOSED:
			_connection->stop();
			_callbacks->_on_session_closed(_id);
			break;
		default:
			break;
		}
	};

	_fsm = std::make_shared<tcp_session_fsm>(state_fn, _id);

	auto read_fn = [this](const std::vector<uint8_t> && rdata) {
		ipfw_logger->debug( " tcp_session id: {} connection: rdata: {}", _id, rdata.size());

		_callbacks->_on_write_data(tcp_utils::write_data(this, std::move(rdata)));
		_local_seq += rdata.size();

#ifdef DUMP
		_dump.insert(_dump.end(), rdata.begin(), rdata.end());
#endif
	};

	auto error_fn = [this](const std::string & err) {
		ipfw_logger->debug( " tcp_session : {} connection err, state {}",   _id, tcp_session_fsm::state_to_str(_fsm->get_state()));

		if(_fsm->get_state() != tcp_session_fsm::state::CLOSED) {
			_callbacks->_on_write_data(tcp_utils::write_rst(this));
			_fsm->on_sent_rst();
		}
	};

	auto connected_fn = [this]() {
		ipfw_logger->debug( " tcp_session : {} connection: connected",   _id);
		//		forward_packets(); TODO
	};

	auto disconnected_fn = [this]() {
		ipfw_logger->debug( " tcp_session : {} connection: disconnected",   _id);
		start_close();
	};

	if(isHttps(sync_packet)){
		_connection = std::make_shared<connection>(_io,
												   _ipfw_args._https_fw_host,
												   _ipfw_args._https_fw_port,
												   read_fn, error_fn, connected_fn, disconnected_fn);
	} else {
		_connection = std::make_shared<connection>(_io,
												   _ipfw_args._http_fw_host,
												   _ipfw_args._http_fw_port,
												   read_fn, error_fn, connected_fn, disconnected_fn);
	}

	_connection->start();

	ipfw_logger->debug( " tcp_session create id: {}",   _id);
}

tcp_session::~tcp_session()
{
	ipfw_logger->debug( " tcp_session delete id: {}",   _id);
#ifdef DUMP
	if(!_dump.empty()) {
		std::ofstream file( std::to_string(_id).c_str(), std::ios_base::app | std::ios_base::out );
		file.write((char*)_dump.data(), _dump.size());
	}
#endif
}

size_t tcp_session::id() const
{
	return _id;
}

size_t tcp_session::calcId(const std::shared_ptr<tcp_packet> & packet)
{
	return hash_fn(packet->_saddr + packet->_daddr +
				   std::to_string(packet->_sport)  + std::to_string(packet->_dport));
}

void tcp_session::handle_packet(const std::shared_ptr<tcp_packet> & packet)
{
	_fsm->on_incomming_tcp_packet(packet);

	if(_fsm->get_state() == tcp_session_fsm::state::ESTABLISHED) {
		if(packet->_tcp_flags.test(tcp_packet::tcp_flags::psh)) {
			if(_connection->is_connected()) {
				forward_packet(packet);
			}
			//TODO store to _snd_data_queue
		}
	}
}

uint32_t tcp_session::remote_seq() const
{
	return _remote_seq;
}

uint32_t tcp_session::local_seq() const
{
	return _local_seq;
}

void tcp_session::shutdown()
{
	_connection->stop();
	_connection = nullptr;
}

void tcp_session::forward_packet(const std::shared_ptr<tcp_packet> &packet)
{
	_connection->write(std::vector<uint8_t>(packet->_payload));
	_remote_seq += packet->_payload.size();
	_callbacks->_on_write_data(tcp_utils::write_ack(this));
}

void tcp_session::forward_packets()
{

}

void tcp_session::start_close()
{
	_callbacks->_on_write_data(tcp_utils::write_fin_ack(this));
	_fsm->on_sent_fin();
}

