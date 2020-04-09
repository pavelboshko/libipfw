#pragma once
#include <stdint.h>
#include <unistd.h>
#include <memory>
#include <string>
#include <bitset>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <sys/types.h>
#include <asm/byteorder.h>
#include <iostream>
#include "ip_utils.h"
#include <vector>
#include "log.h"

class tcp_packet
{
public:
	enum tcp_flags : uint32_t {
		syn = 1, ack, psh, fin, rst
	};
	typedef std::bitset<tcp_flags::rst + 1> tcp_flags_t;

	static std::shared_ptr<tcp_packet> create(const uint8_t * pkt, size_t length);

	tcp_packet(const std::string && saddr, const std::string && daddr,
			   uint16_t sport, uint16_t dport,
			   const tcp_flags_t & tcp_flags, uint32_t seq_number, uint32_t ack_number, uint16_t mss, uint8_t win_scale) :
		_saddr(saddr), _daddr(daddr),
		_sport(sport), _dport(dport),
		_tcp_flags(tcp_flags),
		_seq_number(seq_number), _ack_number(ack_number),
		_mss(mss), _win_scale(win_scale)
	{
		//		BOOST_LOG_TRIVIAL(debug) << "seq_number " << std::hex << _seq_number << " _ack_number " << _ack_number
		//				  << " tcp_flags " << _tcp_flags << std::dec;
	}

	tcp_packet(const std::string && saddr, const std::string && daddr,
			   uint16_t sport, uint16_t dport,
			   const tcp_flags_t & tcp_flags, uint32_t seq_number, uint32_t ack_number,
			   uint16_t mss, uint8_t win_scale,
			   const uint8_t * payload, size_t payload_len) :
		_saddr(saddr), _daddr(daddr),
		_sport(sport), _dport(dport),
		_tcp_flags(tcp_flags),
		_seq_number(seq_number), _ack_number(ack_number),
		_payload(payload, payload + payload_len),
		_mss(mss), _win_scale(win_scale)
	{
		//		BOOST_LOG_TRIVIAL(debug) << "seq_number " << std::hex << _seq_number << " _ack_number " << _ack_number
		//				  << " tcp_flags " << _tcp_flags << std::dec
		//				  << " payload size " << _payload.size();
	}

	const std::string _saddr;
	const std::string  _daddr;
	const uint16_t _sport;
	const uint16_t _dport;
	const tcp_flags_t _tcp_flags;
	const uint32_t _seq_number, _ack_number;
	const std::vector<uint8_t> _payload;
	const uint16_t _mss;  // max segment size
	const uint8_t _win_scale;
};


