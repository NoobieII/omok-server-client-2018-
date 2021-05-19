#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>

class Packet;
class Player;

class Session: public boost::enable_shared_from_this<Session>{
public:
	Session(boost::asio::io_service& io);
	~Session();
	
	void initialize();	//called after the socket is connected
	void start_read_header();
	void read_header_handler(const boost::system::error_code &ec);
	void read_body_handler(const boost::system::error_code &ec);
	
	void disconnect();
	
	void send_packet(Packet *packet);
	void write_handler(const boost::system::error_code &ec, size_t bytes_transferred);

	boost::asio::ip::tcp::socket &get_socket();
	char *get_recv();
private:
	Player *player_;
	boost::asio::ip::tcp::socket socket_;
	char send_[256];
	char recv_[256];
};

#endif