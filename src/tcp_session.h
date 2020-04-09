#pragma once
#include <memory>
#include "tcp_packet.h"
#include "tcp_session_callbacks.h"
#include "ipfw_args.h"
#include "connection.h"
#include "tcp_session_fsm.h"
#include <boost/asio.hpp>
#include <queue>

class tcp_session
{
public:
	tcp_session(const std::shared_ptr<tcp_packet>&  sync_packet,
				ptcp_session_callbacks callbacks, boost::asio::io_service & io, const ipfw_args & args);
	~tcp_session();
	const std::string _saddr;
	const std::string  _daddr;
	const uint16_t _sport;
	const uint16_t _dport;

	size_t id() const;
	static size_t calcId(const std::shared_ptr<tcp_packet> &);
	void handle_packet(const std::shared_ptr<tcp_packet>&  packet);
	uint32_t remote_seq() const;
	uint32_t local_seq() const;
	void shutdown();

private:

	uint32_t _remote_seq; // confirmed bytes received, host notation
	uint32_t _local_seq; // confirmed bytes sent, host notation
	const size_t _id;
	void forward_packet(const std::shared_ptr<tcp_packet>&  packet);
	void forward_packets();
	void start_close();

	ptcp_session_callbacks _callbacks;
	boost::asio::io_service & _io;
	std::shared_ptr<connection> _connection;
	std::shared_ptr<tcp_session_fsm> _fsm;
	std::vector<uint8_t> _dump;
	const ipfw_args _ipfw_args;
//	TODO
//	std::queue<std::shared_ptr<tcp_packet>> _snd_data_queue;

};

typedef std::shared_ptr<tcp_session> ptcp_session;
typedef size_t session_id;
