#include "tcp_packet.h"


std::shared_ptr<tcp_packet> tcp_packet::create(const uint8_t * pkt, size_t length) {
	uint8_t protocol;
	void *saddr;
	void *daddr;
	char flags[10];
	char data[16];
	int flen = 0;
	uint8_t *payload;
	int syn = 0;
	uint16_t sport = 0;
	uint16_t dport = 0;
	char source[INET6_ADDRSTRLEN + 1] = { 0 };
	char dest[INET6_ADDRSTRLEN + 1]  = { 0 };
	tcp_flags_t _tcp_flags;
	uint32_t seq(0), ack_seq(0);
	uint8_t ws(0); uint16_t mss(0);

	uint8_t version = (*pkt) >> 4;
	if (version == 4) {
		struct iphdr *ip4hdr = (struct iphdr *) pkt;
		protocol = ip4hdr->protocol;
		saddr = &ip4hdr->saddr;
		daddr = &ip4hdr->daddr;

		if (ip4hdr->frag_off & IP_MF) {
			ipfw_logger->debug("ip_utils::print_info ip fragment offset {}", (ip4hdr->frag_off & IP_OFFMASK) * 8);
			return nullptr;
		}

		uint8_t ipoptlen = (uint8_t) ((ip4hdr->ihl - 5) * 4);
		payload = (uint8_t *) (pkt + sizeof(struct iphdr) + ipoptlen);
		inet_ntop(version == 4 ? AF_INET : AF_INET6, saddr, source, sizeof(source));
		inet_ntop(version == 4 ? AF_INET : AF_INET6, daddr, dest, sizeof(dest));


		if (protocol == IPPROTO_TCP) {
			if (length - (payload - pkt) < sizeof(struct tcphdr)) {
				ipfw_logger->debug("tcp_packet::handler TCP packet too short");
				return nullptr;
			}

			struct tcphdr *tcp = (struct tcphdr *) payload;

			sport = ntohs(tcp->source);
			dport = ntohs(tcp->dest);

			if (tcp->syn) {
				syn = 1;
				flags[flen++] = 'S';
				_tcp_flags.set(tcp_flags::syn);
			}
			if (tcp->ack) {
				flags[flen++] = 'A';
				_tcp_flags.set(tcp_flags::ack);
			}
			if (tcp->psh) {
				flags[flen++] = 'P';
				_tcp_flags.set(tcp_flags::psh);
			}
			if (tcp->fin) {
				flags[flen++] = 'F';
				_tcp_flags.set(tcp_flags::fin);
			}
			if (tcp->rst) {
				flags[flen++] = 'R';
				_tcp_flags.set(tcp_flags::rst);
			}

			seq = ntohl(tcp->seq); // ISN remote
			ack_seq =  ntohl(tcp->ack_seq);

			const uint8_t tcpoptlen = (uint8_t) ((tcp->doff - 5) * 4);
			const uint8_t *data = payload + sizeof(struct tcphdr) + tcpoptlen;
			const size_t datalen = (const size_t) (length - (data - pkt));
			const uint8_t *tcpoptions = payload + sizeof(struct tcphdr);


			int optlen = tcpoptlen;
			uint8_t *options = (uint8_t *) tcpoptions;
			while (optlen > 0) {
				uint8_t kind = *options;
				uint8_t len = *(options + 1);
				if (kind == 0) // End of options list
					break;

				if (kind == 2 && len == 4)
					mss = ntohs(*((uint16_t *) (options + 2)));

				else if (kind == 3 && len == 3)
					ws = *(options + 2);

				if (kind == 1) {
					optlen--;
					options++;
				} else {
					optlen -= len;
					options += len;
				}
			}


			if(datalen > 0) {
				return std::make_shared<tcp_packet>(std::string(source),std::string(dest),
													sport, dport,
													_tcp_flags,
													seq, ack_seq, mss, ws, data, datalen);
			} else {
				return std::make_shared<tcp_packet>(std::string(source),std::string(dest),
													sport, dport,
													_tcp_flags,
													seq, ack_seq, mss, ws);
			}
		} else {
			return nullptr;
		}
	} else {
		return nullptr;
	}
}
