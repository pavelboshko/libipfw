#pragma once
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <boost/log/trivial.hpp>
#include <sys/types.h>
#include "log.h"

class ip_utils
{
public:
	struct ippseudo {
		struct    in_addr ippseudo_src;	/* source internet address */
		struct    in_addr ippseudo_dst;	/* destination internet address */
		u_int8_t  ippseudo_pad;		/* pad, must be zero */
		u_int8_t  ippseudo_p;		/* protocol */
		u_int16_t ippseudo_len;		/* protocol length */
	};

	static void print_info(const uint8_t * data, size_t len, const std::string & direction = ">>>");
	static uint16_t calc_checksum(uint16_t start, const uint8_t *buffer, size_t length);
	static uint16_t get_mtu();
	static uint16_t get_default_mss();
};
