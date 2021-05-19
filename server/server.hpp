#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <string>

#include <boost/asio.hpp>

class OmokGame;
class Packet;
class Player;
class Session;

class Server{
public:
	Server();
	~Server();
	
	static Server *get_instance();
	
	void start_accept(std::string ip, std::string port);
	void accept_handler(boost::shared_ptr<Session> session);

	boost::asio::io_service &get_io_service();
	
	void send(Packet *packet);
	
	void add_player(int id, Player *player);
	Player *get_player(int id);
	void remove_player(int id);
	int get_num_players_online();
	
	void add_player_name(int id, std::string name);
	std::string get_player_name_by_id(int id);
	int get_player_id_by_name(std::string name);
	void remove_player_name(int id, std::string name);
	
	void add_omok_game(int id, std::string room_name, Player *leader);
	OmokGame *get_omok_game(int id);
	std::map<int, OmokGame*> *get_omok_games();
	void remove_omok_game(int id);
	
	int get_player_id();
	int get_omok_room_id();
private:
	static Server *singleton_;
	boost::asio::io_service io_service_;
	boost::asio::ip::tcp::acceptor acceptor_;
	
	int player_id_;
	int omok_room_id_;
	std::map<int, Player*> players_;
	std::map<std::string, int> player_name_id_;
	std::map<int, std::string> player_id_name_;
	std::map<int, OmokGame*> omok_games_;
};

#endif