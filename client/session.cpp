#include <cstring>
#include <iostream>
#include <boost/bind.hpp>

#include "interface.hpp"
#include "packet.hpp"
#include "player.hpp"
#include "session.hpp"

Session::Session(boost::asio::io_service& io, UIInfo *ui_info):
	player_(new Player(this, ui_info)),
	socket_(io),
	send_(),
	recv_()
{
}

Session::~Session(){
	delete player_;
}

void Session::initialize(){
	//for client, just start reading
	start_read_header();
}

void Session::start_read_header(){
	boost::asio::async_read(socket_,
		boost::asio::buffer(recv_, 4),
		boost::bind(&Session::read_header_handler, shared_from_this(), boost::asio::placeholders::error));
}

void Session::read_header_handler(const boost::system::error_code &ec){
	if(!ec){
		short body_length = *(short*)(recv_ + 2);
		
		if(body_length > 0){
			boost::asio::async_read(socket_,
				boost::asio::buffer(recv_ + 4, body_length),
				boost::bind(&Session::read_body_handler, shared_from_this(), boost::asio::placeholders::error));
		}
		else{
			//if the packet doesn't have a body, then call the body handler directly
			read_body_handler(ec);
		}
	}
	else{
		std::cout << "disconnect() called from session::read_header_hander()\n";
		disconnect();
	}
}

void Session::read_body_handler(const boost::system::error_code &ec){
	if(!ec){
		player_->handle_packet();
		
		//read the next packet
		start_read_header();
	}
	else{
		std::cout << "disconnect() called from session::read_body_handler()\n";
		disconnect();
	}
}

void Session::disconnect(){
	socket_.close();
}

void Session::send_packet(Packet *packet){
	memcpy(send_, packet->get(), packet->size());
	
	boost::asio::async_write(socket_,
		boost::asio::buffer(send_, packet->size()),
		boost::bind(&Session::write_handler,
			shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void Session::write_handler(const boost::system::error_code &ec, size_t bytes_transferred){
	//nothing to do for now
}

Player *Session::get_player(){
	return player_;
}

boost::asio::ip::tcp::socket &Session::get_socket(){
	return socket_;
}

char* Session::get_recv(){
	return recv_;
}