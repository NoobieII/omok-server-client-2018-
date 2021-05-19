#include "omok_game.hpp"
#include "omok_player.hpp"
#include "packet.hpp"
#include "player.hpp"
#include "server.hpp"
#include "session.hpp"

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

Server *Server::singleton_ = 0;

Server::Server():
	io_service_(),
	acceptor_(io_service_),
	player_id_(0),
	omok_room_id_(0),
	players_(),
	player_name_id_(),
	player_id_name_(),
	omok_games_()
{
}


Server::~Server(){
}

Server *Server::get_instance(){
	if(!singleton_){
		singleton_ = new Server;
	}
	return singleton_;
}

void Server::start_accept(std::string ip, std::string port){
	boost::asio::ip::tcp::resolver resolver(io_service_);
	boost::asio::ip::tcp::resolver::query query(ip, port);
	boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
	boost::asio::ip::tcp::endpoint endpoint = *iterator;
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option( boost::asio::ip::tcp::acceptor::reuse_address( false ) );
	acceptor_.bind(endpoint);
	acceptor_.listen(boost::asio::socket_base::max_connections);	
	
	boost::shared_ptr<Session> new_connection(new Session(io_service_));
	
	acceptor_.async_accept(new_connection->get_socket(),
		boost::bind(&Server::accept_handler, this, new_connection));
}

void Server::accept_handler(boost::shared_ptr<Session> session){
	session->initialize();
	
	boost::shared_ptr<Session> new_connection(new Session(io_service_));
	
	acceptor_.async_accept(new_connection->get_socket(),
		boost::bind(&Server::accept_handler, this, new_connection));
}
	

boost::asio::io_service &Server::get_io_service(){
	return io_service_;
}

void Server::send(Packet *packet){
	std::map<int, Player*>::iterator it = players_.begin();
	
	//send a packet to all players with a username (i.e. logged in)
	while(it != players_.end()){
		if(it->second && it->second->is_logged_in()){
			it->second->send(packet);
		}
		it++;
	}
}

void Server::add_player(int id, Player *player){
	players_[id] = player;
}

Player *Server::get_player(int id){
	return players_[id];
}

void Server::remove_player(int id){
	players_.erase(id);
}

int Server::get_num_players_online(){
	return players_.size();
}

void Server::add_player_name(int id, std::string name){
	player_name_id_[name] = id;
	player_id_name_[id] = name;
}

std::string Server::get_player_name_by_id(int id){
	return player_id_name_[id];
}

int Server::get_player_id_by_name(std::string name){
	return player_name_id_[name];
}

void Server::remove_player_name(int id, std::string name){
	player_name_id_.erase(name);
	player_id_name_.erase(id);
}

void Server::add_omok_game(int id, std::string room_name, Player *leader){
	omok_games_[id] = new OmokGame(id, room_name, leader->get_id(), leader->get_name());
}

OmokGame *Server::get_omok_game(int id){
	return omok_games_[id];
}

std::map<int, OmokGame*> *Server::get_omok_games(){
	return &omok_games_;
}

void Server::remove_omok_game(int id){
	if(omok_games_[id]){
		delete omok_games_[id];
	}
	omok_games_.erase(id);
}

int Server::get_player_id(){
	return ++player_id_;
}

int Server::get_omok_room_id(){
	return ++omok_room_id_;
}