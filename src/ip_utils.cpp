#include "ip_utils.h"
#include <unistd.h>
#include <stdint.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>


uint16_t ip_utils::calc_checksum(uint16_t start, const uint8_t *buffer, size_t length) {
	uint32_t sum = start;
	uint16_t *buf = (uint16_t *) buffer;
	size_t len = length;

	while (len > 1) {
		sum += *buf++;
		len -= 2;
	}

	if (len > 0)
		sum += *((uint8_t *) buf);

	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	return (uint16_t) sum;
}


void ip_utils::print_info(const uint8_t * pkt, size_t length,  const std::string & direction ) {

	uint8_t protocol;
	void *saddr;
	void *daddr;
	char flags[10] = { 0 };
	char data[16];
	int flen = 0;
	uint8_t *payload;
	int syn = 0;
	uint16_t sport = 0;
	uint16_t dport = 0;
	char source[INET6_ADDRSTRLEN + 1];
	char dest[INET6_ADDRSTRLEN + 1];

	uint8_t version = (*pkt) >> 4;
	if (version == 4) {
		struct iphdr *ip4hdr = (struct iphdr *) pkt;
		protocol = ip4hdr->protocol;
		saddr = &ip4hdr->saddr;
		daddr = &ip4hdr->daddr;

		if (ip4hdr->frag_off & IP_MF) {
			ipfw_logger->error("ip_utils::print_info ip fragment offset {}", (ip4hdr->frag_off & IP_OFFMASK) * 8);
			return;
		}

		uint8_t ipoptlen = (uint8_t) ((ip4hdr->ihl - 5) * 4);
		payload = (uint8_t *) (pkt + sizeof(struct iphdr) + ipoptlen);
		inet_ntop(version == 4 ? AF_INET : AF_INET6, saddr, source, sizeof(source));
		inet_ntop(version == 4 ? AF_INET : AF_INET6, daddr, dest, sizeof(dest));

		if (protocol == IPPROTO_TCP) {
				if (length - (payload - pkt) < sizeof(struct tcphdr)) {
					ipfw_logger->error("ip_utils::print_info TCP packet too short");
					return;
				}

				struct tcphdr *tcp = (struct tcphdr *) payload;

				sport = ntohs(tcp->source);
				dport = ntohs(tcp->dest);

				if (tcp->syn) {
					syn = 1;
					flags[flen++] = 'S';
				}
				if (tcp->ack)
					flags[flen++] = 'A';
				if (tcp->psh)
					flags[flen++] = 'P';
				if (tcp->fin)
					flags[flen++] = 'F';
				if (tcp->rst)
					flags[flen++] = 'R';

				ipfw_logger->debug("{} Packet v{} {}/{} > {}/{} proto {} flags {}", direction, version,
								   std::string(source), sport, std::string(dest), dport,  "TCP", std::string(flags));
		}
	} else {
		ipfw_logger->error("ip_utils::print_info unsupport ip version {}", version);
	}

}

uint16_t ip_utils::get_mtu()
{
	return 1024 * 2;
}

uint16_t ip_utils::get_default_mss()
{
	return (uint16_t) (get_mtu() - sizeof(struct iphdr) - sizeof(struct tcphdr));
}
