#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "session.hpp"

#include <string>

class OmokGame;
class Packet;

class Player{
public:
	Player(Session *session);
	~Player();
	
	void send(Packet *packet);
	void disconnect();
	
	int get_id();
	void set_name(std::string name);
	std::string get_name();

	void set_is_logged_in(bool flag);
	bool is_logged_in();
	void set_is_in_omok_game(bool flag);
	bool is_in_omok_game();
	
	void set_omok_game(OmokGame *omok_game);
	OmokGame *get_omok_game();
	
	//packet handlers
	void handle_packet();
	void handle_packet_in_lobby();
	void handle_packet_in_omok_game();
	
	//before the user is given a name
	void handle_create_username();
	void handle_username_result();
	//in omok room lobby
	void handle_number_of_players_online();
	void handle_omok_room_info();
	void handle_omok_room_update();
	void handle_omok_join_room();
	void handle_omok_create_room();
	void handle_omok_room_admittance();
	void handle_omok_room_create();
	//in the omok room
	void handle_omok_start_game();
	void handle_omok_place_piece();
	void handle_omok_win_game();
	void handle_omok_lose_game();
	void handle_omok_message();
	void handle_omok_player_has_joined();
	void handle_omok_player_has_left();
	void handle_omok_switch_turn();
	void handle_omok_room_closure();
	void handle_omok_room_leave();
	
	//read function for integer type values
	template<typename TYPE>
	TYPE read(){
		char *recv = session_->get_recv();
		
		TYPE value = *(TYPE*)(recv + read_length_);
		read_length_ += sizeof(TYPE);
		
		return value;
	}
	void skip_bytes(int n);
		
private:
	int id_;
	int read_length_;
	std::string name_;
	bool is_logged_in_;
	bool is_in_omok_game_;
	OmokGame *omok_game_;
	Session *session_;
};

//template specialization for reading string and bool values
template<>
std::string Player::read<std::string>();
template<>
bool Player::read<bool>();

#endif