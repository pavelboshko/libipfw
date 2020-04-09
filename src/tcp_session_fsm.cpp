/*
 * tcp_session_fsm.cpp
 *
 *  Created on: 1 13 2020
 *      Author: pbozhko
 */
#include "tcp_session_fsm.h"
#include <assert.h>

tcp_session_fsm::tcp_session_fsm(const onStateCahange_t && state_fn, size_t id) :
	_state(state::CLOSED),
	_onStateCahange(state_fn),
	_id(id)
{

}

void tcp_session_fsm::on_incomming_tcp_packet(const std::shared_ptr<tcp_packet> &packet)
{
	if(packet->_tcp_flags.test(tcp_packet::tcp_flags::syn)) {
		handle_event(event::CALL_OPEN);
	}
	else
	if(packet->_tcp_flags.test(tcp_packet::tcp_flags::rst))	{
		handle_event(event::RECV_RST);
	}
	else
	if(packet->_tcp_flags.test(tcp_packet::tcp_flags::ack))	{
		if(packet->_tcp_flags.test(tcp_packet::tcp_flags::psh)){
			// TODO
		}
		else
		if(packet->_tcp_flags.test(tcp_packet::tcp_flags::fin)) {
			handle_event(event::RECV_FIN);
		} else {
			handle_event(event::RECV_ACK);
		}

	}
	else
	if(packet->_tcp_flags.test(tcp_packet::tcp_flags::fin))	{
		handle_event(event::RECV_FIN);
	}
	else {
		ipfw_logger->debug( " tcp_session_fsm id {} incomming_tcp_packet unhandle packet",   _id);
		assert(false);
	}
}

void tcp_session_fsm::handle_event(tcp_session_fsm::event ev)
{
	ipfw_logger->debug( " tcp_session_fsm id {} handle_event {}",   _id, event_to_str(ev));
	switch (get_state()) {
		case CLOSED:
			on_close(ev);
		case LISTEN:
			break;
		case SYN_SENT:
			on_syn_sent(ev);
		case SYN_RCVD:
			break;
		case ESTABLISHED:
			on_establish(ev);
			break;
		case CLOSE_WAIT:
			on_close_wait(ev);
			break;
		case LAST_ACK:
			break;
		case FIN_WAIT_1:
			on_fin_wait_1(ev);
			break;
		case FIN_WAIT_2:
			on_fin_wait_2(ev);
			break;
		case CLOSING:
			on_closing(ev);
			break;
		case TIME_WAIT:
			break;
	default:
		ipfw_logger->debug( " tcp_session_fsm {} on_incomming_tcp_packet unhandle event",   _id);
		assert(false);
		break;
	}
}

tcp_session_fsm::state tcp_session_fsm::get_state()
{
	return _state;
}

void tcp_session_fsm::on_sent_rst()
{
	handle_event(tcp_session_fsm::event::SND_RST);
}

void tcp_session_fsm::on_sent_fin()
{
	handle_event(tcp_session_fsm::event::SEND_FIN);
}

void tcp_session_fsm::set_state(tcp_session_fsm::state s)
{
	state old = _state;
	_state = s;
	if(old != _state) {
		_onStateCahange(old, _state);
	}
}

std::string tcp_session_fsm::state_to_str(tcp_session_fsm::state s)
{
	switch (s) {
		case CLOSED: return "CLOSED";
		case LISTEN: return "LISTEN";
		case SYN_SENT: return "SYN_SENT";
		case SYN_RCVD: return "SYN_RCVD";
		case ESTABLISHED: return "ESTABLISHED";
		case CLOSE_WAIT: return "CLOSE_WAIT";
		case LAST_ACK: return "LAST_ACK";
		case FIN_WAIT_1: return "FIN_WAIT_1";
		case FIN_WAIT_2: return "FIN_WAIT_2";
		case CLOSING: return "CLOSING";
		case TIME_WAIT: return "TIME_WAIT";
	default:
		assert(false);
		return "";
	}
}

std::string tcp_session_fsm::event_to_str(tcp_session_fsm::event s)
{
	switch (s) {
		case CALL_OPEN: return "CALL_OPEN";
		case SEND_SYN_ACK: return "SEND_SYN_ACK";
		case RECV_ACK: return "RECV_ACK";
		case SEND_ACK: return "SEND_ACK";
		case RECV_FIN: return "RECV_FIN";
		case SEND_FIN: return "SEND_FIN";
		case RECV_RST: return "RECV_RST";
		case SND_RST: return "SND_RST";
		case TIMEOUT: return "TIMEOUT";
		case RECV_PUSH: return "RECV_PUSH";
		case SEND_PUSH: return "SEND_PUSH";
	default:
		assert(false);
		return "";
	}
}

void tcp_session_fsm::on_close(tcp_session_fsm::event ev)
{
	switch (ev) {
		case CALL_OPEN:
			set_state(SYN_SENT);
			break;
		default:
			break;
	}
}

void tcp_session_fsm::on_syn_sent(tcp_session_fsm::event ev)
{
	switch (ev) {
		case RECV_RST:
			set_state(CLOSED);
			break;
		case RECV_ACK:
			set_state(ESTABLISHED);
			break;
		default:
			break;
	}
}

void tcp_session_fsm::on_establish(tcp_session_fsm::event ev)
{
	switch (ev) {
		case RECV_RST:
		case SND_RST:
			set_state(CLOSED);
			break;
		case RECV_FIN:
			set_state(CLOSE_WAIT);
			break;
		case SEND_FIN:
			set_state(FIN_WAIT_1);
			break;
		default:
			break;
	}
}

void tcp_session_fsm::on_close_wait(tcp_session_fsm::event ev)
{
	switch (ev) {
		case TIMEOUT:
		case SND_RST:
		case RECV_RST:
			set_state(CLOSED);
			break;
		case RECV_ACK:
			set_state(CLOSED);
			break;
		default:
			break;
	}
}

void tcp_session_fsm::on_fin_wait_1(tcp_session_fsm::event ev)
{
	switch (ev) {
		case TIMEOUT:
		case SND_RST:
		case RECV_RST:
			set_state(CLOSED);
			break;
		case RECV_FIN:
			set_state(CLOSED);
			break;
		case RECV_ACK:
			set_state(FIN_WAIT_2);
			break;
		default:
			break;
	}
}

void tcp_session_fsm::on_fin_wait_2(tcp_session_fsm::event ev)
{
	switch (ev) {
		case TIMEOUT:
		case SND_RST:
		case RECV_RST:
			set_state(CLOSED);
			break;
		case RECV_FIN:
			set_state(CLOSED);
			break;
		default:
			break;
	}
}

void tcp_session_fsm::on_closing(tcp_session_fsm::event ev)
{
	switch (ev) {
		case TIMEOUT:
		case RECV_ACK:
			set_state(CLOSED);
			break;	
		default:
			break;
	}
}


