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
#include "log.h"
#include <sys/types.h>
#include <asm/byteorder.h>
#include "tcp_session.h"


class tcp_utils
{
public:
	static std::vector<uint8_t> write_rst(const tcp_session * session) {
		return generate_tcp_packet(session, NULL, 0, 0, 1, 0, 1, 0);
	}

	static std::vector<uint8_t> write_syn_ack(const tcp_session * session) {
		return generate_tcp_packet(session, NULL, 0, 1, 1, 0, 0, 0);
	}

	static std::vector<uint8_t> write_ack(const tcp_session * session) {
		return generate_tcp_packet(session, NULL, 0, 0, 1, 0, 0, 0);
	}

	static std::vector<uint8_t> write_data(const tcp_session * session, const std::vector<uint8_t> && wdata) {
		return generate_tcp_packet(session, wdata.data(), wdata.size(), 0, 1, 0, 0, 1);
	}

	static std::vector<uint8_t> write_fin_ack(const tcp_session * session) {
		return generate_tcp_packet(session,NULL, 0, 0, 1, 1, 0, 0);
	}

	static uint16_t get_receive_window() {
		return 0xffff;
	}

	static std::vector<uint8_t> generate_tcp_packet(const tcp_session * session,
													const uint8_t *data, size_t datalen,
													int syn, int ack, int fin, int rst, int psh)
	{
		size_t len;
		struct tcphdr *tcp;
		uint16_t csum;
		char source[INET6_ADDRSTRLEN + 1];
		char dest[INET6_ADDRSTRLEN + 1];

		// Build packet
		int optlen = (syn ? 4 + 3 + 1 : 0);
		uint8_t *options;

		len = sizeof(struct iphdr) + sizeof(struct tcphdr) + optlen + datalen;
		std::vector<uint8_t> buffer(len);

		struct iphdr *ip4 = (struct iphdr *) buffer.data();
		tcp = (struct tcphdr *) (buffer.data() + sizeof(struct iphdr));
		options = buffer.data() + sizeof(struct iphdr) + sizeof(struct tcphdr);
		if (datalen)
			memcpy(buffer.data() + sizeof(struct iphdr) + sizeof(struct tcphdr) + optlen, data, datalen);

		// Build IP4 header
		memset(ip4, 0, sizeof(struct iphdr));
		ip4->version = 4;
		ip4->ihl = sizeof(struct iphdr) >> 2;
		ip4->tot_len = htons(len);
		ip4->ttl = IPDEFTTL;
		ip4->protocol = IPPROTO_TCP;
		ip4->saddr = inet_addr(session->_daddr.c_str());
		ip4->daddr = inet_addr(session->_saddr.c_str());

		// Calculate IP4 checksum
		ip4->check = ~ip_utils::calc_checksum(0, (uint8_t *) ip4, sizeof(struct iphdr));

		// Calculate TCP4 checksum
		ip_utils::ippseudo pseudo;
		memset(&pseudo, 0, sizeof(ip_utils::ippseudo));
		pseudo.ippseudo_src.s_addr = (__be32) ip4->saddr;
		pseudo.ippseudo_dst.s_addr = (__be32) ip4->daddr;
		pseudo.ippseudo_p = ip4->protocol;
		pseudo.ippseudo_len = htons(sizeof(struct tcphdr) + optlen + datalen);

		csum = ip_utils::calc_checksum(0, (uint8_t *) &pseudo, sizeof(ip_utils::ippseudo));


		// Build TCP header
		memset(tcp, 0, sizeof(struct tcphdr));
		tcp->source = htons(session->_dport);
		tcp->dest = htons(session->_sport);
		tcp->seq = htonl(session->local_seq());
		tcp->ack_seq = htonl((uint32_t)session->remote_seq());
		tcp->doff = (__u16) ((sizeof(struct tcphdr) + optlen) >> 2);
		tcp->syn = (__u16) syn;
		tcp->ack = (__u16) ack;
		tcp->fin = (__u16) fin;
		tcp->rst = (__u16) rst;
		tcp->psh = (__u16) psh;
		tcp->window = htons(get_receive_window()); //htons(cur->recv_window >> cur->recv_scale);

		if (!tcp->ack)
			tcp->ack_seq = 0;

		// TCP options
		if (syn) {
			*(options) = 2; // MSS
			*(options + 1) = 4; // total option length
			*((uint16_t *) (options + 2)) = ip_utils::get_default_mss();

			*(options + 4) = 3; // window scale
			*(options + 5) = 3; // total option length
			*(options + 6) = 1;	// TODO cur->recv_scale;

			*(options + 7) = 0; // End, padding
		}

		// Continue checksum
		csum = ip_utils::calc_checksum(csum, (uint8_t *) tcp, sizeof(struct tcphdr));
		csum = ip_utils::calc_checksum(csum, options, (size_t) optlen);
		csum = ip_utils::calc_checksum(csum, data, datalen);
		tcp->check = ~csum;

		return std::move(buffer);
	}
};
