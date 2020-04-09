#pragma once

#include "log.h"
#include <functional>
#include "tcp_packet.h"

class tcp_session_fsm
{
public:
	enum state {
		CLOSED = 0,
		LISTEN,
		SYN_SENT,
		SYN_RCVD,
		ESTABLISHED,
		CLOSE_WAIT,
		LAST_ACK,
		FIN_WAIT_1,
		FIN_WAIT_2,
		CLOSING,
		TIME_WAIT
	};

	enum event {
		CALL_OPEN,
		SEND_SYN_ACK,
		RECV_ACK,
		SEND_ACK,
		RECV_FIN,
		SEND_FIN,
		RECV_RST,
		SND_RST,
		TIMEOUT,
		RECV_PUSH,
		SEND_PUSH,
	};

	typedef std::function<void (state, state)> onStateCahange_t;

	tcp_session_fsm(const onStateCahange_t && state_fn, size_t id);
	void on_incomming_tcp_packet(const std::shared_ptr<tcp_packet> & packet);	
	state get_state();
	void on_sent_rst();
	void on_sent_fin();

	static std::string state_to_str(state s);
	static std::string event_to_str(event s);

private:
	void set_state(state s);
	void handle_event(event ev);
	void on_close(event ev);
	void on_syn_sent(event ev);
	void on_establish(event ev);
	void on_closing(event ev);\
	void on_close_wait(event ev);
	void on_fin_wait_1(event ev);
	void on_fin_wait_2(event ev);

	state _state;
	onStateCahange_t _onStateCahange;
	const size_t _id;
};


