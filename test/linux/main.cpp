
#include <unistd.h>
#include <boost/asio/signal_set.hpp>
#include <boost/asio.hpp>
#include "execute_command.h"
#include "ipfw_tun.h"
#include "log.h"
#include <chrono>
#include <atomic>
#include <algorithm>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

constexpr auto if_name="tun1000";
constexpr auto if_ip="10.10.10.1";
constexpr int fw_mark = 0x01;

constexpr uint16_t if_mask=24;
static std::atomic<bool> use_loop (true);


std::string nornalize_string(const std::string & str) {
	std::string ret (str);
	ret.erase(std::remove(ret.begin(), ret.end(), '\n'), ret.end());
	ret.erase(std::remove(ret.begin(), ret.end(), '\r'), ret.end());
	return ret;
}

void init_tun(const std::string & if_name, const std::string & ip, uint16_t mask, const std::vector<std::string> & resolve_hosts) {
	ExecuteCommand("ip a add %1%/%2% dev %3%", ip, mask, if_name);
	ExecuteCommand("ip l s %1% up", if_name);


	std::string route_table = ExecuteCommand("grep tls  /etc/iproute2/rt_tables");
	if(route_table.empty()) {
		throw std::runtime_error("need add route table echo '201 tls' >> /etc/iproute2/rt_tables");
	}

	std::string default_dev = ExecuteCommand("ip route | grep default | awk '{ print $5 }'");
	if(default_dev.empty()) {
		throw std::runtime_error("can`t detect default interface");
	}

	std::string default_gw_addr = ExecuteCommand("ip route | grep default | awk '{ print $3 }'");
	if(default_gw_addr.empty()) {
		throw std::runtime_error("can`t detect default gw");
	}

	std::string rule = ExecuteCommand("ip rule | grep tls");
	if(rule.empty()) {
		ExecuteCommand("ip rule add fwmark 1 table tls");
	}

	for(auto & ip : resolve_hosts) {
		ExecuteCommand("ip route add %1%/32 dev %2%", ip, if_name);
		ExecuteCommand("ip route add %1% via %2% dev %3% table tls", ip, nornalize_string(default_gw_addr),
					   nornalize_string(default_dev));

	}

	ExecuteCommand("iptables -t nat -A POSTROUTING -o %1% -j MASQUERADE",  nornalize_string(default_dev));
	ExecuteCommand("iptables -A FORWARD -i %1% -o %2% -j ACCEPT", if_name, nornalize_string(default_dev));
}

//
const std::vector<std::string> resolve_host(const std::string & host) {
	struct hostent *hp = gethostbyname(host.c_str());
	if(hp) {
		std::vector<std::string> resolve_hosts;
		unsigned int i=0;
		while ( hp -> h_addr_list[i] != NULL) {
			resolve_hosts.emplace_back(std::string(inet_ntoa( *( struct in_addr*)( hp -> h_addr_list[i]))));
		   i++;
		}
		return resolve_hosts;
	} else {
		throw std::runtime_error("can`t resolve_host");
	}
}


void init_log() {
	auto stderr_sink = spdlog::sinks::stderr_sink_mt::instance();
	ipfw_logger = std::make_shared<spdlog::logger>("ipfw", stderr_sink);
	ipfw_logger->set_level(spdlog::level::debug);
}


// example sudo ./ipfw_test --resource tls-mobile.securitycode.ru
// wget http://tls-mobile.securitycode.ru:443/4mb
int main(int argc, char *argv[])
{
	po::options_description desc;
	po::variables_map vm;
	desc.add_options()
		("resource, r", po::value<std::string>(), "tls resource");

	po::store(po::parse_command_line(argc, argv, desc), vm);

	init_log();

	auto resource = vm["resource"].as<std::string>();
	ipfw_logger->info("Create for resource {}", resource);

	auto tun_handle = tun::alloc_tun(if_name);

	init_tun(if_name, if_ip, if_mask, resolve_host(resource));

	auto ipfwtun = std::make_shared<ipfw_tun>("127.0.0.1", 8443, "127.0.0.1", 8080, tun_handle);
	boost::asio::signal_set _signals(ipfwtun->io(), SIGTERM, SIGINT);

	_signals.async_wait([&](const boost::system::error_code& ec,  int signal_number) {
		ipfw_logger->info("Handle signal {}", signal_number);
		use_loop = false;
	});

	ipfwtun->start();

	while(use_loop) {
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}

	ipfwtun->stop();
	close(tun_handle);

	return 0;
}
